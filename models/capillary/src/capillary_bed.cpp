// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_bed.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <variant>

namespace mehlissa::models::capillary {
namespace {

constexpr std::array expected_region_order{
    CapillaryRegionKind::arteriole, CapillaryRegionKind::capillary, CapillaryRegionKind::venule};
constexpr std::size_t capillary_region_index = 1;

[[nodiscard]] bool approximately_equal(const double left, const double right) noexcept {
    constexpr double relative_tolerance = 1.0e-12;
    const auto scale = std::max(std::abs(left), std::abs(right));
    return std::abs(left - right) <= relative_tolerance * scale;
}

struct QuantityContext final {
    std::string_view quantity;
    std::string_view region_id;
};

void require_positive_finite(const double value, const QuantityContext context) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary region '" + std::string{context.region_id} +
                                      "' requires " + std::string{context.quantity} +
                                      " to be positive and finite"};
    }
}

[[nodiscard]] CapillaryRegionMetrics derive_metrics(const CapillaryRegion& region,
                                                    const core::FlowRate flow_rate,
                                                    const std::uint64_t parallel_vessel_count) {
    const auto radius = region.diameter / 2.0;
    const auto single_cross_section = std::numbers::pi * radius * radius;
    const auto total_cross_section = single_cross_section * parallel_vessel_count;
    const auto blood_volume = total_cross_section * region.length;
    const auto velocity = flow_rate / total_cross_section;
    const auto transit = region.length / velocity;
    const auto transit_nanoseconds = core::in_seconds(transit) * 1'000'000'000.0;
    const auto maximum =
        static_cast<double>(std::numeric_limits<core::SimulationClock::Duration::rep>::max());
    if (!std::isfinite(transit_nanoseconds) || transit_nanoseconds <= 0.0 ||
        transit_nanoseconds > maximum) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "Capillary region '" + region.id +
                                      "' has an unrepresentable geometry-derived transit time"};
    }
    const auto rounded =
        static_cast<core::SimulationClock::Duration::rep>(std::llround(transit_nanoseconds));
    if (rounded <= 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary region '" + region.id +
                                      "' transit is below simulation-clock resolution"};
    }
    return {single_cross_section, total_cross_section, blood_volume, velocity,
            core::SimulationClock::Duration{rounded}};
}

[[nodiscard]] std::vector<CapillaryRegionMetrics>
validate_config(const CapillaryBedConfig& config) {
    if (config.component_name.empty() || config.model_id.empty() || config.entry_port_id.empty() ||
        config.exit_port_id.empty() || config.return_target_model_id.empty() ||
        config.return_target_port_id.empty()) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "A capillary bed requires non-empty component, model, and port identifiers"};
    }
    if (config.entry_port_id == config.exit_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary-bed entry and exit ports must be distinct"};
    }
    if (config.total_parallel_path_count == 0 || config.perfused_path_count == 0 ||
        config.perfused_path_count > config.total_parallel_path_count) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "A capillary bed requires a positive, bounded number of perfused paths"};
    }
    if (config.regions.size() != expected_region_order.size()) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "A capillary bed requires exactly arteriole, capillary, and venule regions"};
    }
    require_positive_finite(core::in_cubic_meters_per_second(config.volume_flow_rate),
                            {"network volume flow", config.model_id});

    std::unordered_set<std::string> region_ids;
    std::vector<CapillaryRegionMetrics> metrics;
    metrics.reserve(config.regions.size());
    for (std::size_t index{}; index < config.regions.size(); ++index) {
        const auto& region = config.regions[index];
        if (region.id.empty() || !region_ids.insert(region.id).second ||
            region.kind != expected_region_order[index] || region.parallel_vessel_count == 0) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Capillary regions require unique IDs, positive vessel counts, and physiological "
                "order"};
        }
        require_positive_finite(core::in_meters(region.length), {"length", region.id});
        require_positive_finite(core::in_meters(region.diameter), {"diameter", region.id});
        if (region.kind == CapillaryRegionKind::capillary &&
            region.parallel_vessel_count != config.perfused_path_count) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "The capillary region vessel count must equal the perfused path count"};
        }
        metrics.push_back(
            derive_metrics(region, config.volume_flow_rate, region.parallel_vessel_count));
    }
    return metrics;
}

[[nodiscard]] std::vector<CapillaryRegionMetrics>
derive_current_metrics(const CapillaryBedConfig& config, const core::FlowRate flow_rate,
                       const std::uint64_t perfused_path_count) {
    std::vector<CapillaryRegionMetrics> metrics;
    metrics.reserve(config.regions.size());
    for (const auto& region : config.regions) {
        const auto vessel_count = region.kind == CapillaryRegionKind::capillary
                                      ? perfused_path_count
                                      : region.parallel_vessel_count;
        metrics.push_back(derive_metrics(region, flow_rate, vessel_count));
    }
    return metrics;
}

[[nodiscard]] std::uint64_t open_path_count(const CapillaryRecruitmentProfile& profile,
                                            const CapillaryRecruitmentState& state) {
    std::unordered_map<std::string_view, std::uint64_t> group_path_counts;
    group_path_counts.reserve(profile.sphincter_groups.size());
    for (const auto& group : profile.sphincter_groups) {
        group_path_counts.emplace(group.id, group.path_count);
    }
    std::uint64_t result{};
    for (const auto& group_id : state.open_sphincter_group_ids) {
        const auto count = group_path_counts.at(group_id);
        if (result > std::numeric_limits<std::uint64_t>::max() - count) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Open capillary path count overflows its representation"};
        }
        result += count;
    }
    return result;
}

void validate_recruitment_compatibility(const CapillaryBedConfig& config,
                                        const CapillaryRecruitmentProfile& profile) {
    validate_capillary_recruitment_profile(profile);
    if (profile.compatible_model_id != config.model_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary-recruitment profile targets a different model"};
    }
    std::uint64_t group_path_count{};
    for (const auto& group : profile.sphincter_groups) {
        if (group_path_count > std::numeric_limits<std::uint64_t>::max() - group.path_count) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Capillary sphincter-group path count overflows"};
        }
        group_path_count += group.path_count;
    }
    if (group_path_count != config.total_parallel_path_count) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Capillary sphincter groups must partition all parallel paths exactly once"};
    }
    if (open_path_count(profile, profile.states.front()) != config.perfused_path_count) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Initial recruitment state must match the capillary-bed perfused path count"};
    }
    for (const auto& state : profile.states) {
        const auto perfused_path_count = open_path_count(profile, state);
        const auto flow_rate =
            profile.boundary_condition == CapillaryBoundaryCondition::fixed_total_flow
                ? config.volume_flow_rate
                : config.volume_flow_rate * (static_cast<double>(perfused_path_count) /
                                             static_cast<double>(config.perfused_path_count));
        static_cast<void>(derive_current_metrics(config, flow_rate, perfused_path_count));
    }
}

void validate_exchange_compatibility(const CapillaryBedConfig& config,
                                     const CapillaryExchangeProfile& profile) {
    validate_capillary_exchange_profile(profile);
    if (profile.compatible_model_id != config.model_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary-exchange profile targets a different model"};
    }
}

void validate_entity_observation_compatibility(const CapillaryBedConfig& config,
                                               const CapillaryEntityObservationProfile& profile) {
    validate_capillary_entity_observation_profile(profile);
    if (profile.compatible_model_id != config.model_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Capillary-entity-observation profile targets a different model"};
    }
}

void validate_entity_disposition_compatibility(
    const CapillaryBedConfig& config, const CapillaryEntityObservationProfile* observation,
    const CapillaryEntityDispositionProfile& disposition) {
    validate_capillary_entity_disposition_profile(disposition);
    if (observation == nullptr || disposition.compatible_model_id != config.model_id ||
        disposition.compatible_observation_profile_id != observation->profile_id ||
        disposition.source_port_id == config.entry_port_id ||
        disposition.source_port_id == config.exit_port_id) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Entity disposition requires compatible capillary and observation profiles"};
    }
}

void validate_conserved_target(const coupling::ConservedTransfer& transfer,
                               const CapillaryBedConfig& config,
                               const core::SimulationClock::Duration synchronization_time,
                               const core::FlowRate expected_flow_rate) {
    coupling::validate_transfer(transfer);
    const auto& header = coupling::transfer_header(transfer);
    if (header.target_model_id != config.model_id ||
        header.target_port_id != config.entry_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Conserved transfer does not target this capillary-bed entry"};
    }
    if (header.emitted_at != synchronization_time) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Conserved transfer must enter the capillary bed at synchronization time"};
    }
    if (const auto* flow = std::get_if<coupling::VolumeFlowTransfer>(&transfer);
        flow != nullptr &&
        !approximately_equal(core::in_cubic_meters_per_second(flow->flow_rate),
                             core::in_cubic_meters_per_second(expected_flow_rate))) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Volume-flow transfer does not match the capillary-bed continuity flow"};
    }
}

void route_conserved_return(coupling::ConservedTransfer& transfer, const CapillaryBedConfig& config,
                            const core::SimulationClock::Duration emitted_at) {
    auto& header = coupling::transfer_header(transfer);
    header.source_model_id = config.model_id;
    header.source_port_id = config.exit_port_id;
    header.target_model_id = config.return_target_model_id;
    header.target_port_id = config.return_target_port_id;
    header.emitted_at = emitted_at;
}

template <typename Resident>
void advance_resident(Resident& resident, const std::vector<CapillaryRegion>& regions,
                      const std::vector<CapillaryRegionMetrics>& metrics,
                      core::SimulationClock::Duration delta) {
    constexpr double nanoseconds_per_second = 1'000'000'000.0;
    while (delta > core::SimulationClock::Duration::zero() &&
           resident.region_index < metrics.size()) {
        const auto active_region_index = resident.region_index;
        const auto region_length = core::in_meters(regions[resident.region_index].length);
        const auto travelled = core::in_meters(resident.region_distance);
        const auto remaining_distance = std::max(0.0, region_length - travelled);
        const auto velocity =
            core::in_meters_per_second(metrics[resident.region_index].mean_velocity);
        const auto available_distance =
            velocity * static_cast<double>(delta.count()) / nanoseconds_per_second;
        const auto distance_tolerance = region_length * 1.0e-12;
        if (available_distance + distance_tolerance < remaining_distance) {
            resident.region_distance += core::meters(available_distance);
            resident.region_residence_times.at(active_region_index) += delta;
            delta = core::SimulationClock::Duration::zero();
        } else {
            const auto required_nanoseconds = static_cast<core::SimulationClock::Duration::rep>(
                std::llround(remaining_distance / velocity * nanoseconds_per_second));
            const auto required = core::SimulationClock::Duration{required_nanoseconds};
            resident.region_residence_times.at(active_region_index) += std::min(required, delta);
            delta = required < delta ? delta - required : core::SimulationClock::Duration::zero();
            ++resident.region_index;
            resident.region_distance = {};
        }
    }
}

} // namespace

std::string_view to_string(const CapillaryRegionKind kind) noexcept {
    switch (kind) {
    case CapillaryRegionKind::arteriole:
        return "arteriole";
    case CapillaryRegionKind::capillary:
        return "capillary";
    case CapillaryRegionKind::venule:
        return "venule";
    }
    return "unknown";
}

CapillaryBed::CapillaryBed(CapillaryBedConfig config)
    : config_{std::move(config)}, region_metrics_{validate_config(config_)},
      current_volume_flow_rate_{config_.volume_flow_rate},
      current_perfused_path_count_{config_.perfused_path_count} {}

CapillaryBed::CapillaryBed(CapillaryBedConfig config, CapillaryBedProfiles profiles)
    : CapillaryBed{std::move(config)} {
    if (profiles.recruitment.has_value()) {
        validate_recruitment_compatibility(config_, *profiles.recruitment);
        recruitment_profile_ = std::move(profiles.recruitment);
        apply_recruitment_state(0);
        next_recruitment_state_index_ = 1;
    }
    if (profiles.exchange.has_value()) {
        validate_exchange_compatibility(config_, *profiles.exchange);
        exchange_profile_ = std::move(profiles.exchange);
    }
    if (profiles.entity_observation.has_value()) {
        validate_entity_observation_compatibility(config_, *profiles.entity_observation);
        entity_observation_profile_ = std::move(profiles.entity_observation);
    }
    if (profiles.entity_disposition.has_value()) {
        validate_entity_disposition_compatibility(
            config_,
            entity_observation_profile_.has_value() ? &*entity_observation_profile_ : nullptr,
            *profiles.entity_disposition);
        entity_disposition_profile_ = std::move(profiles.entity_disposition);
    }
}

CapillaryBed::CapillaryBed(CapillaryBedConfig config,
                           CapillaryRecruitmentProfile recruitment_profile)
    : CapillaryBed{std::move(config),
                   CapillaryBedProfiles{std::move(recruitment_profile), std::nullopt, std::nullopt,
                                        std::nullopt}} {}

CapillaryBed::CapillaryBed(CapillaryBedConfig config, CapillaryExchangeProfile exchange_profile)
    : CapillaryBed{std::move(config),
                   CapillaryBedProfiles{std::nullopt, std::move(exchange_profile), std::nullopt,
                                        std::nullopt}} {}

CapillaryBed::CapillaryBed(CapillaryBedConfig config,
                           CapillaryRecruitmentProfile recruitment_profile,
                           CapillaryExchangeProfile exchange_profile)
    : CapillaryBed{std::move(config),
                   CapillaryBedProfiles{std::move(recruitment_profile), std::move(exchange_profile),
                                        std::nullopt, std::nullopt}} {}

std::string_view CapillaryBed::name() const noexcept { return config_.component_name; }

std::string_view CapillaryBed::model_id() const noexcept { return config_.model_id; }

bool CapillaryBed::accepts_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.entry_port_id;
}

bool CapillaryBed::emits_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.exit_port_id;
}

void CapillaryBed::initialize(core::SimulationContext& context) {
    if (state_ != State::building) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "A capillary bed can only be initialized once"};
    }
    synchronization_time_ = context.clock().now();
    if (recruitment_profile_.has_value()) {
        const auto final_offset = recruitment_profile_->states.back().effective_at.count();
        const auto maximum = std::numeric_limits<core::SimulationClock::Duration::rep>::max();
        if (final_offset > maximum - synchronization_time_.count()) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Capillary-recruitment schedule exceeds clock range"};
        }
    }
    recruitment_origin_time_ = synchronization_time_;
    state_ = State::initialized;
}

void CapillaryBed::advance(core::SimulationContext& context,
                           const core::SimulationClock::Duration delta) {
    if (state_ != State::initialized || context.clock().now() != synchronization_time_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Capillary bed and simulation clock are not synchronized"};
    }

    core::SimulationClock target_clock{synchronization_time_};
    target_clock.advance(delta);
    auto* disposition_random =
        entity_disposition_profile_.has_value()
            ? &context.random_stream(entity_disposition_profile_->random_stream_name)
            : nullptr;
    auto cursor = synchronization_time_;
    if (recruitment_profile_.has_value()) {
        const auto& states = recruitment_profile_->states;
        while (next_recruitment_state_index_ < states.size()) {
            const auto event_time = core::SimulationClock::Duration{
                recruitment_origin_time_.count() +
                states[next_recruitment_state_index_].effective_at.count()};
            if (event_time > target_clock.now()) {
                break;
            }
            if (event_time < cursor) {
                throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                          "Capillary-recruitment schedule moved backwards"};
            }
            advance_residents({event_time - cursor, target_clock.now()}, disposition_random);
            apply_recruitment_state(next_recruitment_state_index_);
            ++next_recruitment_state_index_;
            cursor = event_time;
        }
    }
    advance_residents({target_clock.now() - cursor, target_clock.now()}, disposition_random);
    synchronization_time_ = target_clock.now();
}

void CapillaryBed::advance_residents(const AdvanceInterval interval,
                                     core::RandomStream* disposition_random) {
    std::vector<ResidentEntity> remaining_entities;
    remaining_entities.reserve(resident_entities_.size());
    for (auto& resident : resident_entities_) {
        advance_resident(resident, config_.regions, region_metrics_, interval.delta);
        if (resident.region_index == config_.regions.size()) {
            const auto observation = record_entity_observation(resident, interval.emitted_at);
            const auto disposition =
                observation.has_value() && disposition_random != nullptr
                    ? sample_entity_disposition(resident, *observation, interval.emitted_at,
                                                *disposition_random)
                    : std::nullopt;
            if (disposition.has_value()) {
                outbound_entity_dispositions_.push_back(*disposition);
            } else {
                outbound_entities_.push_back({
                    std::string{coupling::entity_transfer_contract_version},
                    resident.transfer.entity_id,
                    std::move(resident.transfer.entity_type),
                    config_.model_id,
                    config_.exit_port_id,
                    config_.return_target_model_id,
                    config_.return_target_port_id,
                    interval.emitted_at,
                });
            }
        } else {
            remaining_entities.push_back(std::move(resident));
        }
    }
    resident_entities_ = std::move(remaining_entities);

    std::vector<ResidentConservedTransfer> remaining_conserved;
    remaining_conserved.reserve(resident_conserved_transfers_.size());
    for (auto& resident : resident_conserved_transfers_) {
        const auto previous_region_index = resident.region_index;
        advance_resident(resident, config_.regions, region_metrics_, interval.delta);
        if (previous_region_index <= capillary_region_index &&
            resident.region_index > capillary_region_index) {
            apply_substance_exchange(resident.transfer, interval.emitted_at);
        }
        if (resident.region_index == config_.regions.size()) {
            route_conserved_return(resident.transfer, config_, interval.emitted_at);
            outbound_conserved_transfers_.push_back(std::move(resident.transfer));
        } else {
            remaining_conserved.push_back(std::move(resident));
        }
    }
    resident_conserved_transfers_ = std::move(remaining_conserved);
}

std::optional<CapillaryEntityObservationRecord>
CapillaryBed::record_entity_observation(const ResidentEntity& resident,
                                        const core::SimulationClock::Duration reported_at) {
    if (!entity_observation_profile_.has_value()) {
        return std::nullopt;
    }
    const auto& profile = *entity_observation_profile_;
    const auto rule =
        std::find_if(profile.entity_rules.begin(), profile.entity_rules.end(),
                     [&resident](const auto& candidate) {
                         return candidate.entity_type == resident.transfer.entity_type;
                     });
    const auto* applied_rule = rule == profile.entity_rules.end() ? nullptr : &*rule;
    auto record = make_capillary_entity_observation(
        {resident.transfer.entity_id, resident.transfer.entity_type, profile.profile_id,
         reported_at, resident.region_residence_times},
        applied_rule);
    if (!has_normalized_outcome_likelihoods(record)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary entity-observation likelihoods are not normalized"};
    }
    if (entity_observation_records_.size() < profile.maximum_buffered_records) {
        entity_observation_records_.push_back(record);
    } else if (dropped_entity_observation_records_ < std::numeric_limits<std::uint64_t>::max()) {
        ++dropped_entity_observation_records_;
    }
    return record;
}

std::optional<coupling::EntityDispositionTransfer> CapillaryBed::sample_entity_disposition(
    const ResidentEntity& resident, const CapillaryEntityObservationRecord& observation,
    const core::SimulationClock::Duration decided_at, core::RandomStream& random) const {
    if (!entity_disposition_profile_.has_value() || !observation.interaction_rule_applied) {
        return std::nullopt;
    }
    constexpr double two_to_minus_53 = 1.0 / 9'007'199'254'740'992.0;
    const auto grid_value = random.next_u64() >> 11U;
    const auto draw = (static_cast<double>(grid_value) + 0.5) * two_to_minus_53;
    if (draw < observation.pass_through_likelihood) {
        return std::nullopt;
    }

    coupling::EntityDispositionKind kind{};
    double probability{};
    const auto retention_end =
        observation.pass_through_likelihood + observation.retention_likelihood;
    const auto adhesion_end = retention_end + observation.adhesion_likelihood;
    if (draw < retention_end) {
        kind = coupling::EntityDispositionKind::retained;
        probability = observation.retention_likelihood;
    } else if (draw < adhesion_end) {
        kind = coupling::EntityDispositionKind::adhered;
        probability = observation.adhesion_likelihood;
    } else {
        kind = coupling::EntityDispositionKind::extravasated;
        probability = observation.extravasation_likelihood;
    }
    const auto& profile = *entity_disposition_profile_;
    const auto& target = disposition_target(profile, kind);
    coupling::EntityDispositionTransfer transfer{
        std::string{coupling::entity_disposition_contract_version},
        resident.transfer.entity_id,
        resident.transfer.entity_type,
        kind,
        profile.profile_id,
        config_.model_id,
        profile.source_port_id,
        target.model_id,
        target.compartment_id,
        decided_at,
        draw,
        probability,
    };
    coupling::validate_entity_disposition(transfer);
    return transfer;
}

void CapillaryBed::apply_substance_exchange(coupling::ConservedTransfer& transfer,
                                            const core::SimulationClock::Duration reported_at) {
    if (!exchange_profile_.has_value()) {
        return;
    }
    auto* substance = std::get_if<coupling::SubstanceAmountTransfer>(&transfer);
    if (substance == nullptr) {
        return;
    }
    const auto rule =
        std::find_if(exchange_profile_->substance_rules.begin(),
                     exchange_profile_->substance_rules.end(), [&substance](const auto& candidate) {
                         return candidate.substance_id == substance->substance_id;
                     });
    if (rule == exchange_profile_->substance_rules.end()) {
        return;
    }

    const auto incoming = core::in_moles(substance->amount);
    const auto transferred_to_endothelium = incoming * rule->blood_to_endothelium_fraction;
    const auto outgoing_blood = incoming - transferred_to_endothelium;
    const auto transferred_to_interstitium =
        transferred_to_endothelium * rule->endothelium_to_interstitium_fraction;
    const auto retained_endothelium = transferred_to_endothelium - transferred_to_interstitium;
    const auto transferred_to_cell =
        transferred_to_interstitium * rule->interstitium_to_cell_fraction;
    const auto retained_interstitium = transferred_to_interstitium - transferred_to_cell;

    CapillaryExchangeRecord record{
        substance->header.transfer_id,
        substance->substance_id,
        exchange_profile_->profile_id,
        reported_at,
        substance->amount,
        core::moles(outgoing_blood),
        core::moles(retained_endothelium),
        core::moles(retained_interstitium),
        core::moles(transferred_to_cell),
    };
    if (!is_balanced(record)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary substance exchange is not mass balanced"};
    }
    auto& inventory = tissue_inventories_[substance->substance_id];
    inventory.endothelium_amount += record.endothelium_amount;
    inventory.interstitium_amount += record.interstitium_amount;
    inventory.cell_amount += record.cell_amount;
    substance->amount = record.outgoing_blood_amount;
    exchange_records_.push_back(std::move(record));
}

void CapillaryBed::apply_recruitment_state(const std::size_t state_index) {
    if (!recruitment_profile_.has_value()) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary recruitment state requires a profile"};
    }
    const auto& profile = *recruitment_profile_;
    const auto& state = profile.states.at(state_index);
    current_perfused_path_count_ = open_path_count(profile, state);
    if (profile.boundary_condition == CapillaryBoundaryCondition::fixed_total_flow) {
        current_volume_flow_rate_ = config_.volume_flow_rate;
    } else {
        current_volume_flow_rate_ =
            config_.volume_flow_rate * (static_cast<double>(current_perfused_path_count_) /
                                        static_cast<double>(config_.perfused_path_count));
    }
    region_metrics_ =
        derive_current_metrics(config_, current_volume_flow_rate_, current_perfused_path_count_);
    current_recruitment_state_index_ = state_index;
}

void CapillaryBed::finalize(core::SimulationContext&) noexcept { state_ = State::finalized; }

void CapillaryBed::accept_entity(coupling::EntityTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only an initialized capillary bed can accept entities"};
    }
    coupling::validate_entity_transfer(transfer);
    if (transfer.target_model_id != config_.model_id ||
        transfer.target_port_id != config_.entry_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Entity transfer does not target this capillary-bed entry"};
    }
    if (transfer.emitted_at != synchronization_time_) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Entity transfer must enter the capillary bed at synchronization time"};
    }
    if (!held_entity_ids_.insert(transfer.entity_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary bed already holds this entity ID"};
    }
    resident_entities_.push_back({std::move(transfer), {}, {}, {}});
}

std::vector<coupling::EntityTransfer> CapillaryBed::take_outbound_entities() {
    auto result = std::move(outbound_entities_);
    outbound_entities_.clear();
    for (const auto& transfer : result) {
        held_entity_ids_.erase(transfer.entity_id);
    }
    return result;
}

void CapillaryBed::accept_conserved_transfer(coupling::ConservedTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{
            core::ErrorCode::lifecycle_invalid,
            "Only an initialized capillary bed can accept conserved transfers"};
    }
    validate_conserved_target(transfer, config_, synchronization_time_, current_volume_flow_rate_);
    const auto& transfer_id = coupling::transfer_header(transfer).transfer_id;
    if (!held_transfer_ids_.insert(transfer_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary bed already holds this transfer ID"};
    }
    resident_conserved_transfers_.push_back({std::move(transfer), {}, {}, {}});
}

std::vector<coupling::ConservedTransfer> CapillaryBed::take_outbound_conserved_transfers() {
    auto result = std::move(outbound_conserved_transfers_);
    outbound_conserved_transfers_.clear();
    for (const auto& transfer : result) {
        held_transfer_ids_.erase(coupling::transfer_header(transfer).transfer_id);
    }
    return result;
}

std::size_t CapillaryBed::resident_conserved_transfer_count() const noexcept {
    return resident_conserved_transfers_.size();
}

std::size_t CapillaryBed::region_count() const noexcept { return config_.regions.size(); }

std::uint64_t CapillaryBed::total_parallel_path_count() const noexcept {
    return config_.total_parallel_path_count;
}

std::uint64_t CapillaryBed::perfused_path_count() const noexcept {
    return current_perfused_path_count_;
}

core::FlowRate CapillaryBed::volume_flow_rate() const noexcept { return current_volume_flow_rate_; }

bool CapillaryBed::has_recruitment_profile() const noexcept {
    return recruitment_profile_.has_value();
}

std::string_view CapillaryBed::recruitment_state_id() const noexcept {
    if (!recruitment_profile_.has_value()) {
        return {};
    }
    return recruitment_profile_->states[current_recruitment_state_index_].id;
}

std::size_t CapillaryBed::open_sphincter_group_count() const noexcept {
    if (!recruitment_profile_.has_value()) {
        return 0;
    }
    return recruitment_profile_->states[current_recruitment_state_index_]
        .open_sphincter_group_ids.size();
}

std::optional<CapillaryBoundaryCondition> CapillaryBed::boundary_condition() const noexcept {
    if (!recruitment_profile_.has_value()) {
        return std::nullopt;
    }
    return recruitment_profile_->boundary_condition;
}

bool CapillaryBed::has_exchange_profile() const noexcept { return exchange_profile_.has_value(); }

std::string_view CapillaryBed::exchange_profile_id() const noexcept {
    if (!exchange_profile_.has_value()) {
        return {};
    }
    return exchange_profile_->profile_id;
}

CapillaryTissueInventory CapillaryBed::tissue_inventory(const std::string_view substance_id) const {
    const auto inventory = tissue_inventories_.find(std::string{substance_id});
    return inventory == tissue_inventories_.end() ? CapillaryTissueInventory{} : inventory->second;
}

std::size_t CapillaryBed::exchange_record_count() const noexcept {
    return exchange_records_.size();
}

std::vector<CapillaryExchangeRecord> CapillaryBed::take_exchange_records() {
    auto result = std::move(exchange_records_);
    exchange_records_.clear();
    return result;
}

bool CapillaryBed::has_entity_observation_profile() const noexcept {
    return entity_observation_profile_.has_value();
}

std::string_view CapillaryBed::entity_observation_profile_id() const noexcept {
    if (!entity_observation_profile_.has_value()) {
        return {};
    }
    return entity_observation_profile_->profile_id;
}

std::vector<CapillaryEntityPosition> CapillaryBed::entity_positions() const {
    std::vector<CapillaryEntityPosition> result;
    result.reserve(resident_entities_.size());
    for (const auto& resident : resident_entities_) {
        if (resident.region_index >= config_.regions.size()) {
            continue;
        }
        const auto& region = config_.regions[resident.region_index];
        const auto axial_position = core::in_meters(resident.region_distance);
        const auto region_length = core::in_meters(region.length);
        const auto accumulated = std::accumulate(resident.region_residence_times.begin(),
                                                 resident.region_residence_times.end(),
                                                 core::SimulationClock::Duration::zero());
        result.push_back({resident.transfer.entity_id, resident.transfer.entity_type, region.id,
                          std::string{to_string(region.kind)}, resident.region_distance,
                          axial_position / region_length, accumulated});
    }
    return result;
}

std::size_t CapillaryBed::entity_observation_record_count() const noexcept {
    return entity_observation_records_.size();
}

std::uint64_t CapillaryBed::dropped_entity_observation_record_count() const noexcept {
    return dropped_entity_observation_records_;
}

std::vector<CapillaryEntityObservationRecord> CapillaryBed::take_entity_observation_records() {
    auto result = std::move(entity_observation_records_);
    entity_observation_records_.clear();
    return result;
}

bool CapillaryBed::has_entity_disposition_profile() const noexcept {
    return entity_disposition_profile_.has_value();
}

std::string_view CapillaryBed::entity_disposition_profile_id() const noexcept {
    return entity_disposition_profile_.has_value() ? entity_disposition_profile_->profile_id
                                                   : std::string_view{};
}

std::size_t CapillaryBed::pending_entity_disposition_count() const noexcept {
    return outbound_entity_dispositions_.size();
}

std::vector<coupling::EntityDispositionTransfer> CapillaryBed::take_outbound_entity_dispositions() {
    auto result = std::move(outbound_entity_dispositions_);
    outbound_entity_dispositions_.clear();
    for (const auto& transfer : result) {
        held_entity_ids_.erase(transfer.entity_id);
    }
    return result;
}

const CapillaryRegionMetrics& CapillaryBed::region_metrics(const CapillaryRegionKind kind) const {
    const auto region =
        std::find_if(config_.regions.begin(), config_.regions.end(),
                     [kind](const auto& candidate) { return candidate.kind == kind; });
    if (region == config_.regions.end()) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary region metrics are unavailable"};
    }
    return region_metrics_.at(static_cast<std::size_t>(region - config_.regions.begin()));
}

std::size_t CapillaryBed::resident_entity_count() const noexcept {
    return resident_entities_.size();
}

std::size_t CapillaryBed::resident_entity_count_in(const CapillaryRegionKind kind) const noexcept {
    return static_cast<std::size_t>(std::count_if(
        resident_entities_.begin(), resident_entities_.end(), [this, kind](const auto& resident) {
            return resident.region_index < config_.regions.size() &&
                   config_.regions[resident.region_index].kind == kind;
        }));
}

} // namespace mehlissa::models::capillary

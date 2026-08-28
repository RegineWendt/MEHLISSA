// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_parallel_beds.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <iterator>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::organ {
namespace {

void validate_beds(const std::vector<PulmonaryParallelBedParameters>& beds) {
    if (beds.size() < 2) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Parallel pulmonary circulation requires at least two beds"};
    }
    std::unordered_set<std::string> ids;
    auto sum = 0.0;
    for (const auto& bed : beds) {
        const auto fraction = bed.perfusion_fraction.si_value();
        if (bed.id.empty() || !ids.insert(bed.id).second || !std::isfinite(fraction) ||
            fraction <= 0.0 || fraction >= 1.0 ||
            bed.transit_time <= core::SimulationClock::Duration::zero()) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Parallel pulmonary beds require unique IDs, positive fractions, and transit"};
        }
        sum += fraction;
    }
    if (std::abs(sum - 1.0) > 1.0e-10) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Parallel pulmonary bed fractions must sum to one"};
    }
}

[[nodiscard]] std::uint64_t mix_entity_id(std::uint64_t value) noexcept {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

} // namespace

PulmonaryParallelBedsModel::PulmonaryParallelBedsModel(PulmonaryZeroDimensionalConfig config)
    : config_{std::move(config)}, aggregate_{config_} {
    validate_beds(config_.parameters.parallel_beds);
    bed_transits_.reserve(config_.parameters.parallel_beds.size());
    for (const auto& bed : config_.parameters.parallel_beds) {
        bed_transits_.push_back(std::make_unique<PulmonaryCirculation>(PulmonaryCirculationConfig{
            config_.component_name,
            config_.model_id,
            config_.entry_port_id,
            config_.exit_port_id,
            config_.return_target_model_id,
            config_.return_target_port_id,
            {{bed.id, bed.transit_time}},
        }));
    }
}

std::string_view PulmonaryParallelBedsModel::name() const noexcept {
    return config_.component_name;
}
std::string_view PulmonaryParallelBedsModel::model_id() const noexcept { return config_.model_id; }
bool PulmonaryParallelBedsModel::accepts_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.entry_port_id;
}
bool PulmonaryParallelBedsModel::emits_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.exit_port_id;
}

void PulmonaryParallelBedsModel::initialize(core::SimulationContext& context) {
    aggregate_.initialize(context);
    for (auto& transit : bed_transits_) {
        transit->initialize(context);
    }
}

void PulmonaryParallelBedsModel::advance(core::SimulationContext& context,
                                         const core::SimulationClock::Duration delta) {
    aggregate_.advance(context, delta);
    for (auto& transit : bed_transits_) {
        transit->advance(context, delta);
    }
}

void PulmonaryParallelBedsModel::finalize(core::SimulationContext& context) noexcept {
    aggregate_.finalize(context);
    for (auto& transit : bed_transits_) {
        transit->finalize(context);
    }
}

void PulmonaryParallelBedsModel::accept_entity(coupling::EntityTransfer transfer) {
    const auto bed_index = bed_index_for_entity(transfer.entity_id);
    bed_transits_[bed_index]->accept_entity(std::move(transfer));
}

std::vector<coupling::EntityTransfer> PulmonaryParallelBedsModel::take_outbound_entities() {
    std::vector<coupling::EntityTransfer> result;
    for (auto& transit : bed_transits_) {
        auto outbound = transit->take_outbound_entities();
        result.insert(result.end(), std::make_move_iterator(outbound.begin()),
                      std::make_move_iterator(outbound.end()));
    }
    return result;
}

void PulmonaryParallelBedsModel::accept_conserved_transfer(coupling::ConservedTransfer transfer) {
    aggregate_.accept_conserved_transfer(std::move(transfer));
}

std::vector<coupling::ConservedTransfer>
PulmonaryParallelBedsModel::take_outbound_conserved_transfers() {
    return aggregate_.take_outbound_conserved_transfers();
}

std::size_t PulmonaryParallelBedsModel::resident_conserved_transfer_count() const noexcept {
    return aggregate_.resident_conserved_transfer_count();
}

PulmonaryParallelBedsState PulmonaryParallelBedsModel::state() const {
    const auto aggregate = aggregate_.state();
    std::vector<PulmonaryParallelBedState> beds;
    beds.reserve(config_.parameters.parallel_beds.size());
    for (const auto& parameters : config_.parameters.parallel_beds) {
        const auto fraction = parameters.perfusion_fraction.si_value();
        const auto flow = aggregate.pulmonary_outflow * fraction;
        const auto transit_seconds =
            static_cast<double>(parameters.transit_time.count()) /
            static_cast<double>(core::SimulationClock::Duration::period::den);
        beds.push_back({parameters.id, parameters.perfusion_fraction, flow,
                        aggregate.effective_pulmonary_vascular_resistance / fraction,
                        aggregate.effective_pulmonary_arterial_compliance * fraction,
                        parameters.transit_time, flow * core::seconds(transit_seconds)});
    }
    return {aggregate, std::move(beds)};
}

std::size_t
PulmonaryParallelBedsModel::bed_index_for_entity(const std::uint64_t entity_id) const noexcept {
    constexpr auto denominator = 9007199254740992.0;
    const auto mixed = mix_entity_id(entity_id);
    const auto draw = static_cast<double>(mixed >> 11U) / denominator;
    auto cumulative = 0.0;
    for (std::size_t index = 0; index < config_.parameters.parallel_beds.size(); ++index) {
        cumulative += config_.parameters.parallel_beds[index].perfusion_fraction.si_value();
        if (draw < cumulative) {
            return index;
        }
    }
    return config_.parameters.parallel_beds.size() - 1;
}

std::size_t PulmonaryParallelBedsModel::resident_entity_count() const noexcept {
    auto count = std::size_t{0};
    for (const auto& transit : bed_transits_) {
        count += transit->resident_count();
    }
    return count;
}

} // namespace mehlissa::models::organ

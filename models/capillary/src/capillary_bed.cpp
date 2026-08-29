// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_bed.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <array>
#include <utility>

namespace mehlissa::models::capillary {
namespace {

constexpr std::array expected_region_order{
    CapillaryRegionKind::arteriole, CapillaryRegionKind::capillary, CapillaryRegionKind::venule};

void validate_config(const CapillaryBedConfig& config) {
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

    std::unordered_set<std::string> region_ids;
    for (std::size_t index{}; index < config.regions.size(); ++index) {
        const auto& region = config.regions[index];
        if (region.id.empty() || !region_ids.insert(region.id).second ||
            region.kind != expected_region_order[index] ||
            region.transit_time <= core::SimulationClock::Duration::zero()) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Capillary regions require unique IDs, positive transit, and physiological order"};
        }
    }
}

void validate_conserved_target(const coupling::ConservedTransfer& transfer,
                               const CapillaryBedConfig& config,
                               const core::SimulationClock::Duration synchronization_time) {
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
                      core::SimulationClock::Duration delta) {
    while (delta > core::SimulationClock::Duration::zero() &&
           resident.region_index < regions.size()) {
        const auto required = regions[resident.region_index].transit_time - resident.region_time;
        if (delta < required) {
            resident.region_time += delta;
            delta = {};
        } else {
            delta -= required;
            ++resident.region_index;
            resident.region_time = {};
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

CapillaryBed::CapillaryBed(CapillaryBedConfig config) : config_{std::move(config)} {
    validate_config(config_);
}

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
    state_ = State::initialized;
}

void CapillaryBed::advance(core::SimulationContext& context,
                           const core::SimulationClock::Duration delta) {
    if (state_ != State::initialized || context.clock().now() != synchronization_time_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Capillary bed and simulation clock are not synchronized"};
    }

    core::SimulationClock next{synchronization_time_};
    next.advance(delta);

    std::vector<ResidentEntity> remaining_entities;
    remaining_entities.reserve(resident_entities_.size());
    for (auto& resident : resident_entities_) {
        advance_resident(resident, config_.regions, delta);
        if (resident.region_index == config_.regions.size()) {
            outbound_entities_.push_back({
                std::string{coupling::entity_transfer_contract_version},
                resident.transfer.entity_id,
                std::move(resident.transfer.entity_type),
                config_.model_id,
                config_.exit_port_id,
                config_.return_target_model_id,
                config_.return_target_port_id,
                next.now(),
            });
        } else {
            remaining_entities.push_back(std::move(resident));
        }
    }
    resident_entities_ = std::move(remaining_entities);

    std::vector<ResidentConservedTransfer> remaining_conserved;
    remaining_conserved.reserve(resident_conserved_transfers_.size());
    for (auto& resident : resident_conserved_transfers_) {
        advance_resident(resident, config_.regions, delta);
        if (resident.region_index == config_.regions.size()) {
            route_conserved_return(resident.transfer, config_, next.now());
            outbound_conserved_transfers_.push_back(std::move(resident.transfer));
        } else {
            remaining_conserved.push_back(std::move(resident));
        }
    }
    resident_conserved_transfers_ = std::move(remaining_conserved);
    synchronization_time_ = next.now();
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
    resident_entities_.push_back({std::move(transfer), {}, {}});
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
    validate_conserved_target(transfer, config_, synchronization_time_);
    const auto& transfer_id = coupling::transfer_header(transfer).transfer_id;
    if (!held_transfer_ids_.insert(transfer_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary bed already holds this transfer ID"};
    }
    resident_conserved_transfers_.push_back({std::move(transfer), {}, {}});
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
    return config_.perfused_path_count;
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

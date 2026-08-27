// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_circulation.hpp>

#include <mehlissa/core/error.hpp>

#include <unordered_set>
#include <utility>

namespace mehlissa::models::organ {
namespace {

void validate_config(const PulmonaryCirculationConfig& config) {
    if (config.component_name.empty() || config.model_id.empty() || config.entry_port_id.empty() ||
        config.exit_port_id.empty() || config.return_target_model_id.empty() ||
        config.return_target_port_id.empty() || config.regions.empty()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Pulmonary circulation requires IDs, ports, and regions"};
    }
    if (config.entry_port_id == config.exit_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Pulmonary entry and exit ports must be distinct"};
    }
    std::unordered_set<std::string> region_ids;
    for (const auto& region : config.regions) {
        if (region.id.empty() || !region_ids.insert(region.id).second ||
            region.transit_time <= core::SimulationClock::Duration::zero()) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Pulmonary regions require unique IDs and positive transit"};
        }
    }
}

void validate_conserved_target(const coupling::ConservedTransfer& transfer,
                               const PulmonaryCirculationConfig& config,
                               const core::SimulationClock::Duration synchronization_time) {
    coupling::validate_transfer(transfer);
    const auto& header = coupling::transfer_header(transfer);
    if (header.target_model_id != config.model_id ||
        header.target_port_id != config.entry_port_id ||
        header.emitted_at != synchronization_time) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Conserved transfer does not match pulmonary entry synchronization"};
    }
}

void route_conserved_return(coupling::ConservedTransfer& transfer,
                            const PulmonaryCirculationConfig& config,
                            const core::SimulationClock::Duration emitted_at) {
    auto& header = coupling::transfer_header(transfer);
    header.source_model_id = config.model_id;
    header.source_port_id = config.exit_port_id;
    header.target_model_id = config.return_target_model_id;
    header.target_port_id = config.return_target_port_id;
    header.emitted_at = emitted_at;
}

} // namespace

PulmonaryCirculation::PulmonaryCirculation(PulmonaryCirculationConfig config)
    : config_{std::move(config)} {
    validate_config(config_);
}

std::string_view PulmonaryCirculation::name() const noexcept { return config_.component_name; }
std::string_view PulmonaryCirculation::model_id() const noexcept { return config_.model_id; }
bool PulmonaryCirculation::accepts_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.entry_port_id;
}
bool PulmonaryCirculation::emits_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.exit_port_id;
}

void PulmonaryCirculation::initialize(core::SimulationContext& context) {
    if (state_ != State::building) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Pulmonary circulation can only initialize once"};
    }
    synchronization_time_ = context.clock().now();
    state_ = State::initialized;
}

void PulmonaryCirculation::advance(core::SimulationContext& context,
                                   const core::SimulationClock::Duration delta) {
    if (state_ != State::initialized || context.clock().now() != synchronization_time_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Pulmonary circulation is not synchronized"};
    }
    core::SimulationClock next{synchronization_time_};
    next.advance(delta);

    std::vector<ResidentEntity> remaining_residents;
    remaining_residents.reserve(residents_.size());
    for (auto& resident : residents_) {
        auto remaining_time = delta;
        while (remaining_time > core::SimulationClock::Duration::zero() &&
               resident.region_index < config_.regions.size()) {
            const auto required =
                config_.regions[resident.region_index].transit_time - resident.region_time;
            if (remaining_time < required) {
                resident.region_time += remaining_time;
                remaining_time = {};
            } else {
                remaining_time -= required;
                ++resident.region_index;
                resident.region_time = {};
            }
        }
        if (resident.region_index == config_.regions.size()) {
            outbound_.push_back({
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
            remaining_residents.push_back(std::move(resident));
        }
    }
    residents_ = std::move(remaining_residents);

    std::vector<ResidentConservedTransfer> remaining_conserved;
    remaining_conserved.reserve(resident_conserved_transfers_.size());
    for (auto& resident : resident_conserved_transfers_) {
        auto remaining_time = delta;
        while (remaining_time > core::SimulationClock::Duration::zero() &&
               resident.region_index < config_.regions.size()) {
            const auto required =
                config_.regions[resident.region_index].transit_time - resident.region_time;
            if (remaining_time < required) {
                resident.region_time += remaining_time;
                remaining_time = {};
            } else {
                remaining_time -= required;
                ++resident.region_index;
                resident.region_time = {};
            }
        }
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

void PulmonaryCirculation::finalize(core::SimulationContext&) noexcept {
    state_ = State::finalized;
}

void PulmonaryCirculation::accept_entity(coupling::EntityTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only initialized pulmonary circulation accepts entities"};
    }
    coupling::validate_entity_transfer(transfer);
    if (transfer.target_model_id != config_.model_id ||
        transfer.target_port_id != config_.entry_port_id ||
        transfer.emitted_at != synchronization_time_) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Entity does not match pulmonary entry synchronization"};
    }
    if (!held_entity_ids_.insert(transfer.entity_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Pulmonary circulation already holds this entity ID"};
    }
    residents_.push_back({std::move(transfer), 0, {}});
}

std::vector<coupling::EntityTransfer> PulmonaryCirculation::take_outbound_entities() {
    auto result = std::move(outbound_);
    outbound_.clear();
    for (const auto& transfer : result) {
        held_entity_ids_.erase(transfer.entity_id);
    }
    return result;
}

void PulmonaryCirculation::accept_conserved_transfer(coupling::ConservedTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{
            core::ErrorCode::lifecycle_invalid,
            "Only initialized pulmonary circulation accepts conserved transfers"};
    }
    validate_conserved_target(transfer, config_, synchronization_time_);
    const auto& transfer_id = coupling::transfer_header(transfer).transfer_id;
    if (!held_transfer_ids_.insert(transfer_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Pulmonary circulation already holds this transfer ID"};
    }
    resident_conserved_transfers_.push_back({std::move(transfer), 0, {}});
}

std::vector<coupling::ConservedTransfer> PulmonaryCirculation::take_outbound_conserved_transfers() {
    auto result = std::move(outbound_conserved_transfers_);
    outbound_conserved_transfers_.clear();
    for (const auto& transfer : result) {
        held_transfer_ids_.erase(coupling::transfer_header(transfer).transfer_id);
    }
    return result;
}

std::size_t PulmonaryCirculation::resident_conserved_transfer_count() const noexcept {
    return resident_conserved_transfers_.size();
}

std::size_t PulmonaryCirculation::region_count() const noexcept { return config_.regions.size(); }
std::size_t PulmonaryCirculation::resident_count() const noexcept { return residents_.size(); }

} // namespace mehlissa::models::organ

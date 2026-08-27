// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/lung_compartment.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <iterator>
#include <utility>

namespace mehlissa::models::organ {
namespace {

void validate_config(const LungCompartmentConfig& config) {
    if (config.component_name.empty() || config.model_id.empty() || config.entry_port_id.empty() ||
        config.exit_port_id.empty() || config.return_target_model_id.empty() ||
        config.return_target_port_id.empty()) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "A lung compartment requires non-empty component, model, and port identifiers"};
    }
    if (config.entry_port_id == config.exit_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Lung entry and exit ports must be distinct"};
    }
    if (config.transit_time <= core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Lung transit time must be positive"};
    }
}

void validate_conserved_target(const coupling::ConservedTransfer& transfer,
                               const LungCompartmentConfig& config,
                               const core::SimulationClock::Duration synchronization_time) {
    coupling::validate_transfer(transfer);
    const auto& header = coupling::transfer_header(transfer);
    if (header.target_model_id != config.model_id ||
        header.target_port_id != config.entry_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Conserved transfer does not target this lung entry port"};
    }
    if (header.emitted_at != synchronization_time) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Conserved transfer must be accepted at its declared synchronization time"};
    }
}

void route_conserved_return(coupling::ConservedTransfer& transfer,
                            const LungCompartmentConfig& config,
                            const core::SimulationClock::Duration emitted_at) {
    auto& header = coupling::transfer_header(transfer);
    header.source_model_id = config.model_id;
    header.source_port_id = config.exit_port_id;
    header.target_model_id = config.return_target_model_id;
    header.target_port_id = config.return_target_port_id;
    header.emitted_at = emitted_at;
}

} // namespace

LungCompartment::LungCompartment(LungCompartmentConfig config) : config_{std::move(config)} {
    validate_config(config_);
}

std::string_view LungCompartment::name() const noexcept { return config_.component_name; }

std::string_view LungCompartment::model_id() const noexcept { return config_.model_id; }

bool LungCompartment::accepts_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.entry_port_id;
}

bool LungCompartment::emits_entity_at(const std::string_view port_id) const noexcept {
    return port_id == config_.exit_port_id;
}

void LungCompartment::initialize(core::SimulationContext& context) {
    if (state_ != State::building) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "A lung compartment can only be initialized once"};
    }
    synchronization_time_ = context.clock().now();
    state_ = State::initialized;
}

void LungCompartment::advance(core::SimulationContext& context,
                              const core::SimulationClock::Duration delta) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only an initialized lung compartment can advance"};
    }
    if (context.clock().now() != synchronization_time_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Lung compartment and simulation clock are not synchronized"};
    }

    core::SimulationClock next{synchronization_time_};
    next.advance(delta);
    const auto completion_time = next.now();

    std::vector<ResidentEntity> remaining;
    remaining.reserve(residents_.size());
    for (auto& resident : residents_) {
        if (resident.residence_time >=
            config_.transit_time - std::min(delta, config_.transit_time)) {
            outbound_.push_back(coupling::EntityTransfer{
                std::string{coupling::entity_transfer_contract_version},
                resident.transfer.entity_id,
                std::move(resident.transfer.entity_type),
                config_.model_id,
                config_.exit_port_id,
                config_.return_target_model_id,
                config_.return_target_port_id,
                completion_time,
            });
            ++completed_count_;
        } else {
            resident.residence_time += delta;
            remaining.push_back(std::move(resident));
        }
    }
    residents_ = std::move(remaining);

    std::vector<ResidentConservedTransfer> remaining_conserved;
    remaining_conserved.reserve(resident_conserved_transfers_.size());
    for (auto& resident : resident_conserved_transfers_) {
        if (resident.residence_time >=
            config_.transit_time - std::min(delta, config_.transit_time)) {
            route_conserved_return(resident.transfer, config_, completion_time);
            outbound_conserved_transfers_.push_back(std::move(resident.transfer));
        } else {
            resident.residence_time += delta;
            remaining_conserved.push_back(std::move(resident));
        }
    }
    resident_conserved_transfers_ = std::move(remaining_conserved);
    synchronization_time_ = completion_time;
}

void LungCompartment::finalize(core::SimulationContext&) noexcept { state_ = State::finalized; }

void LungCompartment::accept_entity(coupling::EntityTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only an initialized lung compartment can accept entities"};
    }
    coupling::validate_entity_transfer(transfer);
    if (transfer.target_model_id != config_.model_id ||
        transfer.target_port_id != config_.entry_port_id) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Entity transfer does not target this lung entry port"};
    }
    if (transfer.emitted_at != synchronization_time_) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Entity transfer must be accepted at its declared synchronization time"};
    }
    if (!held_entity_ids_.insert(transfer.entity_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Lung compartment already holds this entity ID"};
    }

    residents_.push_back(ResidentEntity{std::move(transfer), {}});
    ++accepted_count_;
}

std::vector<coupling::EntityTransfer> LungCompartment::take_outbound_entities() {
    auto result = std::move(outbound_);
    outbound_.clear();
    for (const auto& transfer : result) {
        held_entity_ids_.erase(transfer.entity_id);
    }
    return result;
}

void LungCompartment::accept_conserved_transfer(coupling::ConservedTransfer transfer) {
    if (state_ != State::initialized) {
        throw core::MehlissaError{
            core::ErrorCode::lifecycle_invalid,
            "Only an initialized lung compartment can accept conserved transfers"};
    }
    validate_conserved_target(transfer, config_, synchronization_time_);
    const auto& transfer_id = coupling::transfer_header(transfer).transfer_id;
    if (!held_transfer_ids_.insert(transfer_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Lung compartment already holds this transfer ID"};
    }
    resident_conserved_transfers_.push_back({std::move(transfer), {}});
}

std::vector<coupling::ConservedTransfer> LungCompartment::take_outbound_conserved_transfers() {
    auto result = std::move(outbound_conserved_transfers_);
    outbound_conserved_transfers_.clear();
    for (const auto& transfer : result) {
        held_transfer_ids_.erase(coupling::transfer_header(transfer).transfer_id);
    }
    return result;
}

std::size_t LungCompartment::resident_conserved_transfer_count() const noexcept {
    return resident_conserved_transfers_.size();
}

std::size_t LungCompartment::resident_count() const noexcept { return residents_.size(); }

std::size_t LungCompartment::outbound_count() const noexcept { return outbound_.size(); }

std::uint64_t LungCompartment::accepted_count() const noexcept { return accepted_count_; }

std::uint64_t LungCompartment::completed_count() const noexcept { return completed_count_; }

} // namespace mehlissa::models::organ

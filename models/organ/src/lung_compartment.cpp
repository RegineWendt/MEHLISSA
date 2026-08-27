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

std::size_t LungCompartment::resident_count() const noexcept { return residents_.size(); }

std::size_t LungCompartment::outbound_count() const noexcept { return outbound_.size(); }

std::uint64_t LungCompartment::accepted_count() const noexcept { return accepted_count_; }

std::uint64_t LungCompartment::completed_count() const noexcept { return completed_count_; }

} // namespace mehlissa::models::organ

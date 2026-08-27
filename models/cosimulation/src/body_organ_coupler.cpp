// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/body_organ_coupler.hpp>

#include <mehlissa/core/error.hpp>

#include <iterator>
#include <utility>

namespace mehlissa::models::cosimulation {

BodyOrganCoupler::BodyOrganCoupler(body::CompartmentTransport& body,
                                   coupling::ModelComponent& organ, BodyOrganRoute route)
    : body_{body}, organ_{organ}, route_{std::move(route)} {
    if (route_.body_departure_segment_id.empty() || route_.body_departure_port_id.empty() ||
        route_.organ_entry_port_id.empty() || route_.organ_exit_port_id.empty() ||
        route_.body_return_port_id.empty() || route_.body_return_segment_id.empty()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "A body-organ route requires six non-empty identifiers"};
    }
    if (!organ_.accepts_entity_at(route_.organ_entry_port_id) ||
        !organ_.emits_entity_at(route_.organ_exit_port_id)) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Body-organ route is incompatible with the organ ports"};
    }
}

void BodyOrganCoupler::send_to_organ(const std::uint64_t entity_id,
                                     const core::SimulationClock::Duration time) {
    body_.handoff_particle(entity_id, route_.body_departure_segment_id);
    try {
        organ_.accept_entity({
            std::string{coupling::entity_transfer_contract_version},
            entity_id,
            "nanodevice",
            body_.graph().model_id,
            route_.body_departure_port_id,
            std::string{organ_.model_id()},
            route_.organ_entry_port_id,
            time,
        });
        in_flight_entity_ids_.insert(entity_id);
    } catch (...) {
        body_.receive_returned_particle(entity_id, route_.body_departure_segment_id, time);
        throw;
    }
}

std::size_t
BodyOrganCoupler::receive_from_organ(const core::SimulationClock::Duration synchronization_time) {
    auto outbound = organ_.take_outbound_entities();
    pending_returns_.insert(pending_returns_.end(), std::make_move_iterator(outbound.begin()),
                            std::make_move_iterator(outbound.end()));

    std::size_t received{};
    while (!pending_returns_.empty()) {
        const auto& transfer = pending_returns_.front();
        validate_return(transfer, synchronization_time);
        body_.receive_returned_particle(transfer.entity_id, route_.body_return_segment_id,
                                        transfer.emitted_at);
        in_flight_entity_ids_.erase(transfer.entity_id);
        pending_returns_.erase(pending_returns_.begin());
        ++received;
        ++completed_round_trip_count_;
    }
    return received;
}

std::size_t BodyOrganCoupler::in_flight_count() const noexcept {
    return in_flight_entity_ids_.size();
}

std::size_t BodyOrganCoupler::pending_return_count() const noexcept {
    return pending_returns_.size();
}

std::uint64_t BodyOrganCoupler::completed_round_trip_count() const noexcept {
    return completed_round_trip_count_;
}

void BodyOrganCoupler::validate_return(
    const coupling::EntityTransfer& transfer,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_entity_transfer(transfer);
    if (transfer.source_model_id != organ_.model_id() ||
        transfer.source_port_id != route_.organ_exit_port_id ||
        transfer.target_model_id != body_.graph().model_id ||
        transfer.target_port_id != route_.body_return_port_id ||
        !in_flight_entity_ids_.contains(transfer.entity_id) ||
        transfer.emitted_at != synchronization_time) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Organ return does not match the active body-organ route"};
    }
}

} // namespace mehlissa::models::cosimulation

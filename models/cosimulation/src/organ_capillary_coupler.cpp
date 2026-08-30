// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/organ_capillary_coupler.hpp>

#include <mehlissa/core/error.hpp>

#include <iterator>
#include <utility>

namespace mehlissa::models::cosimulation {
namespace {

void append(std::vector<coupling::EntityTransfer>& destination,
            std::vector<coupling::EntityTransfer> source) {
    destination.insert(destination.end(), std::make_move_iterator(source.begin()),
                       std::make_move_iterator(source.end()));
}

void append(std::vector<coupling::ConservedTransfer>& destination,
            std::vector<coupling::ConservedTransfer> source) {
    destination.insert(destination.end(), std::make_move_iterator(source.begin()),
                       std::make_move_iterator(source.end()));
}

void append(std::vector<coupling::EntityDispositionTransfer>& destination,
            std::vector<coupling::EntityDispositionTransfer> source) {
    destination.insert(destination.end(), std::make_move_iterator(source.begin()),
                       std::make_move_iterator(source.end()));
}

[[nodiscard]] std::size_t disposition_index(const coupling::EntityDispositionKind kind) noexcept {
    switch (kind) {
    case coupling::EntityDispositionKind::retained:
        return 0;
    case coupling::EntityDispositionKind::adhered:
        return 1;
    case coupling::EntityDispositionKind::extravasated:
        return 2;
    }
    return 0;
}

} // namespace

OrganCapillaryCoupler::OrganCapillaryCoupler(OrganCapillaryEndpoints endpoints,
                                             OrganCapillaryRoute route)
    : organ_{endpoints.organ}, capillary_{endpoints.capillary}, route_{std::move(route)} {
    if (route_.organ_departure_port_id.empty() || route_.capillary_entry_port_id.empty() ||
        route_.capillary_exit_port_id.empty() || route_.organ_return_port_id.empty()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "An organ-capillary route requires four non-empty ports"};
    }
    if (organ_.model_id() == capillary_.model_id() ||
        route_.organ_departure_port_id == route_.organ_return_port_id ||
        route_.capillary_entry_port_id == route_.capillary_exit_port_id ||
        !organ_.emits_entity_at(route_.organ_departure_port_id) ||
        !organ_.accepts_entity_at(route_.organ_return_port_id) ||
        !capillary_.accepts_entity_at(route_.capillary_entry_port_id) ||
        !capillary_.emits_entity_at(route_.capillary_exit_port_id)) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Organ-capillary route is incompatible with its endpoints"};
    }
}

CoupledTransferCounts OrganCapillaryCoupler::transfer_to_capillary(
    const core::SimulationClock::Duration synchronization_time) {
    append(pending_entity_departures_, organ_.take_outbound_entities());
    append(pending_conserved_departures_, organ_.take_outbound_conserved_transfers());

    CoupledTransferCounts transferred{};
    while (!pending_entity_departures_.empty()) {
        const auto& transfer = pending_entity_departures_.front();
        validate_entity_departure(transfer, synchronization_time);
        capillary_.accept_entity(transfer);
        outstanding_entity_ids_.insert(transfer.entity_id);
        pending_entity_departures_.erase(pending_entity_departures_.begin());
        ++transferred.entities;
    }
    while (!pending_conserved_departures_.empty()) {
        const auto& transfer = pending_conserved_departures_.front();
        validate_conserved_departure(transfer, synchronization_time);
        capillary_.accept_conserved_transfer(transfer);
        outstanding_conserved_transfer_ids_.insert(coupling::transfer_header(transfer).transfer_id);
        pending_conserved_departures_.erase(pending_conserved_departures_.begin());
        ++transferred.conserved_transfers;
    }
    return transferred;
}

CoupledTransferCounts OrganCapillaryCoupler::transfer_to_organ(
    const core::SimulationClock::Duration synchronization_time) {
    append(pending_entity_returns_, capillary_.take_outbound_entities());
    append(pending_conserved_returns_, capillary_.take_outbound_conserved_transfers());

    CoupledTransferCounts transferred{};
    while (!pending_entity_returns_.empty()) {
        const auto& transfer = pending_entity_returns_.front();
        validate_entity_return(transfer, synchronization_time);
        organ_.accept_entity(transfer);
        outstanding_entity_ids_.erase(transfer.entity_id);
        pending_entity_returns_.erase(pending_entity_returns_.begin());
        ++transferred.entities;
        ++completed_entity_round_trips_;
    }
    while (!pending_conserved_returns_.empty()) {
        const auto& transfer = pending_conserved_returns_.front();
        validate_conserved_return(transfer, synchronization_time);
        organ_.accept_conserved_transfer(transfer);
        outstanding_conserved_transfer_ids_.erase(coupling::transfer_header(transfer).transfer_id);
        pending_conserved_returns_.erase(pending_conserved_returns_.begin());
        ++transferred.conserved_transfers;
        ++completed_conserved_round_trips_;
    }
    return transferred;
}

std::size_t OrganCapillaryCoupler::transfer_terminal_dispositions(
    coupling::EntityDispositionSource& source, coupling::EntityDispositionSink& sink,
    const core::SimulationClock::Duration synchronization_time) {
    append(pending_terminal_dispositions_, source.take_outbound_entity_dispositions());

    std::size_t transferred{};
    while (!pending_terminal_dispositions_.empty()) {
        const auto& transfer = pending_terminal_dispositions_.front();
        validate_terminal_disposition(transfer, sink, synchronization_time);
        sink.accept_entity_disposition(transfer);
        outstanding_entity_ids_.erase(transfer.entity_id);
        ++completed_terminal_dispositions_.at(disposition_index(transfer.disposition));
        pending_terminal_dispositions_.erase(pending_terminal_dispositions_.begin());
        ++transferred;
    }
    return transferred;
}

std::size_t OrganCapillaryCoupler::outstanding_entity_count() const noexcept {
    return outstanding_entity_ids_.size();
}

std::size_t OrganCapillaryCoupler::outstanding_conserved_transfer_count() const noexcept {
    return outstanding_conserved_transfer_ids_.size();
}

std::size_t OrganCapillaryCoupler::pending_departure_count() const noexcept {
    return pending_entity_departures_.size() + pending_conserved_departures_.size();
}

std::size_t OrganCapillaryCoupler::pending_return_count() const noexcept {
    return pending_entity_returns_.size() + pending_conserved_returns_.size();
}

std::size_t OrganCapillaryCoupler::pending_terminal_disposition_count() const noexcept {
    return pending_terminal_dispositions_.size();
}

std::uint64_t OrganCapillaryCoupler::completed_entity_round_trip_count() const noexcept {
    return completed_entity_round_trips_;
}

std::uint64_t OrganCapillaryCoupler::completed_conserved_round_trip_count() const noexcept {
    return completed_conserved_round_trips_;
}

std::uint64_t OrganCapillaryCoupler::completed_terminal_disposition_count() const noexcept {
    return completed_terminal_dispositions_[0] + completed_terminal_dispositions_[1] +
           completed_terminal_dispositions_[2];
}

std::uint64_t OrganCapillaryCoupler::completed_terminal_disposition_count(
    const coupling::EntityDispositionKind kind) const noexcept {
    return completed_terminal_dispositions_[disposition_index(kind)];
}

void OrganCapillaryCoupler::validate_entity_departure(
    const coupling::EntityTransfer& transfer,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_entity_transfer(transfer);
    if (transfer.source_model_id != organ_.model_id() ||
        transfer.source_port_id != route_.organ_departure_port_id ||
        transfer.target_model_id != capillary_.model_id() ||
        transfer.target_port_id != route_.capillary_entry_port_id ||
        transfer.emitted_at != synchronization_time ||
        outstanding_entity_ids_.contains(transfer.entity_id)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Organ departure does not match the active capillary route"};
    }
}

void OrganCapillaryCoupler::validate_entity_return(
    const coupling::EntityTransfer& transfer,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_entity_transfer(transfer);
    if (transfer.source_model_id != capillary_.model_id() ||
        transfer.source_port_id != route_.capillary_exit_port_id ||
        transfer.target_model_id != organ_.model_id() ||
        transfer.target_port_id != route_.organ_return_port_id ||
        transfer.emitted_at != synchronization_time ||
        !outstanding_entity_ids_.contains(transfer.entity_id)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Capillary return does not match an organ departure"};
    }
}

void OrganCapillaryCoupler::validate_conserved_departure(
    const coupling::ConservedTransfer& transfer,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_transfer(transfer);
    const auto& header = coupling::transfer_header(transfer);
    if (header.source_model_id != organ_.model_id() ||
        header.source_port_id != route_.organ_departure_port_id ||
        header.target_model_id != capillary_.model_id() ||
        header.target_port_id != route_.capillary_entry_port_id ||
        header.emitted_at != synchronization_time ||
        outstanding_conserved_transfer_ids_.contains(header.transfer_id)) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Conserved organ departure does not match the active capillary route"};
    }
}

void OrganCapillaryCoupler::validate_conserved_return(
    const coupling::ConservedTransfer& transfer,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_transfer(transfer);
    const auto& header = coupling::transfer_header(transfer);
    if (header.source_model_id != capillary_.model_id() ||
        header.source_port_id != route_.capillary_exit_port_id ||
        header.target_model_id != organ_.model_id() ||
        header.target_port_id != route_.organ_return_port_id ||
        header.emitted_at != synchronization_time ||
        !outstanding_conserved_transfer_ids_.contains(header.transfer_id)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Conserved capillary return does not match an organ departure"};
    }
}

void OrganCapillaryCoupler::validate_terminal_disposition(
    const coupling::EntityDispositionTransfer& transfer,
    const coupling::EntityDispositionSink& sink,
    const core::SimulationClock::Duration synchronization_time) const {
    coupling::validate_entity_disposition(transfer);
    if (transfer.source_model_id != capillary_.model_id() ||
        transfer.source_port_id == route_.capillary_exit_port_id ||
        transfer.target_model_id != sink.disposition_model_id() ||
        !sink.accepts_disposition_at(transfer.target_compartment_id) ||
        transfer.decided_at != synchronization_time ||
        !outstanding_entity_ids_.contains(transfer.entity_id)) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Terminal disposition does not close an outstanding capillary entity route"};
    }
}

} // namespace mehlissa::models::cosimulation

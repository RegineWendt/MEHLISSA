// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/coupling/conserved_transfer.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <string>

namespace mehlissa::models::coupling {
namespace {

void validate_header(const TransferHeader& header) {
    if (header.contract_version != conserved_transfer_contract_version ||
        header.transfer_id.empty() || header.source_model_id.empty() ||
        header.source_port_id.empty() || header.target_model_id.empty() ||
        header.target_port_id.empty()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Conserved transfer has an invalid version, ID, or route"};
    }
    if (header.emitted_at < core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Conserved transfer cannot precede simulation time zero"};
    }
}

template <typename Transfer> struct RecordPair final {
    const std::unordered_map<std::string, Transfer>& sent;
    const std::unordered_map<std::string, Transfer>& received;
};

template <typename Transfer>
[[nodiscard]] std::size_t unmatched(const RecordPair<Transfer> records) {
    std::size_t result{};
    for (const auto& [id, transfer] : records.sent) {
        const auto match = records.received.find(id);
        if (match == records.received.end() || match->second != transfer) {
            ++result;
        }
    }
    for (const auto& [id, transfer] : records.received) {
        const auto match = records.sent.find(id);
        if (match == records.sent.end() || match->second != transfer) {
            ++result;
        }
    }
    return result;
}

} // namespace

void validate_transfer(const PopulationTransfer& transfer) {
    validate_header(transfer.header);
    if (transfer.population_type.empty() || transfer.count == 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Population transfer requires a type and positive count"};
    }
}

void validate_transfer(const SubstanceAmountTransfer& transfer) {
    validate_header(transfer.header);
    if (transfer.substance_id.empty() || !std::isfinite(core::in_moles(transfer.amount)) ||
        core::in_moles(transfer.amount) <= 0.0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Substance transfer requires an ID and positive finite amount"};
    }
}

void validate_transfer(const VolumeFlowTransfer& transfer) {
    validate_header(transfer.header);
    if (!std::isfinite(core::in_cubic_meters_per_second(transfer.flow_rate)) ||
        core::in_cubic_meters_per_second(transfer.flow_rate) <= 0.0 ||
        transfer.interval <= core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Flow transfer requires positive finite flow and interval"};
    }
}

core::Volume integrated_volume(const VolumeFlowTransfer& transfer) {
    validate_transfer(transfer);
    const auto seconds = static_cast<double>(transfer.interval.count()) / 1'000'000'000.0;
    return core::cubic_meters(core::in_cubic_meters_per_second(transfer.flow_rate) * seconds);
}

template <typename Transfer>
void ConservationLedger::record(const Transfer& transfer,
                                std::unordered_map<std::string, Transfer>& records,
                                const std::string_view role) {
    validate_transfer(transfer);
    if (!records.emplace(transfer.header.transfer_id, transfer).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Duplicate " + std::string{role} + " transfer ID"};
    }
}

void ConservationLedger::record_sent(const PopulationTransfer& transfer) {
    record(transfer, populations_sent_, "sent population");
}
void ConservationLedger::record_received(const PopulationTransfer& transfer) {
    record(transfer, populations_received_, "received population");
}
void ConservationLedger::record_sent(const SubstanceAmountTransfer& transfer) {
    record(transfer, substances_sent_, "sent substance");
}
void ConservationLedger::record_received(const SubstanceAmountTransfer& transfer) {
    record(transfer, substances_received_, "received substance");
}
void ConservationLedger::record_sent(const VolumeFlowTransfer& transfer) {
    record(transfer, flows_sent_, "sent flow");
}
void ConservationLedger::record_received(const VolumeFlowTransfer& transfer) {
    record(transfer, flows_received_, "received flow");
}

std::size_t ConservationLedger::outstanding_transfer_count() const noexcept {
    return unmatched(RecordPair{populations_sent_, populations_received_}) +
           unmatched(RecordPair{substances_sent_, substances_received_}) +
           unmatched(RecordPair{flows_sent_, flows_received_});
}

void ConservationLedger::verify_balanced() const {
    if (outstanding_transfer_count() != 0) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Cross-model population, substance, or flow ledger is unbalanced"};
    }
}

} // namespace mehlissa::models::coupling

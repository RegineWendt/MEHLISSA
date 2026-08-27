// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <mehlissa/core/error.hpp>

#include <string>

namespace mehlissa::models::coupling {

void validate_entity_transfer(const EntityTransfer& transfer) {
    if (transfer.contract_version != entity_transfer_contract_version) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Unsupported entity-transfer contract version: " +
                                      transfer.contract_version};
    }
    if (transfer.entity_id == 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "An entity transfer requires a non-zero entity ID"};
    }
    if (transfer.entity_type.empty() || transfer.source_model_id.empty() ||
        transfer.source_port_id.empty() || transfer.target_model_id.empty() ||
        transfer.target_port_id.empty()) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "An entity transfer requires non-empty type, model, and port identifiers"};
    }
    if (transfer.emitted_at < core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "An entity transfer cannot precede simulation time zero"};
    }
}

} // namespace mehlissa::models::coupling

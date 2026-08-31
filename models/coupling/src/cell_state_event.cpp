// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/coupling/cell_state_event.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>

namespace mehlissa::models::coupling {

void validate_cell_state_event(const CellStateEvent& event) {
    if (event.contract_version != cell_state_event_contract_version || event.event_id.empty() ||
        event.event_type.empty() || event.source_model_id.empty() || event.source_cell_id.empty() ||
        event.source_request_id.empty() || event.target_model_id.empty() ||
        event.target_port_id.empty() ||
        event.occurred_at < core::SimulationClock::Duration::zero() || event.measure_name.empty() ||
        !std::isfinite(event.measure_value)) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Cell-state event is incomplete or invalid"};
    }
}

} // namespace mehlissa::models::coupling

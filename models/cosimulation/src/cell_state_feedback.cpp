// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/cell_state_feedback.hpp>

#include <mehlissa/core/error.hpp>

namespace mehlissa::models::cosimulation {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

} // namespace

void validate_cell_state_feedback_config(const CellStateFeedbackConfig& config) {
    if (config.event_id.empty() || config.source_model_id.empty() ||
        config.source_cell_id.empty() || config.target_model_id.empty() ||
        config.target_port_id.empty()) {
        invalid("Cell-state feedback configuration is incomplete");
    }
}

std::optional<coupling::CellStateEvent>
make_cell_state_feedback_event(const cell::ApoptosisResponse& response,
                               const CellStateFeedbackConfig& config) {
    cell::validate_apoptosis_response(response);
    validate_cell_state_feedback_config(config);
    if (response.model_id != config.source_model_id || response.cell_id != config.source_cell_id) {
        invalid("Cell-state response does not match the feedback source");
    }
    if (response.state != cell::CellState::apoptosis_committed) {
        return std::nullopt;
    }
    coupling::CellStateEvent event{
        std::string{coupling::cell_state_event_contract_version},
        config.event_id,
        std::string{coupling::apoptosis_committed_event_type},
        response.model_id,
        response.cell_id,
        response.request_id,
        config.target_model_id,
        config.target_port_id,
        response.observed_at,
        "synthetic_effect_fraction",
        response.effect_fraction,
    };
    coupling::validate_cell_state_event(event);
    return event;
}

} // namespace mehlissa::models::cosimulation

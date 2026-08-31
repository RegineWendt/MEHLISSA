// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_CELL_STATE_FEEDBACK_HPP
#define MEHLISSA_MODELS_COSIMULATION_CELL_STATE_FEEDBACK_HPP

#include <mehlissa/models/cell/apoptosis_response.hpp>
#include <mehlissa/models/coupling/cell_state_event.hpp>

#include <optional>
#include <string>

namespace mehlissa::models::cosimulation {

struct CellStateFeedbackConfig final {
    std::string event_id;
    std::string source_model_id;
    std::string source_cell_id;
    std::string target_model_id;
    std::string target_port_id;
};

void validate_cell_state_feedback_config(const CellStateFeedbackConfig& config);

[[nodiscard]] std::optional<coupling::CellStateEvent>
make_cell_state_feedback_event(const cell::ApoptosisResponse& response,
                               const CellStateFeedbackConfig& config);

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_CELL_STATE_FEEDBACK_HPP

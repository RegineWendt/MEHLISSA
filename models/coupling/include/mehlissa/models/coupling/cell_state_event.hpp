// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_CELL_STATE_EVENT_HPP
#define MEHLISSA_MODELS_COUPLING_CELL_STATE_EVENT_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <string>
#include <string_view>

namespace mehlissa::models::coupling {

inline constexpr std::string_view cell_state_event_contract_version = "1.0.0";
inline constexpr std::string_view apoptosis_committed_event_type = "cell.apoptosis_committed";

struct CellStateEvent final {
    std::string contract_version;
    std::string event_id;
    std::string event_type;
    std::string source_model_id;
    std::string source_cell_id;
    std::string source_request_id;
    std::string target_model_id;
    std::string target_port_id;
    core::SimulationClock::Duration occurred_at{};
    std::string measure_name;
    double measure_value{};

    [[nodiscard]] bool operator==(const CellStateEvent&) const noexcept = default;
};

void validate_cell_state_event(const CellStateEvent& event);

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_CELL_STATE_EVENT_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_D_COMMUNICATION_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_D_COMMUNICATION_HPP

#include <mehlissa/scenarios/fingerprinting/level_c_assembly.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

struct LevelDCommunicationResult final {
    std::string local_collection_message_id;
    std::string collector_uplink_message_id;
    std::string gateway_measurement_id;
    std::string ban_frame_id;
    std::string external_report_id;
    std::vector<std::string> local_collection_route;
    std::vector<std::string> collector_uplink_route;
    core::SimulationClock::Duration local_collection_completed_at{};
    core::SimulationClock::Duration collector_uplink_completed_at{};
    core::SimulationClock::Duration gateway_measurement_at{};
    core::SimulationClock::Duration external_report_at{};
    std::uint64_t attempted_local_messages{};
    std::uint64_t delivered_local_messages{};
    std::uint64_t attempted_ban_frames{};
    std::uint64_t delivered_ban_frames{};
    core::Energy local_transmitter_energy{};
    core::Energy local_receiver_energy{};
    core::Energy local_link_energy{};
    core::Energy ban_transmitter_energy{};
    core::Energy ban_receiver_energy{};
    core::Energy ban_link_energy{};
    std::string qualification;

    [[nodiscard]] bool operator==(const LevelDCommunicationResult&) const noexcept = default;
};

[[nodiscard]] LevelDCommunicationResult
run_level_d_communication(const LevelAPlan& plan, const LevelCAssemblyResult& assembly);
void apply_level_d_communication(LevelARuntimeResult& runtime,
                                 const LevelDCommunicationResult& communication);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_D_COMMUNICATION_HPP

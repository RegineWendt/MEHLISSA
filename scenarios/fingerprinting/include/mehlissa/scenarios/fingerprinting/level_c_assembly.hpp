// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_C_ASSEMBLY_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_C_ASSEMBLY_HPP

#include <mehlissa/scenarios/fingerprinting/level_b_detection.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

struct FingerprintTileRelease final {
    std::string release_id;
    std::string payload_id;
    std::string tile_id;
    std::string source_detection_event_id;
    core::SimulationClock::Duration released_at{};

    [[nodiscard]] bool operator==(const FingerprintTileRelease&) const noexcept = default;
};

struct LevelCAssemblyInput final {
    std::uint64_t available_detected_locators{};
    std::uint64_t required_unique_tiles{};
    core::SimulationClock::Duration assembly_duration{};
};

struct LevelCAssemblyResult final {
    std::string assembly_id;
    std::string fingerprint_id;
    std::vector<FingerprintTileRelease> releases;
    std::uint64_t required_unique_tiles{};
    bool complete{};
    core::SimulationClock::Duration started_at{};
    std::optional<core::SimulationClock::Duration> completed_at;
    std::string qualification;

    [[nodiscard]] bool operator==(const LevelCAssemblyResult&) const noexcept = default;
};

[[nodiscard]] LevelCAssemblyInput default_level_c_assembly_input(const LevelAPlan& plan);
[[nodiscard]] LevelCAssemblyResult run_level_c_assembly(const LevelAPlan& plan,
                                                        const LevelBDetectionResult& detection,
                                                        const LevelCAssemblyInput& input);
void apply_level_c_assembly(LevelARuntimeResult& runtime, const LevelCAssemblyResult& assembly);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_C_ASSEMBLY_HPP

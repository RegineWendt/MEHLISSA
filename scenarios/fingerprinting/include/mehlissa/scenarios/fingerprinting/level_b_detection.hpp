// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_B_DETECTION_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_B_DETECTION_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/scenarios/fingerprinting/runtime_coordinator.hpp>

#include <optional>
#include <string>

namespace mehlissa::scenarios::fingerprinting {

struct LevelBDetectionInput final {
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration exposure_duration{};
    double initial_bound_fraction{};
};

struct LevelBDetectionResult final {
    std::string receptor_profile_id;
    std::string receptor_model_id;
    std::string detector_device_id;
    std::string event_id;
    std::string signal_id;
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration exposure_duration{};
    double equilibrium_bound_fraction{};
    double final_bound_fraction{};
    bool detected{};
    std::optional<core::SimulationClock::Duration> relative_threshold_crossing;
    std::optional<core::SimulationClock::Duration> absolute_detection_time;

    [[nodiscard]] bool operator==(const LevelBDetectionResult&) const noexcept = default;
};

[[nodiscard]] LevelBDetectionInput default_level_b_detection_input(const LevelAPlan& plan);
[[nodiscard]] LevelBDetectionResult run_level_b_detection(const LevelAPlan& plan,
                                                          const LevelBDetectionInput& input);
void apply_level_b_detection(LevelARuntimeResult& runtime, const LevelBDetectionResult& detection);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_B_DETECTION_HPP

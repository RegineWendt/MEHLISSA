// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_A_COMPOSER_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_A_COMPOSER_HPP

#include <mehlissa/experiment/fingerprint_timer_baseline.hpp>
#include <mehlissa/scenarios/fingerprinting/scenario_profile.hpp>

#include <filesystem>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

struct ResolvedArtifact final {
    ArtifactRole role{};
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

struct LevelAPlan final {
    ScenarioProfile profile;
    std::vector<ResolvedArtifact> artifacts;
    experiment::FingerprintTimerBaseline timer_baseline;
    experiment::FingerprintTimerRun timer_run;
};

[[nodiscard]] LevelAPlan compose_level_a_plan(const ScenarioProfile& profile,
                                              const std::filesystem::path& repository_root);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_A_COMPOSER_HPP

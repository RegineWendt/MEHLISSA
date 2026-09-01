// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_RUNTIME_COORDINATOR_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_RUNTIME_COORDINATOR_HPP

#include <mehlissa/scenarios/fingerprinting/level_a_composer.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

inline constexpr std::string_view fingerprinting_runtime_contract_version = "1.0.0";

enum class ExecutionBasis : std::uint8_t { model_execution, historical_timer, software_surrogate };

[[nodiscard]] std::string_view to_string(ExecutionBasis basis) noexcept;

struct RuntimeComponentIdentity final {
    ArtifactRole role{};
    std::string profile_id;
    std::string model_id;
    bool instantiated{};
    bool advanced{};

    [[nodiscard]] bool operator==(const RuntimeComponentIdentity&) const noexcept = default;
};

struct StageTraceEntry final {
    std::size_t ordinal{};
    StageKind stage{};
    core::SimulationClock::Duration time{};
    ExecutionBasis basis{ExecutionBasis::historical_timer};
    std::string component_id;
    std::string input_identity;
    std::string output_identity;
    std::string qualification;

    [[nodiscard]] bool operator==(const StageTraceEntry&) const noexcept = default;
};

struct LevelARuntimeResult final {
    std::string contract_version;
    std::string scenario_id;
    std::string run_id;
    std::uint64_t master_seed{};
    std::vector<RuntimeComponentIdentity> components;
    std::vector<StageTraceEntry> stages;
    core::SimulationClock::Duration component_probe_duration{};

    [[nodiscard]] bool operator==(const LevelARuntimeResult&) const noexcept = default;
};

[[nodiscard]] LevelARuntimeResult run_level_a_runtime(const LevelAPlan& plan);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_RUNTIME_COORDINATOR_HPP

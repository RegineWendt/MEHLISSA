// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_E_ANALYSIS_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_E_ANALYSIS_HPP

#include <mehlissa/scenarios/fingerprinting/level_b_detection.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

enum class ClassificationKind : std::uint8_t {
    true_positive,
    true_negative,
    false_positive,
    false_negative
};

[[nodiscard]] std::string_view to_string(ClassificationKind classification) noexcept;

struct LevelECase final {
    std::string case_id;
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration exposure_duration{};
    bool target_present{};
};

struct LevelECaseResult final {
    std::string case_id;
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration exposure_duration{};
    bool target_present{};
    bool detected{};
    double final_bound_fraction{};
    ClassificationKind classification{ClassificationKind::true_negative};

    [[nodiscard]] bool operator==(const LevelECaseResult&) const noexcept = default;
};

struct ProportionInterval final {
    double estimate{};
    double lower_95{};
    double upper_95{};

    [[nodiscard]] bool operator==(const ProportionInterval&) const noexcept = default;
};

struct LevelEClassificationSummary final {
    std::uint64_t true_positive{};
    std::uint64_t true_negative{};
    std::uint64_t false_positive{};
    std::uint64_t false_negative{};
    std::optional<ProportionInterval> sensitivity;
    std::optional<ProportionInterval> specificity;
    std::optional<ProportionInterval> false_positive_rate;
    std::optional<ProportionInterval> false_negative_rate;

    [[nodiscard]] bool operator==(const LevelEClassificationSummary&) const noexcept = default;
};

struct LevelEAnalysisResult final {
    std::string analysis_id;
    std::vector<LevelECaseResult> cases;
    LevelEClassificationSummary summary;
    std::vector<std::string> varied_parameters;
    std::vector<std::string> limitations;

    [[nodiscard]] bool operator==(const LevelEAnalysisResult&) const noexcept = default;
};

[[nodiscard]] std::vector<LevelECase> default_level_e_cases(const LevelAPlan& plan);
[[nodiscard]] LevelEAnalysisResult run_level_e_analysis(const LevelAPlan& plan,
                                                        const std::vector<LevelECase>& cases);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_LEVEL_E_ANALYSIS_HPP

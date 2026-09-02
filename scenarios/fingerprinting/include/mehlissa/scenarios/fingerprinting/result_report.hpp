// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_RESULT_REPORT_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_RESULT_REPORT_HPP

#include <mehlissa/scenarios/fingerprinting/level_d_communication.hpp>
#include <mehlissa/scenarios/fingerprinting/level_e_analysis.hpp>
#include <mehlissa/scenarios/fingerprinting/runtime_coordinator.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

inline constexpr std::string_view fingerprinting_result_schema_version = "1.0.0";
inline constexpr std::string_view holistic_fingerprinting_result_schema_version = "2.0.0";

struct ResultArtifactManifestEntry final {
    ArtifactRole role{};
    std::filesystem::path definition_path;
    std::string definition_sha256;
    std::filesystem::path schema_path;
    std::string schema_sha256;

    [[nodiscard]] bool operator==(const ResultArtifactManifestEntry&) const noexcept = default;
};

struct FingerprintingResultReport final {
    std::string schema_version;
    ScenarioIdentity scenario;
    RunIdentity run;
    FingerprintTarget target;
    std::vector<ResultArtifactManifestEntry> artifacts;
    LevelARuntimeResult runtime;
    bool deterministic_replay_required{};
    bool clinical_validation_claim{};
    std::vector<std::string> limitations;

    [[nodiscard]] bool operator==(const FingerprintingResultReport&) const noexcept = default;
};

[[nodiscard]] FingerprintingResultReport
make_fingerprinting_result_report(const LevelAPlan& plan, const LevelARuntimeResult& runtime);

struct ResultReportPaths final {
    std::filesystem::path output;
    std::filesystem::path schema;
};

void write_fingerprinting_result_report(const FingerprintingResultReport& report,
                                        const ResultReportPaths& paths);

struct HolisticFingerprintingResultReport final {
    FingerprintingResultReport reproducibility;
    LevelBDetectionResult detection;
    LevelCAssemblyResult assembly;
    LevelDCommunicationResult communication;
    LevelEAnalysisResult analysis;

    [[nodiscard]] bool
    operator==(const HolisticFingerprintingResultReport&) const noexcept = default;
};

[[nodiscard]] HolisticFingerprintingResultReport
run_holistic_fingerprinting_scenario(const LevelAPlan& plan,
                                     const std::vector<LevelECase>& analysis_cases);

void write_holistic_fingerprinting_result_report(const HolisticFingerprintingResultReport& report,
                                                 const ResultReportPaths& paths);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_RESULT_REPORT_HPP

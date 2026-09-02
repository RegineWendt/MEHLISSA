// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FINGERPRINTING_SCENARIO_PROFILE_HPP
#define MEHLISSA_SCENARIOS_FINGERPRINTING_SCENARIO_PROFILE_HPP

#include <mehlissa/core/error.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::scenarios::fingerprinting {

inline constexpr std::string_view scenario_profile_schema_version = "1.0.0";

enum class ArtifactRole : std::uint8_t {
    body_model,
    body_state,
    organ_model,
    capillary_model,
    capillary_cell_signal,
    receptor_model,
    locator_device,
    collector_device,
    communication_cluster,
    gateway_endpoint,
    active_gateway,
    ban_station,
    timer_baseline
};

enum class StageKind : std::uint8_t {
    injection,
    body_transport,
    organ_transfer,
    capillary_localization,
    molecular_recognition,
    fingerprint_assembly,
    local_collection,
    collector_return,
    gateway_measurement,
    external_report
};

[[nodiscard]] std::string_view to_string(ArtifactRole role) noexcept;
[[nodiscard]] std::string_view to_string(StageKind stage) noexcept;

struct ArtifactReference final {
    ArtifactRole role{};
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;

    [[nodiscard]] bool operator==(const ArtifactReference&) const noexcept = default;
};

struct ScenarioIdentity final {
    std::string id;
    std::string version;
    std::string title;
    std::string acceptance_level;

    [[nodiscard]] bool operator==(const ScenarioIdentity&) const noexcept = default;
};

struct RunIdentity final {
    std::string id;
    std::uint64_t master_seed{};
    std::uint64_t collector_count{};

    [[nodiscard]] bool operator==(const RunIdentity&) const noexcept = default;
};

struct FingerprintTarget final {
    std::string fingerprint_id;
    std::string tissue;
    std::string region_id;

    [[nodiscard]] bool operator==(const FingerprintTarget&) const noexcept = default;
};

struct ScenarioAcceptance final {
    std::vector<StageKind> required_stage_order;
    bool deterministic_replay_required{};
    bool clinical_validation_claim{};

    [[nodiscard]] bool operator==(const ScenarioAcceptance&) const noexcept = default;
};

struct ScenarioSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;

    [[nodiscard]] bool operator==(const ScenarioSource&) const noexcept = default;
};

struct ScenarioProfile final {
    std::string schema_version;
    ScenarioIdentity scenario;
    RunIdentity run;
    FingerprintTarget target;
    std::vector<ArtifactReference> artifacts;
    ScenarioAcceptance acceptance;
    std::vector<ScenarioSource> sources;
    std::vector<std::string> limitations;

    [[nodiscard]] bool operator==(const ScenarioProfile&) const noexcept = default;
};

struct ScenarioProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

class ScenarioProfileError final : public core::MehlissaError {
  public:
    using core::MehlissaError::MehlissaError;
};

void validate_scenario_profile(const ScenarioProfile& profile);

[[nodiscard]] ScenarioProfile load_scenario_profile(const ScenarioProfileLoadRequest& request);

} // namespace mehlissa::scenarios::fingerprinting

#endif // MEHLISSA_SCENARIOS_FINGERPRINTING_SCENARIO_PROFILE_HPP

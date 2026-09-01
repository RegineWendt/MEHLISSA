// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_RESILIENCE_SCENARIO_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_RESILIENCE_SCENARIO_PROFILE_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view resilience_scenario_profile_schema_version = "1.0.0";

enum class ResilienceInjection : std::uint8_t {
    uplink_loss,
    uplink_corruption,
    frame_expiry,
    unauthorized_target,
    disallowed_content_type,
    correlation_mismatch,
    duplicate_request,
    station_command_capacity,
    gateway_command_replay,
    external_response_identity_mismatch,
    external_attempt_capacity,
    local_resource_exhaustion
};

enum class ResilienceDisposition : std::uint8_t {
    lost,
    corrupted,
    expired,
    denied_target,
    denied_content_type,
    denied_correlation_mismatch,
    denied_duplicate_request,
    denied_capacity,
    rejected_invariant,
    rejected_resource
};

[[nodiscard]] std::string_view to_string(ResilienceInjection injection) noexcept;
[[nodiscard]] std::string_view to_string(ResilienceDisposition disposition) noexcept;

struct ResilienceScenario final {
    std::string scenario_id;
    std::string category;
    ResilienceInjection injection{ResilienceInjection::uplink_loss};
    std::string expected_boundary;
    ResilienceDisposition expected_disposition{ResilienceDisposition::lost};
    bool protected_state_unchanged{};
    bool communication_metrics_accounted{};
};

struct ResilienceSecurityScope final {
    std::string threat_model;
    std::vector<std::string> protected_properties;
    std::vector<std::string> excluded_claims;
};

struct ResilienceScenarioSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct ResilienceScenarioProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string baseline_profile_id;
    std::vector<ResilienceScenario> scenarios;
    ResilienceSecurityScope security_scope;
    std::vector<ResilienceScenarioSource> sources;
    std::vector<std::string> limitations;
};

struct ResilienceScenarioProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_resilience_scenario_profile(const ResilienceScenarioProfile& profile);

[[nodiscard]] ResilienceScenarioProfile
load_resilience_scenario_profile(const ResilienceScenarioProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_RESILIENCE_SCENARIO_PROFILE_HPP

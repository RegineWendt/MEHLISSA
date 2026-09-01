// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/resilience_scenario_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <array>
#include <fstream>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::iot {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot open " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid, "Invalid JSON in " + std::string{role} + " '" +
                                                   path.string() + "': " + error.what());
    }
}

[[nodiscard]] ResilienceInjection decode_injection(const std::string_view value) {
    if (value == "uplink_loss") {
        return ResilienceInjection::uplink_loss;
    }
    if (value == "uplink_corruption") {
        return ResilienceInjection::uplink_corruption;
    }
    if (value == "frame_expiry") {
        return ResilienceInjection::frame_expiry;
    }
    if (value == "unauthorized_target") {
        return ResilienceInjection::unauthorized_target;
    }
    if (value == "disallowed_content_type") {
        return ResilienceInjection::disallowed_content_type;
    }
    if (value == "correlation_mismatch") {
        return ResilienceInjection::correlation_mismatch;
    }
    if (value == "duplicate_request") {
        return ResilienceInjection::duplicate_request;
    }
    if (value == "station_command_capacity") {
        return ResilienceInjection::station_command_capacity;
    }
    if (value == "gateway_command_replay") {
        return ResilienceInjection::gateway_command_replay;
    }
    if (value == "external_response_identity_mismatch") {
        return ResilienceInjection::external_response_identity_mismatch;
    }
    if (value == "external_attempt_capacity") {
        return ResilienceInjection::external_attempt_capacity;
    }
    if (value == "local_resource_exhaustion") {
        return ResilienceInjection::local_resource_exhaustion;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown M6 resilience injection");
}

[[nodiscard]] ResilienceDisposition decode_disposition(const std::string_view value) {
    if (value == "lost") {
        return ResilienceDisposition::lost;
    }
    if (value == "corrupted") {
        return ResilienceDisposition::corrupted;
    }
    if (value == "expired") {
        return ResilienceDisposition::expired;
    }
    if (value == "denied_target") {
        return ResilienceDisposition::denied_target;
    }
    if (value == "denied_content_type") {
        return ResilienceDisposition::denied_content_type;
    }
    if (value == "denied_correlation_mismatch") {
        return ResilienceDisposition::denied_correlation_mismatch;
    }
    if (value == "denied_duplicate_request") {
        return ResilienceDisposition::denied_duplicate_request;
    }
    if (value == "denied_capacity") {
        return ResilienceDisposition::denied_capacity;
    }
    if (value == "rejected_invariant") {
        return ResilienceDisposition::rejected_invariant;
    }
    if (value == "rejected_resource") {
        return ResilienceDisposition::rejected_resource;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown M6 resilience disposition");
}

template <typename Target> void decode_strings(const Json& values, Target& target) {
    for (const auto& value : values.array_range()) {
        target.push_back(value.as<std::string>());
    }
}

[[nodiscard]] ResilienceScenarioProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& scope = document.at("security_scope");
    ResilienceScenarioProfile result{document.at("schema_version").as<std::string>(),
                                     identity.at("id").as<std::string>(),
                                     identity.at("version").as<std::string>(),
                                     identity.at("title").as<std::string>(),
                                     document.at("baseline_profile_id").as<std::string>(),
                                     {},
                                     {scope.at("threat_model").as<std::string>(), {}, {}},
                                     {},
                                     {}};
    for (const auto& scenario : document.at("scenarios").array_range()) {
        result.scenarios.push_back(
            {scenario.at("id").as<std::string>(), scenario.at("category").as<std::string>(),
             decode_injection(scenario.at("injection").as<std::string_view>()),
             scenario.at("expected_boundary").as<std::string>(),
             decode_disposition(scenario.at("expected_disposition").as<std::string_view>()),
             scenario.at("protected_state_unchanged").as<bool>(),
             scenario.at("communication_metrics_accounted").as<bool>()});
    }
    decode_strings(scope.at("protected_properties"), result.security_scope.protected_properties);
    decode_strings(scope.at("excluded_claims"), result.security_scope.excluded_claims);
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    decode_strings(document.at("limitations"), result.limitations);
    return result;
}

[[nodiscard]] ResilienceDisposition expected_for(const ResilienceInjection injection) noexcept {
    switch (injection) {
    case ResilienceInjection::uplink_loss:
        return ResilienceDisposition::lost;
    case ResilienceInjection::uplink_corruption:
        return ResilienceDisposition::corrupted;
    case ResilienceInjection::frame_expiry:
        return ResilienceDisposition::expired;
    case ResilienceInjection::unauthorized_target:
        return ResilienceDisposition::denied_target;
    case ResilienceInjection::disallowed_content_type:
        return ResilienceDisposition::denied_content_type;
    case ResilienceInjection::correlation_mismatch:
        return ResilienceDisposition::denied_correlation_mismatch;
    case ResilienceInjection::duplicate_request:
        return ResilienceDisposition::denied_duplicate_request;
    case ResilienceInjection::station_command_capacity:
        return ResilienceDisposition::denied_capacity;
    case ResilienceInjection::gateway_command_replay:
    case ResilienceInjection::external_response_identity_mismatch:
    case ResilienceInjection::external_attempt_capacity:
        return ResilienceDisposition::rejected_invariant;
    case ResilienceInjection::local_resource_exhaustion:
        return ResilienceDisposition::rejected_resource;
    }
    return ResilienceDisposition::rejected_invariant;
}

[[nodiscard]] std::string_view expected_boundary_for(const ResilienceInjection injection) noexcept {
    switch (injection) {
    case ResilienceInjection::uplink_loss:
    case ResilienceInjection::uplink_corruption:
    case ResilienceInjection::frame_expiry:
        return "ban_transport";
    case ResilienceInjection::unauthorized_target:
    case ResilienceInjection::disallowed_content_type:
    case ResilienceInjection::correlation_mismatch:
    case ResilienceInjection::duplicate_request:
    case ResilienceInjection::station_command_capacity:
        return "station_governance";
    case ResilienceInjection::gateway_command_replay:
        return "gateway_adapter";
    case ResilienceInjection::external_response_identity_mismatch:
    case ResilienceInjection::external_attempt_capacity:
        return "external_simulator_adapter";
    case ResilienceInjection::local_resource_exhaustion:
        return "nanodevice";
    }
    return "unknown";
}

[[nodiscard]] bool metrics_expected_for(const ResilienceInjection injection) noexcept {
    return injection == ResilienceInjection::uplink_loss ||
           injection == ResilienceInjection::uplink_corruption ||
           injection == ResilienceInjection::frame_expiry;
}

} // namespace

std::string_view to_string(const ResilienceInjection injection) noexcept {
    switch (injection) {
    case ResilienceInjection::uplink_loss:
        return "uplink_loss";
    case ResilienceInjection::uplink_corruption:
        return "uplink_corruption";
    case ResilienceInjection::frame_expiry:
        return "frame_expiry";
    case ResilienceInjection::unauthorized_target:
        return "unauthorized_target";
    case ResilienceInjection::disallowed_content_type:
        return "disallowed_content_type";
    case ResilienceInjection::correlation_mismatch:
        return "correlation_mismatch";
    case ResilienceInjection::duplicate_request:
        return "duplicate_request";
    case ResilienceInjection::station_command_capacity:
        return "station_command_capacity";
    case ResilienceInjection::gateway_command_replay:
        return "gateway_command_replay";
    case ResilienceInjection::external_response_identity_mismatch:
        return "external_response_identity_mismatch";
    case ResilienceInjection::external_attempt_capacity:
        return "external_attempt_capacity";
    case ResilienceInjection::local_resource_exhaustion:
        return "local_resource_exhaustion";
    }
    return "unknown";
}

std::string_view to_string(const ResilienceDisposition disposition) noexcept {
    switch (disposition) {
    case ResilienceDisposition::lost:
        return "lost";
    case ResilienceDisposition::corrupted:
        return "corrupted";
    case ResilienceDisposition::expired:
        return "expired";
    case ResilienceDisposition::denied_target:
        return "denied_target";
    case ResilienceDisposition::denied_content_type:
        return "denied_content_type";
    case ResilienceDisposition::denied_correlation_mismatch:
        return "denied_correlation_mismatch";
    case ResilienceDisposition::denied_duplicate_request:
        return "denied_duplicate_request";
    case ResilienceDisposition::denied_capacity:
        return "denied_capacity";
    case ResilienceDisposition::rejected_invariant:
        return "rejected_invariant";
    case ResilienceDisposition::rejected_resource:
        return "rejected_resource";
    }
    return "unknown";
}

void validate_resilience_scenario_profile(const ResilienceScenarioProfile& profile) {
    if (profile.schema_version != resilience_scenario_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.baseline_profile_id.empty() || profile.scenarios.empty() ||
        profile.security_scope.threat_model.empty() ||
        profile.security_scope.protected_properties.empty() ||
        profile.security_scope.excluded_claims.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid, "M6 resilience scenario profile is incomplete");
    }

    std::unordered_set<std::string> scenario_ids;
    std::unordered_set<ResilienceInjection> injections;
    for (const auto& scenario : profile.scenarios) {
        if (scenario.scenario_id.empty() || scenario.category.empty() ||
            scenario.expected_boundary.empty() || !scenario.protected_state_unchanged ||
            scenario.expected_disposition != expected_for(scenario.injection) ||
            scenario.expected_boundary != expected_boundary_for(scenario.injection) ||
            scenario.communication_metrics_accounted != metrics_expected_for(scenario.injection) ||
            !scenario_ids.insert(scenario.scenario_id).second ||
            !injections.insert(scenario.injection).second) {
            invalid(core::ErrorCode::data_invalid,
                    "M6 resilience scenarios must be unique, state-safe, and consistent");
        }
    }

    constexpr std::array required_injections{
        ResilienceInjection::uplink_loss,
        ResilienceInjection::uplink_corruption,
        ResilienceInjection::frame_expiry,
        ResilienceInjection::unauthorized_target,
        ResilienceInjection::disallowed_content_type,
        ResilienceInjection::correlation_mismatch,
        ResilienceInjection::duplicate_request,
        ResilienceInjection::station_command_capacity,
        ResilienceInjection::gateway_command_replay,
        ResilienceInjection::external_response_identity_mismatch,
        ResilienceInjection::external_attempt_capacity,
        ResilienceInjection::local_resource_exhaustion};
    if (std::ranges::any_of(required_injections, [&injections](const auto injection) {
            return !injections.contains(injection);
        })) {
        invalid(core::ErrorCode::data_invalid,
                "M6 resilience profile does not cover every required injection");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "M6 resilience sources must be complete and unique");
        }
    }
    const auto has_empty = [](const auto& values) {
        return std::ranges::any_of(values, [](const auto& value) { return value.empty(); });
    };
    if (has_empty(profile.security_scope.protected_properties) ||
        has_empty(profile.security_scope.excluded_claims) || has_empty(profile.limitations)) {
        invalid(core::ErrorCode::data_invalid,
                "M6 resilience scope and limitations must not contain empty entries");
    }
}

ResilienceScenarioProfile
load_resilience_scenario_profile(const ResilienceScenarioProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "M6 resilience scenario schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "M6 resilience profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_resilience_scenario_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M6 resilience profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

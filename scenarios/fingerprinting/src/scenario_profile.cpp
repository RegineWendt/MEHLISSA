// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/scenario_profile.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <string>
#include <unordered_set>

namespace mehlissa::scenarios::fingerprinting {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw ScenarioProfileError{code, message};
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

[[nodiscard]] ArtifactRole artifact_role(const std::string_view id) {
    constexpr std::array roles{ArtifactRole::body_model,
                               ArtifactRole::body_state,
                               ArtifactRole::organ_model,
                               ArtifactRole::capillary_model,
                               ArtifactRole::capillary_cell_signal,
                               ArtifactRole::receptor_model,
                               ArtifactRole::locator_device,
                               ArtifactRole::collector_device,
                               ArtifactRole::communication_cluster,
                               ArtifactRole::gateway_endpoint,
                               ArtifactRole::active_gateway,
                               ArtifactRole::ban_station,
                               ArtifactRole::timer_baseline};
    const auto found =
        std::ranges::find(roles, id, [](const auto role) { return to_string(role); });
    if (found == roles.end()) {
        invalid(core::ErrorCode::data_invalid, "Unknown M7 artifact role: " + std::string{id});
    }
    return *found;
}

[[nodiscard]] StageKind stage_kind(const std::string_view id) {
    constexpr std::array stages{StageKind::injection,
                                StageKind::body_transport,
                                StageKind::organ_transfer,
                                StageKind::capillary_localization,
                                StageKind::molecular_recognition,
                                StageKind::fingerprint_assembly,
                                StageKind::local_collection,
                                StageKind::collector_return,
                                StageKind::gateway_measurement,
                                StageKind::external_report};
    const auto found =
        std::ranges::find(stages, id, [](const auto stage) { return to_string(stage); });
    if (found == stages.end()) {
        invalid(core::ErrorCode::data_invalid, "Unknown M7 stage kind: " + std::string{id});
    }
    return *found;
}

[[nodiscard]] ScenarioProfile decode(const Json& document) {
    const auto& scenario = document.at("scenario");
    const auto& run = document.at("run");
    const auto& target = document.at("target");
    const auto& acceptance = document.at("acceptance");

    std::vector<ArtifactReference> artifacts;
    artifacts.reserve(document.at("artifacts").size());
    for (const auto& artifact : document.at("artifacts").array_range()) {
        artifacts.push_back({artifact_role(artifact.at("role").as<std::string>()),
                             artifact.at("definition_path").as<std::string>(),
                             artifact.at("schema_path").as<std::string>()});
    }

    std::vector<StageKind> stages;
    stages.reserve(acceptance.at("required_stage_order").size());
    for (const auto& stage : acceptance.at("required_stage_order").array_range()) {
        stages.push_back(stage_kind(stage.as<std::string>()));
    }

    std::vector<ScenarioSource> sources;
    sources.reserve(document.at("sources").size());
    for (const auto& source : document.at("sources").array_range()) {
        sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }

    std::vector<std::string> limitations;
    limitations.reserve(document.at("limitations").size());
    for (const auto& limitation : document.at("limitations").array_range()) {
        limitations.push_back(limitation.as<std::string>());
    }

    return {
        document.at("schema_version").as<std::string>(),
        {scenario.at("id").as<std::string>(), scenario.at("version").as<std::string>(),
         scenario.at("title").as<std::string>(), scenario.at("acceptance_level").as<std::string>()},
        {run.at("id").as<std::string>(), run.at("master_seed").as<std::uint64_t>(),
         run.at("collector_count").as<std::uint64_t>()},
        {target.at("fingerprint_id").as<std::string>(), target.at("tissue").as<std::string>(),
         target.at("region_id").as<std::string>()},
        std::move(artifacts),
        {std::move(stages), acceptance.at("deterministic_replay_required").as<bool>(),
         acceptance.at("clinical_validation_claim").as<bool>()},
        std::move(sources),
        std::move(limitations),
    };
}

[[nodiscard]] bool safe_repository_relative_path(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
        return false;
    }
    return std::ranges::none_of(path, [](const auto& part) { return part == ".."; });
}

} // namespace

std::string_view to_string(const ArtifactRole role) noexcept {
    switch (role) {
    case ArtifactRole::body_model:
        return "body_model";
    case ArtifactRole::body_state:
        return "body_state";
    case ArtifactRole::organ_model:
        return "organ_model";
    case ArtifactRole::capillary_model:
        return "capillary_model";
    case ArtifactRole::capillary_cell_signal:
        return "capillary_cell_signal";
    case ArtifactRole::receptor_model:
        return "receptor_model";
    case ArtifactRole::locator_device:
        return "locator_device";
    case ArtifactRole::collector_device:
        return "collector_device";
    case ArtifactRole::communication_cluster:
        return "communication_cluster";
    case ArtifactRole::gateway_endpoint:
        return "gateway_endpoint";
    case ArtifactRole::active_gateway:
        return "active_gateway";
    case ArtifactRole::ban_station:
        return "ban_station";
    case ArtifactRole::timer_baseline:
        return "timer_baseline";
    }
    return "unknown";
}

std::string_view to_string(const StageKind stage) noexcept {
    switch (stage) {
    case StageKind::injection:
        return "injection";
    case StageKind::body_transport:
        return "body_transport";
    case StageKind::organ_transfer:
        return "organ_transfer";
    case StageKind::capillary_localization:
        return "capillary_localization";
    case StageKind::molecular_recognition:
        return "molecular_recognition";
    case StageKind::fingerprint_assembly:
        return "fingerprint_assembly";
    case StageKind::local_collection:
        return "local_collection";
    case StageKind::collector_return:
        return "collector_return";
    case StageKind::gateway_measurement:
        return "gateway_measurement";
    case StageKind::external_report:
        return "external_report";
    }
    return "unknown";
}

void validate_scenario_profile(const ScenarioProfile& profile) {
    if (profile.schema_version != scenario_profile_schema_version || profile.scenario.id.empty() ||
        profile.scenario.version.empty() || profile.scenario.title.empty() ||
        profile.scenario.acceptance_level != "A" || profile.run.id.empty() ||
        profile.run.collector_count == 0 || profile.target.fingerprint_id.empty() ||
        profile.target.tissue.empty() || profile.target.region_id.empty() ||
        !profile.acceptance.deterministic_replay_required ||
        profile.acceptance.clinical_validation_claim || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 fingerprinting scenario identity, target, or acceptance is invalid");
    }

    constexpr std::array required_roles{ArtifactRole::body_model,
                                        ArtifactRole::body_state,
                                        ArtifactRole::organ_model,
                                        ArtifactRole::capillary_model,
                                        ArtifactRole::capillary_cell_signal,
                                        ArtifactRole::receptor_model,
                                        ArtifactRole::locator_device,
                                        ArtifactRole::collector_device,
                                        ArtifactRole::communication_cluster,
                                        ArtifactRole::gateway_endpoint,
                                        ArtifactRole::active_gateway,
                                        ArtifactRole::ban_station,
                                        ArtifactRole::timer_baseline};
    std::unordered_set<ArtifactRole> roles;
    for (const auto& artifact : profile.artifacts) {
        if (!safe_repository_relative_path(artifact.definition_path) ||
            !safe_repository_relative_path(artifact.schema_path) ||
            artifact.definition_path.extension() != ".json" ||
            artifact.schema_path.extension() != ".json" ||
            artifact.definition_path == artifact.schema_path ||
            !roles.insert(artifact.role).second) {
            invalid(
                core::ErrorCode::data_invalid,
                "M7.1 artifacts must have unique roles and safe repository-relative JSON paths");
        }
    }
    if (profile.artifacts.size() != required_roles.size() ||
        std::ranges::any_of(required_roles,
                            [&roles](const auto role) { return !roles.contains(role); })) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 scenario must select exactly one artifact for every required M2-M6 role");
    }

    constexpr std::array required_stages{StageKind::injection,
                                         StageKind::body_transport,
                                         StageKind::organ_transfer,
                                         StageKind::capillary_localization,
                                         StageKind::molecular_recognition,
                                         StageKind::fingerprint_assembly,
                                         StageKind::local_collection,
                                         StageKind::collector_return,
                                         StageKind::gateway_measurement,
                                         StageKind::external_report};
    if (!std::ranges::equal(profile.acceptance.required_stage_order, required_stages)) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 scenario must preserve the canonical causal fingerprinting stage order");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "M7.1 sources must be complete and have unique identifiers");
        }
    }
    if (std::ranges::any_of(profile.limitations,
                            [](const auto& limitation) { return limitation.empty(); })) {
        invalid(core::ErrorCode::data_invalid, "M7.1 limitations must not contain empty entries");
    }
}

ScenarioProfile load_scenario_profile(const ScenarioProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "M7.1 scenario schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "M7.1 scenario profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_scenario_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 scenario profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::scenarios::fingerprinting

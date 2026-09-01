// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_a_composer.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <fstream>
#include <string>
#include <system_error>

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
                "Cannot open M7.1 " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid, "Invalid JSON in M7.1 " + std::string{role} + " '" +
                                                   path.string() + "': " + error.what());
    }
}

void validate_artifact(const ResolvedArtifact& artifact) {
    const auto schema_document = read_json(artifact.schema_path, "artifact schema");
    const auto definition_document = read_json(artifact.definition_path, "artifact definition");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        schema.validate(definition_document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 artifact '" + std::string{to_string(artifact.role)} +
                    "' does not satisfy its selected schema: " + error.what());
    }
}

[[nodiscard]] std::filesystem::path resolve(const std::filesystem::path& repository_root,
                                            const std::filesystem::path& relative_path) {
    std::error_code error;
    const auto canonical_root = std::filesystem::weakly_canonical(repository_root, error);
    if (error) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot resolve M7.1 repository root: " + repository_root.string());
    }
    const auto resolved = std::filesystem::weakly_canonical(canonical_root / relative_path, error);
    if (error) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot resolve M7.1 artifact path: " + relative_path.string());
    }
    const auto relative_to_root = resolved.lexically_relative(canonical_root);
    if (relative_to_root.empty() || relative_to_root.is_absolute() ||
        std::ranges::any_of(relative_to_root, [](const auto& part) { return part == ".."; })) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 artifact path escapes the repository root: " + resolved.string());
    }
    const auto is_regular = std::filesystem::is_regular_file(resolved, error);
    if (error || !is_regular) {
        invalid(core::ErrorCode::input_unreadable,
                "M7.1 artifact path is not a regular file: " + resolved.string());
    }
    return resolved;
}

} // namespace

LevelAPlan compose_level_a_plan(const ScenarioProfile& profile,
                                const std::filesystem::path& repository_root) {
    validate_scenario_profile(profile);
    std::error_code error;
    const auto is_directory = std::filesystem::is_directory(repository_root, error);
    if (error || !is_directory) {
        invalid(core::ErrorCode::input_unreadable,
                "M7.1 repository root is not a directory: " + repository_root.string());
    }

    std::vector<ResolvedArtifact> artifacts;
    artifacts.reserve(profile.artifacts.size());
    for (const auto& reference : profile.artifacts) {
        ResolvedArtifact resolved{reference.role,
                                  resolve(repository_root, reference.definition_path),
                                  resolve(repository_root, reference.schema_path)};
        validate_artifact(resolved);
        artifacts.push_back(std::move(resolved));
    }

    const auto timer =
        std::ranges::find(artifacts, ArtifactRole::timer_baseline, &ResolvedArtifact::role);
    if (timer == artifacts.end()) {
        invalid(core::ErrorCode::data_invalid, "M7.1 timer baseline artifact is missing");
    }
    auto baseline =
        experiment::load_fingerprint_timer_baseline(timer->definition_path, timer->schema_path);
    if (baseline.fingerprint_id != profile.target.fingerprint_id ||
        baseline.target_tissue != profile.target.tissue ||
        baseline.target_region_id != profile.target.region_id) {
        invalid(core::ErrorCode::data_invalid,
                "M7.1 target identity does not match the selected timer baseline");
    }
    auto run = experiment::run_fingerprint_timer_baseline(baseline, profile.run.collector_count);
    return {profile, std::move(artifacts), std::move(baseline), std::move(run)};
}

} // namespace mehlissa::scenarios::fingerprinting

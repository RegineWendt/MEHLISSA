// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/result_report.hpp>

#include <mehlissa/experiment/provenance.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <fstream>
#include <string>

namespace mehlissa::scenarios::fingerprinting {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw ScenarioProfileError{code, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot open M7.3 result schema: " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid,
                "Invalid M7.3 result schema '" + path.string() + "': " + error.what());
    }
}

[[nodiscard]] std::int64_t nanoseconds(const core::SimulationClock::Duration duration) noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

[[nodiscard]] Json encode(const FingerprintingResultReport& report) {
    Json artifacts = Json::array();
    for (const auto& artifact : report.artifacts) {
        artifacts.push_back(Json{jsoncons::json_object_arg,
                                 {{"role", to_string(artifact.role)},
                                  {"definition_path", artifact.definition_path.generic_string()},
                                  {"definition_sha256", artifact.definition_sha256},
                                  {"schema_path", artifact.schema_path.generic_string()},
                                  {"schema_sha256", artifact.schema_sha256}}});
    }

    Json components = Json::array();
    for (const auto& component : report.runtime.components) {
        components.push_back(Json{jsoncons::json_object_arg,
                                  {{"role", to_string(component.role)},
                                   {"profile_id", component.profile_id},
                                   {"model_id", component.model_id},
                                   {"instantiated", component.instantiated},
                                   {"advanced", component.advanced}}});
    }

    Json stages = Json::array();
    for (const auto& stage : report.runtime.stages) {
        stages.push_back(Json{jsoncons::json_object_arg,
                              {{"ordinal", stage.ordinal},
                               {"stage", to_string(stage.stage)},
                               {"time_ns", nanoseconds(stage.time)},
                               {"basis", to_string(stage.basis)},
                               {"component_id", stage.component_id},
                               {"input_identity", stage.input_identity},
                               {"output_identity", stage.output_identity},
                               {"qualification", stage.qualification}}});
    }

    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", report.schema_version},
            {"scenario", Json{jsoncons::json_object_arg,
                              {{"id", report.scenario.id},
                               {"version", report.scenario.version},
                               {"title", report.scenario.title},
                               {"acceptance_level", report.scenario.acceptance_level}}}},
            {"run", Json{jsoncons::json_object_arg,
                         {{"id", report.run.id},
                          {"master_seed", report.run.master_seed},
                          {"collector_count", report.run.collector_count}}}},
            {"target", Json{jsoncons::json_object_arg,
                            {{"fingerprint_id", report.target.fingerprint_id},
                             {"tissue", report.target.tissue},
                             {"region_id", report.target.region_id}}}},
            {"reproducibility",
             Json{jsoncons::json_object_arg,
                  {{"deterministic_replay_required", report.deterministic_replay_required},
                   {"artifacts", std::move(artifacts)}}}},
            {"runtime", Json{jsoncons::json_object_arg,
                             {{"contract_version", report.runtime.contract_version},
                              {"component_probe_duration_ns",
                               nanoseconds(report.runtime.component_probe_duration)},
                              {"components", std::move(components)},
                              {"stages", std::move(stages)}}}},
            {"validity", Json{jsoncons::json_object_arg,
                              {{"clinical_validation_claim", report.clinical_validation_claim},
                               {"limitations", report.limitations}}}},
        }};
}

} // namespace

FingerprintingResultReport make_fingerprinting_result_report(const LevelAPlan& plan,
                                                             const LevelARuntimeResult& runtime) {
    if (runtime.scenario_id != plan.profile.scenario.id || runtime.run_id != plan.profile.run.id ||
        runtime.master_seed != plan.profile.run.master_seed || runtime.stages.size() != 10) {
        invalid(core::ErrorCode::data_invalid,
                "M7.3 runtime result does not match the selected scenario manifest");
    }

    std::vector<ResultArtifactManifestEntry> artifacts;
    artifacts.reserve(plan.artifacts.size());
    for (std::size_t index = 0; index < plan.artifacts.size(); ++index) {
        const auto& resolved = plan.artifacts[index];
        const auto& reference = plan.profile.artifacts[index];
        if (resolved.role != reference.role) {
            invalid(core::ErrorCode::data_invalid,
                    "M7.3 resolved artifact order differs from the scenario manifest");
        }
        artifacts.push_back({resolved.role, reference.definition_path,
                             experiment::sha256_file(resolved.definition_path),
                             reference.schema_path, experiment::sha256_file(resolved.schema_path)});
    }

    return {std::string{fingerprinting_result_schema_version},
            plan.profile.scenario,
            plan.profile.run,
            plan.profile.target,
            std::move(artifacts),
            runtime,
            plan.profile.acceptance.deterministic_replay_required,
            plan.profile.acceptance.clinical_validation_claim,
            plan.profile.limitations};
}

void write_fingerprinting_result_report(const FingerprintingResultReport& report,
                                        const std::filesystem::path& output_path,
                                        const std::filesystem::path& schema_path) {
    auto document = encode(report);
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(read_json(schema_path));
        schema.validate(document);
    } catch (const ScenarioProfileError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M7.3 result does not satisfy the selected schema: " + std::string{error.what()});
    }

    std::ofstream stream{output_path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Cannot write M7.3 result report: " + output_path.string());
    }
    document.dump_pretty(stream);
    stream << '\n';
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Failed while writing M7.3 result report: " + output_path.string());
    }
}

} // namespace mehlissa::scenarios::fingerprinting

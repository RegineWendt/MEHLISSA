// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/result_report.hpp>

#include <mehlissa/experiment/provenance.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
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

[[nodiscard]] Json encode_interval(const std::optional<ProportionInterval>& interval) {
    if (!interval.has_value()) {
        return Json::null();
    }
    return Json{jsoncons::json_object_arg,
                {{"estimate", interval->estimate},
                 {"lower_95", interval->lower_95},
                 {"upper_95", interval->upper_95}}};
}

[[nodiscard]] Json encode_holistic(const HolisticFingerprintingResultReport& report) {
    auto document = encode(report.reproducibility);
    document["schema_version"] = holistic_fingerprinting_result_schema_version;

    const auto& detection = report.detection;
    document["level_b_detection"] =
        Json{jsoncons::json_object_arg,
             {{"receptor_profile_id", detection.receptor_profile_id},
              {"receptor_model_id", detection.receptor_model_id},
              {"detector_device_id", detection.detector_device_id},
              {"event_id", detection.event_id},
              {"signal_id", detection.signal_id},
              {"ligand_concentration_mol_m3",
               core::in_moles_per_cubic_meter(detection.ligand_concentration)},
              {"exposure_duration_ns", nanoseconds(detection.exposure_duration)},
              {"equilibrium_bound_fraction", detection.equilibrium_bound_fraction},
              {"final_bound_fraction", detection.final_bound_fraction},
              {"detected", detection.detected},
              {"relative_threshold_crossing_ns",
               detection.relative_threshold_crossing.has_value()
                   ? Json{nanoseconds(detection.relative_threshold_crossing.value())}
                   : Json::null()},
              {"absolute_detection_time_ns",
               detection.absolute_detection_time.has_value()
                   ? Json{nanoseconds(detection.absolute_detection_time.value())}
                   : Json::null()}}};

    Json releases = Json::array();
    for (const auto& release : report.assembly.releases) {
        releases.push_back(Json{jsoncons::json_object_arg,
                                {{"release_id", release.release_id},
                                 {"payload_id", release.payload_id},
                                 {"tile_id", release.tile_id},
                                 {"source_detection_event_id", release.source_detection_event_id},
                                 {"released_at_ns", nanoseconds(release.released_at)}}});
    }
    const auto& assembly = report.assembly;
    document["level_c_assembly"] =
        Json{jsoncons::json_object_arg,
             {{"assembly_id", assembly.assembly_id},
              {"fingerprint_id", assembly.fingerprint_id},
              {"releases", std::move(releases)},
              {"required_unique_tiles", assembly.required_unique_tiles},
              {"complete", assembly.complete},
              {"started_at_ns", nanoseconds(assembly.started_at)},
              {"completed_at_ns", assembly.completed_at.has_value()
                                      ? Json{nanoseconds(assembly.completed_at.value())}
                                      : Json::null()},
              {"qualification", assembly.qualification}}};

    const auto& communication = report.communication;
    document["level_d_communication"] = Json{
        jsoncons::json_object_arg,
        {{"local_collection_message_id", communication.local_collection_message_id},
         {"collector_uplink_message_id", communication.collector_uplink_message_id},
         {"gateway_measurement_id", communication.gateway_measurement_id},
         {"ban_frame_id", communication.ban_frame_id},
         {"external_report_id", communication.external_report_id},
         {"local_collection_route", communication.local_collection_route},
         {"collector_uplink_route", communication.collector_uplink_route},
         {"local_collection_completed_at_ns",
          nanoseconds(communication.local_collection_completed_at)},
         {"collector_uplink_completed_at_ns",
          nanoseconds(communication.collector_uplink_completed_at)},
         {"gateway_measurement_at_ns", nanoseconds(communication.gateway_measurement_at)},
         {"external_report_at_ns", nanoseconds(communication.external_report_at)},
         {"attempted_local_messages", communication.attempted_local_messages},
         {"delivered_local_messages", communication.delivered_local_messages},
         {"attempted_ban_frames", communication.attempted_ban_frames},
         {"delivered_ban_frames", communication.delivered_ban_frames},
         {"local_transmitter_energy_j", core::in_joules(communication.local_transmitter_energy)},
         {"local_receiver_energy_j", core::in_joules(communication.local_receiver_energy)},
         {"local_link_energy_j", core::in_joules(communication.local_link_energy)},
         {"ban_transmitter_energy_j", core::in_joules(communication.ban_transmitter_energy)},
         {"ban_receiver_energy_j", core::in_joules(communication.ban_receiver_energy)},
         {"ban_link_energy_j", core::in_joules(communication.ban_link_energy)},
         {"qualification", communication.qualification}}};

    Json cases = Json::array();
    for (const auto& item : report.analysis.cases) {
        cases.push_back(Json{jsoncons::json_object_arg,
                             {{"case_id", item.case_id},
                              {"ligand_concentration_mol_m3",
                               core::in_moles_per_cubic_meter(item.ligand_concentration)},
                              {"exposure_duration_ns", nanoseconds(item.exposure_duration)},
                              {"target_present", item.target_present},
                              {"detected", item.detected},
                              {"final_bound_fraction", item.final_bound_fraction},
                              {"classification", to_string(item.classification)}}});
    }
    const auto& summary = report.analysis.summary;
    document["level_e_analysis"] = Json{
        jsoncons::json_object_arg,
        {{"analysis_id", report.analysis.analysis_id},
         {"cases", std::move(cases)},
         {"summary", Json{jsoncons::json_object_arg,
                          {{"true_positive", summary.true_positive},
                           {"true_negative", summary.true_negative},
                           {"false_positive", summary.false_positive},
                           {"false_negative", summary.false_negative},
                           {"sensitivity", encode_interval(summary.sensitivity)},
                           {"specificity", encode_interval(summary.specificity)},
                           {"false_positive_rate", encode_interval(summary.false_positive_rate)},
                           {"false_negative_rate", encode_interval(summary.false_negative_rate)}}}},
         {"varied_parameters", report.analysis.varied_parameters},
         {"limitations", report.analysis.limitations}}};
    return document;
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
                                        const ResultReportPaths& paths) {
    auto document = encode(report);
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(read_json(paths.schema));
        schema.validate(document);
    } catch (const ScenarioProfileError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M7.3 result does not satisfy the selected schema: " + std::string{error.what()});
    }

    std::ofstream stream{paths.output, std::ios::binary | std::ios::trunc};
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Cannot write M7.3 result report: " + paths.output.string());
    }
    document.dump_pretty(stream);
    stream << '\n';
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Failed while writing M7.3 result report: " + paths.output.string());
    }
}

HolisticFingerprintingResultReport
run_holistic_fingerprinting_scenario(const LevelAPlan& plan,
                                     const std::vector<LevelECase>& analysis_cases) {
    auto runtime = run_level_a_runtime(plan);
    const auto detection = run_level_b_detection(plan, default_level_b_detection_input(plan));
    apply_level_b_detection(runtime, detection);
    const auto assembly =
        run_level_c_assembly(plan, detection, default_level_c_assembly_input(plan));
    apply_level_c_assembly(runtime, assembly);
    const auto communication = run_level_d_communication(plan, assembly);
    apply_level_d_communication(runtime, communication);
    const auto analysis = run_level_e_analysis(plan, analysis_cases);
    auto reproducibility = make_fingerprinting_result_report(plan, runtime);
    reproducibility.schema_version = holistic_fingerprinting_result_schema_version;
    reproducibility.limitations.insert(reproducibility.limitations.end(),
                                       analysis.limitations.begin(), analysis.limitations.end());
    return {std::move(reproducibility), detection, assembly, communication, analysis};
}

void write_holistic_fingerprinting_result_report(const HolisticFingerprintingResultReport& report,
                                                 const ResultReportPaths& paths) {
    auto document = encode_holistic(report);
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(read_json(paths.schema));
        schema.validate(document);
    } catch (const ScenarioProfileError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "M7 holistic result does not satisfy the selected schema: " +
                    std::string{error.what()});
    }

    std::ofstream stream{paths.output, std::ios::binary | std::ios::trunc};
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Cannot write M7 holistic result report: " + paths.output.string());
    }
    document.dump_pretty(stream);
    stream << '\n';
    if (!stream) {
        invalid(core::ErrorCode::output_unwritable,
                "Failed while writing M7 holistic result report: " + paths.output.string());
    }
}

} // namespace mehlissa::scenarios::fingerprinting

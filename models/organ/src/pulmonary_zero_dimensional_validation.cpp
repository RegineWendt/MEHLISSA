// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_zero_dimensional_validation.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::organ {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

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

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::schema_invalid,
                "Invalid pulmonary-validation schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary validation does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] PulmonaryValidationAcceptanceRole decode_role(const std::string_view role) {
    if (role == "input") {
        return PulmonaryValidationAcceptanceRole::input;
    }
    if (role == "required") {
        return PulmonaryValidationAcceptanceRole::required;
    }
    return PulmonaryValidationAcceptanceRole::diagnostic;
}

[[nodiscard]] PulmonaryValidationObservation decode_observation(const Json& document) {
    return {
        document.at("mean_si").as<double>(),
        document.at("standard_deviation_si").as<double>(),
        document.at("unit").as<std::string>(),
        decode_role(document.at("acceptance_role").as<std::string_view>()),
    };
}

[[nodiscard]] PulmonaryValidationScope decode_scope(const std::string_view scope) {
    if (scope == "declared_scope") {
        return PulmonaryValidationScope::declared_scope;
    }
    if (scope == "near_scope_crosscheck") {
        return PulmonaryValidationScope::near_scope_crosscheck;
    }
    return PulmonaryValidationScope::stress_test;
}

[[nodiscard]] PulmonaryValidationBoundaryPolicy
decode_boundary_policy(const std::string_view policy) {
    if (policy == "locked_left_atrial_pressure") {
        return PulmonaryValidationBoundaryPolicy::locked_left_atrial_pressure;
    }
    return PulmonaryValidationBoundaryPolicy::measured_wedge_pressure;
}

[[nodiscard]] PulmonaryZeroDimensionalValidationCase decode(const Json& document) {
    const auto& identity = document.at("validation");
    const auto& protocol = document.at("protocol");
    PulmonaryZeroDimensionalValidationCase result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("title").as<std::string>(),
        identity.at("model_definition_id").as<std::string>(),
        protocol.at("maximum_absolute_z_score").as<double>(),
        {},
        {},
        {},
    };

    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back({
            source.at("id").as<std::string>(),
            source.at("citation").as<std::string>(),
            source.at("url").as<std::string>(),
            source.at("license").as<std::string>(),
            source.at("measurement_method").as<std::string>(),
            {},
            {},
        });
    }
    for (const auto& condition : document.at("conditions").array_range()) {
        const auto& observations = condition.at("observations");
        result.conditions.push_back({
            condition.at("id").as<std::string>(),
            condition.at("source_id").as<std::string>(),
            condition.at("description").as<std::string>(),
            decode_scope(condition.at("scope").as<std::string_view>()),
            decode_boundary_policy(condition.at("boundary_policy").as<std::string_view>()),
            decode_observation(observations.at("cardiac_output")),
            observations.contains("pulmonary_arterial_wedge_pressure")
                ? std::optional<PulmonaryValidationObservation>{decode_observation(
                      observations.at("pulmonary_arterial_wedge_pressure"))}
                : std::nullopt,
            decode_observation(observations.at("mean_pulmonary_arterial_pressure")),
            observations.contains("pulmonary_arterial_compliance")
                ? std::optional<PulmonaryValidationObservation>{decode_observation(
                      observations.at("pulmonary_arterial_compliance"))}
                : std::nullopt,
            observations.contains("rc_time_constant")
                ? std::optional<PulmonaryValidationObservation>{decode_observation(
                      observations.at("rc_time_constant"))}
                : std::nullopt,
        });
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

void validate_semantics(const PulmonaryZeroDimensionalValidationCase& validation) {
    if (validation.schema_version != pulmonary_zero_dimensional_validation_schema_version ||
        !std::isfinite(validation.maximum_absolute_z_score) ||
        validation.maximum_absolute_z_score <= 0.0) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary validation has an unsupported version or invalid threshold");
    }
    std::unordered_set<std::string> source_ids;
    for (const auto& source : validation.sources) {
        if (!source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary-validation source IDs must be unique");
        }
    }
    std::unordered_set<std::string> condition_ids;
    for (const auto& condition : validation.conditions) {
        if (!condition_ids.insert(condition.id).second ||
            !source_ids.contains(condition.source_id)) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary-validation conditions require unique IDs and declared sources");
        }
        if (condition.cardiac_output.role != PulmonaryValidationAcceptanceRole::input ||
            condition.mean_pulmonary_arterial_pressure.role ==
                PulmonaryValidationAcceptanceRole::input) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary-validation inputs and endpoints have inconsistent roles");
        }
        if (condition.boundary_policy ==
                PulmonaryValidationBoundaryPolicy::measured_wedge_pressure &&
            !condition.pulmonary_arterial_wedge_pressure.has_value()) {
            invalid(core::ErrorCode::data_invalid,
                    "Measured-wedge validation requires a PAWP observation");
        }
    }
}

[[nodiscard]] PulmonaryValidationEndpointResult
evaluate_endpoint(const std::string& endpoint, const PulmonaryValidationObservation& observation,
                  const double predicted_si, const double threshold) {
    const auto z_score =
        std::abs(predicted_si - observation.mean_si) / observation.standard_deviation_si;
    return {endpoint,     observation.role, observation.mean_si, observation.standard_deviation_si,
            predicted_si, z_score,          z_score <= threshold};
}

[[nodiscard]] PulmonaryMultipointEvidenceStatus
decode_evidence_status(const std::string_view status) {
    if (status == "measured_validation") {
        return PulmonaryMultipointEvidenceStatus::measured_validation;
    }
    return PulmonaryMultipointEvidenceStatus::synthetic_test_only;
}

[[nodiscard]] PulmonaryBodyPosition decode_body_position(const std::string_view position) {
    if (position == "supine") {
        return PulmonaryBodyPosition::supine;
    }
    if (position == "semiupright") {
        return PulmonaryBodyPosition::semiupright;
    }
    return PulmonaryBodyPosition::upright;
}

[[nodiscard]] std::optional<double> optional_number(const Json& document,
                                                    const std::string_view key) {
    if (!document.contains(key)) {
        return std::nullopt;
    }
    return document.at(key).as<double>();
}

[[nodiscard]] PulmonaryZeroDimensionalMultipointValidationCase
decode_multipoint(const Json& document) {
    const auto& identity = document.at("validation");
    const auto& protocol = document.at("protocol");
    PulmonaryZeroDimensionalMultipointValidationCase result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("title").as<std::string>(),
        identity.at("model_definition_id").as<std::string>(),
        decode_evidence_status(identity.at("evidence_status").as<std::string_view>()),
        protocol.at("minimum_stage_count").as<std::size_t>(),
        {},
        {},
        {},
    };

    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back({
            source.at("id").as<std::string>(),
            source.at("citation").as<std::string>(),
            source.at("url").as<std::string>(),
            source.at("license").as<std::string>(),
            source.at("measurement_method").as<std::string>(),
            source.at("data_access").as<std::string>(),
            source.at("cohort_independence").as<std::string>(),
        });
    }
    for (const auto& subject : document.at("subjects").array_range()) {
        PulmonaryMultipointSubject decoded_subject{
            subject.at("id").as<std::string>(),
            subject.at("source_id").as<std::string>(),
            subject.at("phenotype").as<std::string>(),
            decode_body_position(subject.at("body_position").as<std::string_view>()),
            subject.at("cardiac_output_method").as<std::string>(),
            {},
        };
        for (const auto& stage : subject.at("stages").array_range()) {
            decoded_subject.stages.push_back({
                stage.at("id").as<std::string>(),
                stage.at("ordinal").as<std::size_t>(),
                stage.at("workload_watts").as<double>(),
                stage.at("cardiac_output_m3_per_s").as<double>(),
                stage.at("pulmonary_arterial_wedge_pressure_pa").as<double>(),
                stage.at("mean_pulmonary_arterial_pressure_pa").as<double>(),
                optional_number(stage, "systolic_pulmonary_arterial_pressure_pa"),
                optional_number(stage, "diastolic_pulmonary_arterial_pressure_pa"),
                optional_number(stage, "heart_rate_per_minute"),
            });
        }
        result.subjects.push_back(std::move(decoded_subject));
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

[[nodiscard]] bool positive_finite(const double value) {
    return std::isfinite(value) && value > 0.0;
}

inline constexpr double millimetres_of_mercury_to_pascals = 133.322387415;
inline constexpr double litres_per_minute_to_cubic_metres_per_second = 1.0 / 60000.0;

[[nodiscard]] PulmonaryPopulationStatisticKind
decode_population_statistic_kind(const std::string_view kind) {
    if (kind == "mean_standard_deviation") {
        return PulmonaryPopulationStatisticKind::mean_standard_deviation;
    }
    return PulmonaryPopulationStatisticKind::mean_confidence_interval_95;
}

[[nodiscard]] PulmonaryPopulationFlowBasis
decode_population_flow_basis(const std::string_view basis) {
    if (basis == "absolute_cardiac_output") {
        return PulmonaryPopulationFlowBasis::absolute_cardiac_output;
    }
    return PulmonaryPopulationFlowBasis::cardiac_index;
}

[[nodiscard]] PulmonaryPopulationChallengeKind
decode_population_challenge_kind(const std::string_view kind) {
    if (kind == "rest") {
        return PulmonaryPopulationChallengeKind::rest;
    }
    if (kind == "passive_leg_raise") {
        return PulmonaryPopulationChallengeKind::passive_leg_raise;
    }
    return PulmonaryPopulationChallengeKind::exercise;
}

[[nodiscard]] PulmonaryPopulationObservation decode_population_observation(const Json& document,
                                                                           const double si_scale) {
    const auto scale_optional = [&document, si_scale](const std::string_view key) {
        const auto value = optional_number(document, key);
        return value.has_value() ? std::optional<double>{value.value() * si_scale} : std::nullopt;
    };
    return {
        document.at("mean").as<double>() * si_scale,
        scale_optional("standard_deviation"),
        scale_optional("confidence_interval_95_lower"),
        scale_optional("confidence_interval_95_upper"),
        document.at("unit").as<std::string>(),
    };
}

[[nodiscard]] PulmonaryZeroDimensionalPopulationMultipointValidationCase
decode_population_multipoint(const Json& document) {
    const auto& identity = document.at("validation");
    const auto& protocol = document.at("protocol");
    PulmonaryZeroDimensionalPopulationMultipointValidationCase result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("title").as<std::string>(),
        identity.at("model_definition_id").as<std::string>(),
        protocol.at("minimum_stage_count").as<std::size_t>(),
        protocol.at("standard_deviation_multiplier").as<double>(),
        {},
        {},
        {},
    };

    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back({
            source.at("id").as<std::string>(),
            source.at("citation").as<std::string>(),
            source.at("url").as<std::string>(),
            source.at("license").as<std::string>(),
            source.at("measurement_method").as<std::string>(),
            source.at("data_access").as<std::string>(),
            source.at("cohort_independence").as<std::string>(),
        });
    }
    for (const auto& series : document.at("series").array_range()) {
        const auto flow_basis =
            decode_population_flow_basis(series.at("flow_basis").as<std::string_view>());
        const auto flow_scale = litres_per_minute_to_cubic_metres_per_second;
        PulmonaryPopulationSeries decoded_series{
            series.at("id").as<std::string>(),
            series.at("source_id").as<std::string>(),
            series.at("phenotype").as<std::string>(),
            series.at("sample_size").as<std::size_t>(),
            decode_body_position(series.at("body_position").as<std::string_view>()),
            series.at("cardiac_output_method").as<std::string>(),
            decode_population_statistic_kind(series.at("statistic_kind").as<std::string_view>()),
            flow_basis,
            optional_number(series, "reference_body_surface_area_m2"),
            series.at("cohort_overlap").as<std::string>(),
            {},
        };
        for (const auto& stage : series.at("stages").array_range()) {
            decoded_series.stages.push_back({
                stage.at("id").as<std::string>(),
                stage.at("ordinal").as<std::size_t>(),
                decode_population_challenge_kind(stage.at("challenge_kind").as<std::string_view>()),
                optional_number(stage, "workload_watts_mean"),
                decode_population_observation(stage.at("cardiac_flow"), flow_scale),
                decode_population_observation(stage.at("pulmonary_arterial_wedge_pressure"),
                                              millimetres_of_mercury_to_pascals),
                decode_population_observation(stage.at("mean_pulmonary_arterial_pressure"),
                                              millimetres_of_mercury_to_pascals),
                stage.contains("heart_rate")
                    ? std::optional<PulmonaryPopulationObservation>{decode_population_observation(
                          stage.at("heart_rate"), 1.0)}
                    : std::nullopt,
            });
        }
        result.series.push_back(std::move(decoded_series));
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

void validate_multipoint_semantics(
    const PulmonaryZeroDimensionalMultipointValidationCase& validation,
    const bool allow_synthetic_test_data) {
    if (validation.schema_version !=
            pulmonary_zero_dimensional_multipoint_validation_schema_version ||
        validation.minimum_stage_count < 3) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary multipoint validation has an unsupported version or stage count");
    }
    if (validation.evidence_status == PulmonaryMultipointEvidenceStatus::synthetic_test_only &&
        !allow_synthetic_test_data) {
        invalid(core::ErrorCode::data_invalid,
                "Synthetic pulmonary multipoint data are disabled for evidence evaluation");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : validation.sources) {
        if (!source_ids.insert(source.id).second ||
            source.cohort_independence != "confirmed_disjoint") {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary multipoint sources must be unique and cohort-disjoint");
        }
    }

    std::unordered_set<std::string> subject_ids;
    for (const auto& subject : validation.subjects) {
        if (!subject_ids.insert(subject.id).second || !source_ids.contains(subject.source_id) ||
            subject.phenotype != "healthy_control" ||
            subject.stages.size() < validation.minimum_stage_count) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary multipoint subjects are not independently identifiable, healthy, "
                    "source-linked, and sufficiently sampled");
        }

        std::unordered_set<std::string> stage_ids;
        std::unordered_set<std::size_t> ordinals;
        std::vector<double> flows;
        flows.reserve(subject.stages.size());
        for (std::size_t index = 0; index < subject.stages.size(); ++index) {
            const auto& stage = subject.stages[index];
            const auto pulsatile_values_present =
                static_cast<unsigned>(stage.systolic_pulmonary_arterial_pressure_si.has_value()) +
                static_cast<unsigned>(stage.diastolic_pulmonary_arterial_pressure_si.has_value()) +
                static_cast<unsigned>(stage.heart_rate_per_minute.has_value());
            if (!stage_ids.insert(stage.id).second || !ordinals.insert(stage.ordinal).second ||
                stage.ordinal != index ||
                (index > 0 && stage.workload_watts < subject.stages[index - 1].workload_watts) ||
                !std::isfinite(stage.workload_watts) || stage.workload_watts < 0.0 ||
                !positive_finite(stage.cardiac_output_si) ||
                !positive_finite(stage.pulmonary_arterial_wedge_pressure_si) ||
                !positive_finite(stage.mean_pulmonary_arterial_pressure_si) ||
                stage.mean_pulmonary_arterial_pressure_si <=
                    stage.pulmonary_arterial_wedge_pressure_si ||
                (pulsatile_values_present != 0U && pulsatile_values_present != 3U)) {
                invalid(core::ErrorCode::data_invalid,
                        "Pulmonary multipoint stages contain invalid, duplicate, or incomplete "
                        "measurements");
            }
            if (pulsatile_values_present == 3U &&
                (!positive_finite(stage.systolic_pulmonary_arterial_pressure_si.value()) ||
                 !positive_finite(stage.diastolic_pulmonary_arterial_pressure_si.value()) ||
                 !positive_finite(stage.heart_rate_per_minute.value()) ||
                 stage.systolic_pulmonary_arterial_pressure_si.value() <=
                     stage.mean_pulmonary_arterial_pressure_si ||
                 stage.mean_pulmonary_arterial_pressure_si <=
                     stage.diastolic_pulmonary_arterial_pressure_si.value())) {
                invalid(core::ErrorCode::data_invalid,
                        "Pulmonary multipoint pulsatile measurements are inconsistent");
            }
            flows.push_back(stage.cardiac_output_si);
        }
        std::ranges::sort(flows);
        if (std::adjacent_find(flows.begin(), flows.end()) != flows.end()) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary multipoint regression requires distinct cardiac outputs");
        }
    }
}

void validate_population_observation(const PulmonaryPopulationObservation& observation,
                                     const PulmonaryPopulationStatisticKind statistic_kind) {
    const auto has_standard_deviation = observation.standard_deviation_si.has_value();
    const auto has_lower = observation.confidence_interval_95_lower_si.has_value();
    const auto has_upper = observation.confidence_interval_95_upper_si.has_value();
    if (!positive_finite(observation.mean_si)) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary population observations require a positive finite mean");
    }
    if (statistic_kind == PulmonaryPopulationStatisticKind::mean_standard_deviation) {
        if (!has_standard_deviation || has_lower || has_upper ||
            !positive_finite(observation.standard_deviation_si.value())) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary population mean/SD series require exactly one positive SD");
        }
        return;
    }
    if (has_standard_deviation || !has_lower || !has_upper ||
        !positive_finite(observation.confidence_interval_95_lower_si.value()) ||
        !positive_finite(observation.confidence_interval_95_upper_si.value()) ||
        observation.confidence_interval_95_lower_si.value() > observation.mean_si ||
        observation.mean_si > observation.confidence_interval_95_upper_si.value() ||
        observation.confidence_interval_95_lower_si.value() >=
            observation.confidence_interval_95_upper_si.value()) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary population mean/95% CI series require ordered confidence bounds; "
                "a rounded bound may equal the rounded mean");
    }
}

[[nodiscard]] double population_absolute_flow(const PulmonaryPopulationSeries& series,
                                              const PulmonaryPopulationStage& stage) {
    if (series.flow_basis == PulmonaryPopulationFlowBasis::absolute_cardiac_output) {
        return stage.cardiac_flow.mean_si;
    }
    return stage.cardiac_flow.mean_si * series.reference_body_surface_area_m2.value();
}

void validate_population_multipoint_semantics(
    const PulmonaryZeroDimensionalPopulationMultipointValidationCase& validation) {
    if (validation.schema_version !=
            pulmonary_zero_dimensional_population_multipoint_validation_schema_version ||
        validation.minimum_stage_count < 3 ||
        !positive_finite(validation.standard_deviation_multiplier)) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary population multipoint validation has an unsupported protocol");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : validation.sources) {
        if (!source_ids.insert(source.id).second ||
            source.cohort_independence != "confirmed_disjoint") {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary population sources must be unique and cohort-disjoint");
        }
    }

    std::unordered_set<std::string> series_ids;
    for (const auto& series : validation.series) {
        if (!series_ids.insert(series.id).second || !source_ids.contains(series.source_id) ||
            series.phenotype != "healthy_control" || series.sample_size == 0 ||
            series.cohort_overlap != "disjoint_from_other_series" ||
            series.stages.size() < validation.minimum_stage_count) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary population series require unique IDs, disjoint healthy cohorts, "
                    "declared sources, and sufficient stages");
        }
        if ((series.flow_basis == PulmonaryPopulationFlowBasis::cardiac_index) !=
            series.reference_body_surface_area_m2.has_value()) {
            invalid(
                core::ErrorCode::data_invalid,
                "Cardiac-index series require one reference BSA; absolute-flow series forbid it");
        }
        if (series.reference_body_surface_area_m2.has_value() &&
            !positive_finite(series.reference_body_surface_area_m2.value())) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary population reference BSA must be positive and finite");
        }

        std::unordered_set<std::string> stage_ids;
        std::vector<double> flows;
        std::optional<double> previous_exercise_workload;
        for (std::size_t index = 0; index < series.stages.size(); ++index) {
            const auto& stage = series.stages[index];
            if (!stage_ids.insert(stage.id).second || stage.ordinal != index ||
                (stage.workload_watts_mean.has_value() &&
                 (!std::isfinite(stage.workload_watts_mean.value()) ||
                  stage.workload_watts_mean.value() < 0.0)) ||
                stage.mean_pulmonary_arterial_pressure.mean_si <=
                    stage.pulmonary_arterial_wedge_pressure.mean_si) {
                invalid(core::ErrorCode::data_invalid,
                        "Pulmonary population stages contain invalid identifiers, order, "
                        "workload, or pressures");
            }
            if (stage.challenge_kind == PulmonaryPopulationChallengeKind::exercise) {
                if (stage.workload_watts_mean.has_value() &&
                    previous_exercise_workload.has_value() &&
                    stage.workload_watts_mean.value() < previous_exercise_workload.value()) {
                    invalid(core::ErrorCode::data_invalid,
                            "Pulmonary population exercise workload must be nondecreasing");
                }
                if (stage.workload_watts_mean.has_value()) {
                    previous_exercise_workload = stage.workload_watts_mean;
                }
            }
            validate_population_observation(stage.cardiac_flow, series.statistic_kind);
            validate_population_observation(stage.pulmonary_arterial_wedge_pressure,
                                            series.statistic_kind);
            validate_population_observation(stage.mean_pulmonary_arterial_pressure,
                                            series.statistic_kind);
            if (stage.heart_rate.has_value()) {
                validate_population_observation(stage.heart_rate.value(), series.statistic_kind);
            }
            const auto expected_flow_unit =
                series.flow_basis == PulmonaryPopulationFlowBasis::absolute_cardiac_output
                    ? "L/min"
                    : "L/min/m2";
            if (stage.cardiac_flow.reported_unit != expected_flow_unit ||
                stage.pulmonary_arterial_wedge_pressure.reported_unit != "mmHg" ||
                stage.mean_pulmonary_arterial_pressure.reported_unit != "mmHg" ||
                (stage.heart_rate.has_value() && stage.heart_rate->reported_unit != "1/min")) {
                invalid(core::ErrorCode::data_invalid,
                        "Pulmonary population observation units do not match the series contract");
            }
            flows.push_back(population_absolute_flow(series, stage));
        }
        std::ranges::sort(flows);
        if (std::adjacent_find(flows.begin(), flows.end()) != flows.end()) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary population regression requires distinct mean cardiac outputs");
        }
    }
}

[[nodiscard]] PulmonaryLinearFit linear_fit(const std::vector<double>& x,
                                            const std::vector<double>& y) {
    const auto count = static_cast<double>(x.size());
    const auto mean_x = std::accumulate(x.begin(), x.end(), 0.0) / count;
    const auto mean_y = std::accumulate(y.begin(), y.end(), 0.0) / count;
    double covariance{};
    double x_variance{};
    double y_variance{};
    for (std::size_t index = 0; index < x.size(); ++index) {
        const auto centered_x = x[index] - mean_x;
        const auto centered_y = y[index] - mean_y;
        covariance += centered_x * centered_y;
        x_variance += centered_x * centered_x;
        y_variance += centered_y * centered_y;
    }
    if (!positive_finite(x_variance)) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary multipoint regression requires flow variation");
    }
    const auto slope = covariance / x_variance;
    const auto intercept = mean_y - slope * mean_x;
    const auto coefficient =
        y_variance > 0.0 ? (covariance * covariance) / (x_variance * y_variance) : 1.0;
    return {slope, intercept, std::clamp(coefficient, 0.0, 1.0)};
}

} // namespace

PulmonaryZeroDimensionalValidationCase
load_pulmonary_zero_dimensional_validation_case(const PulmonaryValidationCaseLoadRequest& request) {
    const auto document = read_json(request.validation_path, "pulmonary-validation data");
    const auto schema_document = read_json(request.schema_path, "pulmonary-validation schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    validate_document(document, schema, request.validation_path);
    auto result = decode(document);
    validate_semantics(result);
    return result;
}

PulmonaryZeroDimensionalValidationReport evaluate_pulmonary_zero_dimensional_validation(
    const PulmonaryZeroDimensionalValidationCase& validation,
    const LungModelDefinition& model_definition) {
    if (validation.model_definition_id != model_definition.definition_id ||
        model_definition.model.variant != LungModelVariant::pulmonary_zero_dimensional ||
        !model_definition.model.zero_dimensional_parameters.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary validation and model definition are incompatible");
    }

    std::unordered_set<std::string> calibration_urls;
    for (const auto& source : model_definition.sources) {
        calibration_urls.insert(source.url);
    }
    for (const auto& source : validation.sources) {
        if (calibration_urls.contains(source.url)) {
            invalid(core::ErrorCode::data_invalid,
                    "Independent pulmonary validation reuses a model evidence source");
        }
    }

    PulmonaryZeroDimensionalValidationReport report{
        validation.validation_id, validation.model_definition_id, true, true, 0, 0, 0, {},
    };
    const auto locked_parameters = model_definition.model.zero_dimensional_parameters.value();

    for (const auto& condition : validation.conditions) {
        auto parameters = locked_parameters;
        parameters.baseline_cardiac_output =
            core::cubic_meters_per_second(condition.cardiac_output.mean_si);
        if (condition.boundary_policy ==
            PulmonaryValidationBoundaryPolicy::measured_wedge_pressure) {
            if (!condition.pulmonary_arterial_wedge_pressure.has_value()) {
                invalid(core::ErrorCode::data_invalid,
                        "Measured-wedge validation lost its PAWP observation");
            }
            parameters.left_atrial_pressure =
                core::pascals(condition.pulmonary_arterial_wedge_pressure.value().mean_si);
        }

        PulmonaryZeroDimensionalModel model{PulmonaryZeroDimensionalConfig{
            "organ.lung.validation",
            "lung.pulmonary-0d.validation",
            "pulmonary-arterial-entry",
            "pulmonary-venous-exit",
            "body.validation",
            "pulmonary-venous-return",
            parameters,
        }};
        const auto state = model.state();
        PulmonaryValidationConditionResult condition_result{condition.id, condition.scope, {}};
        condition_result.endpoints.push_back(evaluate_endpoint(
            "mean_pulmonary_arterial_pressure", condition.mean_pulmonary_arterial_pressure,
            core::in_pascals(state.mean_pulmonary_arterial_pressure),
            validation.maximum_absolute_z_score));
        if (condition.pulmonary_arterial_compliance.has_value()) {
            condition_result.endpoints.push_back(evaluate_endpoint(
                "pulmonary_arterial_compliance", condition.pulmonary_arterial_compliance.value(),
                core::in_cubic_meters_per_pascal(state.effective_pulmonary_arterial_compliance),
                validation.maximum_absolute_z_score));
        }
        if (condition.rc_time_constant.has_value()) {
            condition_result.endpoints.push_back(
                evaluate_endpoint("rc_time_constant", condition.rc_time_constant.value(),
                                  core::in_seconds(state.pressure_time_constant),
                                  validation.maximum_absolute_z_score));
        }

        for (const auto& endpoint : condition_result.endpoints) {
            if (endpoint.role == PulmonaryValidationAcceptanceRole::required) {
                ++report.required_endpoint_count;
                if (endpoint.accepted) {
                    ++report.accepted_required_endpoint_count;
                } else {
                    report.required_endpoints_pass = false;
                }
            } else if (endpoint.role == PulmonaryValidationAcceptanceRole::diagnostic &&
                       !endpoint.accepted) {
                ++report.failed_diagnostic_endpoint_count;
            }
        }
        report.conditions.push_back(std::move(condition_result));
    }
    if (report.required_endpoint_count == 0) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary validation must declare at least one required endpoint");
    }
    return report;
}

PulmonaryZeroDimensionalMultipointValidationCase
load_pulmonary_zero_dimensional_multipoint_validation_case(
    const PulmonaryMultipointValidationCaseLoadRequest& request) {
    const auto document =
        read_json(request.validation_path, "pulmonary multipoint validation data");
    const auto schema_document =
        read_json(request.schema_path, "pulmonary multipoint validation schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    validate_document(document, schema, request.validation_path);
    auto result = decode_multipoint(document);
    validate_multipoint_semantics(result, request.allow_synthetic_test_data);
    return result;
}

PulmonaryZeroDimensionalMultipointValidationReport
evaluate_pulmonary_zero_dimensional_multipoint_validation(
    const PulmonaryZeroDimensionalMultipointValidationCase& validation,
    const LungModelDefinition& model_definition) {
    if (validation.model_definition_id != model_definition.definition_id ||
        model_definition.model.variant != LungModelVariant::pulmonary_zero_dimensional ||
        !model_definition.model.zero_dimensional_parameters.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary multipoint validation and model definition are incompatible");
    }

    std::unordered_set<std::string> calibration_urls;
    for (const auto& source : model_definition.sources) {
        calibration_urls.insert(source.url);
    }
    for (const auto& source : validation.sources) {
        if (calibration_urls.contains(source.url)) {
            invalid(core::ErrorCode::data_invalid,
                    "Independent pulmonary multipoint validation reuses a model evidence source");
        }
    }

    PulmonaryZeroDimensionalMultipointValidationReport report{
        validation.validation_id,
        validation.model_definition_id,
        true,
        validation.evidence_status == PulmonaryMultipointEvidenceStatus::measured_validation,
        validation.subjects.size(),
        0,
        {},
    };
    const auto locked_parameters = model_definition.model.zero_dimensional_parameters.value();

    for (const auto& subject : validation.subjects) {
        PulmonaryMultipointSubjectResult subject_result{subject.id, {}, {}, {}, 0.0, 0.0, {}};
        std::vector<double> flows;
        std::vector<double> observed_pressures;
        std::vector<double> predicted_pressures;
        std::vector<double> wedge_pressures;
        flows.reserve(subject.stages.size());
        observed_pressures.reserve(subject.stages.size());
        predicted_pressures.reserve(subject.stages.size());
        wedge_pressures.reserve(subject.stages.size());

        double pressure_error_sum{};
        double squared_pressure_error_sum{};
        for (const auto& stage : subject.stages) {
            auto parameters = locked_parameters;
            parameters.baseline_cardiac_output =
                core::cubic_meters_per_second(stage.cardiac_output_si);
            parameters.left_atrial_pressure =
                core::pascals(stage.pulmonary_arterial_wedge_pressure_si);
            PulmonaryZeroDimensionalModel model{PulmonaryZeroDimensionalConfig{
                "organ.lung.multipoint-validation",
                "lung.pulmonary-0d.multipoint-validation",
                "pulmonary-arterial-entry",
                "pulmonary-venous-exit",
                "body.validation",
                "pulmonary-venous-return",
                parameters,
            }};
            const auto state = model.state();
            const auto predicted_pressure =
                core::in_pascals(state.mean_pulmonary_arterial_pressure);
            const auto residual = predicted_pressure - stage.mean_pulmonary_arterial_pressure_si;
            const auto observed_resistance = (stage.mean_pulmonary_arterial_pressure_si -
                                              stage.pulmonary_arterial_wedge_pressure_si) /
                                             stage.cardiac_output_si;
            const auto predicted_resistance = core::in_pascal_seconds_per_cubic_meter(
                state.effective_pulmonary_vascular_resistance);
            const auto predicted_compliance =
                core::in_cubic_meters_per_pascal(state.effective_pulmonary_arterial_compliance);
            std::optional<double> observed_compliance;
            std::optional<double> observed_rc;
            if (stage.systolic_pulmonary_arterial_pressure_si.has_value()) {
                const auto stroke_volume =
                    stage.cardiac_output_si * 60.0 / stage.heart_rate_per_minute.value();
                const auto pulse_pressure = stage.systolic_pulmonary_arterial_pressure_si.value() -
                                            stage.diastolic_pulmonary_arterial_pressure_si.value();
                observed_compliance = stroke_volume / pulse_pressure;
                observed_rc = observed_resistance * observed_compliance.value();
            }

            subject_result.stages.push_back({
                stage.id,
                stage.workload_watts,
                stage.cardiac_output_si,
                stage.mean_pulmonary_arterial_pressure_si,
                predicted_pressure,
                residual,
                observed_resistance,
                predicted_resistance,
                observed_compliance,
                predicted_compliance,
                observed_rc,
                core::in_seconds(state.pressure_time_constant),
            });
            flows.push_back(stage.cardiac_output_si);
            observed_pressures.push_back(stage.mean_pulmonary_arterial_pressure_si);
            predicted_pressures.push_back(predicted_pressure);
            wedge_pressures.push_back(stage.pulmonary_arterial_wedge_pressure_si);
            pressure_error_sum += residual;
            squared_pressure_error_sum += residual * residual;
        }

        const auto stage_count = static_cast<double>(subject.stages.size());
        subject_result.observed_mpap_flow_fit = linear_fit(flows, observed_pressures);
        subject_result.predicted_mpap_flow_fit = linear_fit(flows, predicted_pressures);
        subject_result.observed_pawp_flow_fit = linear_fit(flows, wedge_pressures);
        subject_result.mean_pressure_error_si = pressure_error_sum / stage_count;
        subject_result.root_mean_square_pressure_error_si =
            std::sqrt(squared_pressure_error_sum / stage_count);
        report.stage_count += subject.stages.size();
        report.subjects.push_back(std::move(subject_result));
    }
    return report;
}

PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_pulmonary_zero_dimensional_population_multipoint_validation_case(
    const PulmonaryPopulationMultipointValidationCaseLoadRequest& request) {
    const auto document =
        read_json(request.validation_path, "pulmonary population multipoint validation data");
    const auto schema_document =
        read_json(request.schema_path, "pulmonary population multipoint validation schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    validate_document(document, schema, request.validation_path);
    auto result = decode_population_multipoint(document);
    validate_population_multipoint_semantics(result);
    return result;
}

PulmonaryZeroDimensionalPopulationMultipointValidationReport
evaluate_pulmonary_zero_dimensional_population_multipoint_validation(
    const PulmonaryZeroDimensionalPopulationMultipointValidationCase& validation,
    const LungModelDefinition& model_definition) {
    if (validation.model_definition_id != model_definition.definition_id ||
        model_definition.model.variant != LungModelVariant::pulmonary_zero_dimensional ||
        !model_definition.model.zero_dimensional_parameters.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "Pulmonary population validation and model definition are incompatible");
    }

    std::unordered_set<std::string> calibration_urls;
    for (const auto& source : model_definition.sources) {
        calibration_urls.insert(source.url);
    }
    for (const auto& source : validation.sources) {
        if (calibration_urls.contains(source.url)) {
            invalid(core::ErrorCode::data_invalid,
                    "Independent pulmonary population validation reuses a model evidence source");
        }
    }

    PulmonaryZeroDimensionalPopulationMultipointValidationReport report{
        validation.validation_id,
        validation.model_definition_id,
        true,
        true,
        true,
        validation.series.size(),
        0,
        0,
        {},
    };
    const auto locked_parameters = model_definition.model.zero_dimensional_parameters.value();

    for (const auto& series : validation.series) {
        PulmonaryPopulationSeriesResult series_result{series.id, series.sample_size, {}, {}, 0.0, 0,
                                                      {}};
        std::vector<double> flows;
        std::vector<double> observed_pressures;
        std::vector<double> predicted_pressures;
        double squared_pressure_error_sum{};
        flows.reserve(series.stages.size());
        observed_pressures.reserve(series.stages.size());
        predicted_pressures.reserve(series.stages.size());

        for (const auto& stage : series.stages) {
            const auto flow = population_absolute_flow(series, stage);
            auto parameters = locked_parameters;
            parameters.baseline_cardiac_output = core::cubic_meters_per_second(flow);
            parameters.left_atrial_pressure =
                core::pascals(stage.pulmonary_arterial_wedge_pressure.mean_si);
            PulmonaryZeroDimensionalModel model{PulmonaryZeroDimensionalConfig{
                "organ.lung.population-multipoint-validation",
                "lung.pulmonary-0d.population-multipoint-validation",
                "pulmonary-arterial-entry",
                "pulmonary-venous-exit",
                "body.validation",
                "pulmonary-venous-return",
                parameters,
            }};
            const auto predicted_pressure =
                core::in_pascals(model.state().mean_pulmonary_arterial_pressure);
            const auto observed_pressure = stage.mean_pulmonary_arterial_pressure.mean_si;
            const auto residual = predicted_pressure - observed_pressure;
            double lower{};
            double upper{};
            std::optional<double> z_score;
            if (series.statistic_kind ==
                PulmonaryPopulationStatisticKind::mean_standard_deviation) {
                const auto standard_deviation =
                    stage.mean_pulmonary_arterial_pressure.standard_deviation_si.value();
                lower = observed_pressure -
                        validation.standard_deviation_multiplier * standard_deviation;
                upper = observed_pressure +
                        validation.standard_deviation_multiplier * standard_deviation;
                z_score = std::abs(residual) / standard_deviation;
            } else {
                lower =
                    stage.mean_pulmonary_arterial_pressure.confidence_interval_95_lower_si.value();
                upper =
                    stage.mean_pulmonary_arterial_pressure.confidence_interval_95_upper_si.value();
            }
            const auto accepted = predicted_pressure >= lower && predicted_pressure <= upper;
            if (accepted) {
                ++series_result.accepted_stage_count;
                ++report.accepted_stage_count;
            } else {
                report.all_stages_agree = false;
            }
            series_result.stages.push_back({
                stage.id,
                stage.workload_watts_mean,
                flow,
                observed_pressure,
                predicted_pressure,
                residual,
                lower,
                upper,
                z_score,
                accepted,
            });
            flows.push_back(flow);
            observed_pressures.push_back(observed_pressure);
            predicted_pressures.push_back(predicted_pressure);
            squared_pressure_error_sum += residual * residual;
        }
        series_result.observed_mpap_flow_fit = linear_fit(flows, observed_pressures);
        series_result.predicted_mpap_flow_fit = linear_fit(flows, predicted_pressures);
        series_result.root_mean_square_pressure_error_si =
            std::sqrt(squared_pressure_error_sum / static_cast<double>(series.stages.size()));
        report.stage_count += series.stages.size();
        report.series.push_back(std::move(series_result));
    }
    return report;
}

} // namespace mehlissa::models::organ

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_zero_dimensional_validation.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cmath>
#include <fstream>
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
                core::in_cubic_meters_per_pascal(parameters.pulmonary_arterial_compliance),
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

} // namespace mehlissa::models::organ

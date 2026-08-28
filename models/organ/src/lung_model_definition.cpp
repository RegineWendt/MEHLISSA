// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/lung_model_definition.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
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
                "Invalid lung-model schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Lung-model definition does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration duration_from_seconds(const double seconds) {
    constexpr double nanoseconds_per_second = 1'000'000'000.0;
    const auto maximum =
        static_cast<double>(std::numeric_limits<std::int64_t>::max()) / nanoseconds_per_second;
    if (!std::isfinite(seconds) || seconds <= 0.0 || seconds > maximum) {
        invalid(core::ErrorCode::data_invalid,
                "Lung-model transit duration must be positive, finite, and representable");
    }
    const auto nanoseconds =
        static_cast<std::int64_t>(std::llround(seconds * nanoseconds_per_second));
    if (nanoseconds <= 0) {
        invalid(core::ErrorCode::data_invalid,
                "Lung-model transit duration is below simulation-clock resolution");
    }
    return core::SimulationClock::Duration{nanoseconds};
}

[[nodiscard]] LungModelConfig decode_model(const Json& document) {
    const auto variant_name = document.at("variant").as<std::string>();
    const auto& component = document.at("component");
    const auto& timing = document.at("timing");

    LungModelConfig config{};
    if (variant_name == "effective_compartment") {
        config.variant = LungModelVariant::effective_compartment;
    } else if (variant_name == "regional_circulation") {
        config.variant = LungModelVariant::regional_circulation;
    } else {
        config.variant = LungModelVariant::pulmonary_zero_dimensional;
    }
    config.component_name = component.at("name").as<std::string>();
    config.model_id = component.at("model_id").as<std::string>();
    config.entry_port_id = component.at("entry_port_id").as<std::string>();
    config.exit_port_id = component.at("exit_port_id").as<std::string>();
    config.return_target_model_id = component.at("return_target_model_id").as<std::string>();
    config.return_target_port_id = component.at("return_target_port_id").as<std::string>();

    if (config.variant == LungModelVariant::effective_compartment) {
        config.compartment_transit_time =
            duration_from_seconds(timing.at("transit_time_s").as<double>());
    } else if (config.variant == LungModelVariant::regional_circulation) {
        for (const auto& region : timing.at("regions").array_range()) {
            config.regions.push_back(
                {region.at("id").as<std::string>(),
                 duration_from_seconds(region.at("transit_time_s").as<double>())});
        }
    } else {
        const auto& hemodynamics = document.at("hemodynamics");
        const auto value = [&hemodynamics](const std::string_view name) {
            return hemodynamics.at(name).at("value_si").as<double>();
        };
        std::optional<PulmonaryFlowAdaptationParameters> flow_adaptation;
        if (hemodynamics.contains("flow_adaptation")) {
            const auto& adaptation = hemodynamics.at("flow_adaptation");
            flow_adaptation = PulmonaryFlowAdaptationParameters{
                core::cubic_meters_per_second(
                    adaptation.at("reference_cardiac_output").at("value_si").as<double>()),
                adaptation.at("resistance_exponent").at("value_si").as<double>(),
                adaptation.at("compliance_exponent").at("value_si").as<double>(),
                core::Dimensionless::from_si(
                    adaptation.at("maximum_flow_ratio").at("value_si").as<double>()),
            };
        }
        config.zero_dimensional_parameters = PulmonaryZeroDimensionalParameters{
            core::cubic_meters_per_second(value("baseline_cardiac_output")),
            core::pascals(value("left_atrial_pressure")),
            core::pascal_seconds_per_cubic_meter(value("pulmonary_vascular_resistance")),
            core::cubic_meters_per_pascal(value("pulmonary_arterial_compliance")),
            duration_from_seconds(timing.at("pulmonary_transit_time_s").as<double>()),
            core::Dimensionless::from_si(value("right_lung_perfusion_fraction")),
            flow_adaptation,
        };
    }
    return config;
}

[[nodiscard]] LungModelEvidenceQuantity decode_evidence_quantity(const Json& value) {
    const auto& uncertainty = value.at("uncertainty");
    return {
        value.at("value_si").as<double>(),
        value.at("unit").as<std::string>(),
        {uncertainty.at("kind").as<std::string>(),
         uncertainty.contains("lower_si")
             ? std::optional<double>{uncertainty.at("lower_si").as<double>()}
             : std::nullopt,
         uncertainty.contains("upper_si")
             ? std::optional<double>{uncertainty.at("upper_si").as<double>()}
             : std::nullopt},
        value.at("source_id").as<std::string>(),
        value.at("role").as<std::string>(),
        value.at("derivation").as<std::string>(),
    };
}

[[nodiscard]] PulmonaryHemodynamicEvidence decode_hemodynamics(const Json& document) {
    const auto& hemodynamics = document.at("hemodynamics");
    PulmonaryHemodynamicEvidence result{
        decode_evidence_quantity(hemodynamics.at("baseline_cardiac_output")),
        decode_evidence_quantity(hemodynamics.at("left_atrial_pressure")),
        decode_evidence_quantity(hemodynamics.at("pulmonary_vascular_resistance")),
        decode_evidence_quantity(hemodynamics.at("pulmonary_arterial_compliance")),
        decode_evidence_quantity(hemodynamics.at("pulmonary_transit_time")),
        decode_evidence_quantity(hemodynamics.at("right_lung_perfusion_fraction")),
        decode_evidence_quantity(hemodynamics.at("mean_pulmonary_arterial_pressure_target")),
        std::nullopt,
    };
    if (hemodynamics.contains("flow_adaptation")) {
        const auto& adaptation = hemodynamics.at("flow_adaptation");
        result.flow_adaptation = PulmonaryFlowAdaptationEvidence{
            decode_evidence_quantity(adaptation.at("reference_cardiac_output")),
            decode_evidence_quantity(adaptation.at("resistance_exponent")),
            decode_evidence_quantity(adaptation.at("compliance_exponent")),
            decode_evidence_quantity(adaptation.at("maximum_flow_ratio")),
        };
    }
    return result;
}

[[nodiscard]] LungModelDefinition decode(const Json& document) {
    const auto& identity = document.at("definition");
    const auto& validity = document.at("validity");
    LungModelDefinition result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        decode_model(document),
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
        std::nullopt,
        std::nullopt,
    };

    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("url").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    if (document.contains("external_data")) {
        const auto& external = document.at("external_data");
        ExternalPulmonaryDataReference reference{
            external.at("source_id").as<std::string>(),
            external.at("path").as<std::string>(),
            external.at("sha256").as<std::string>(),
            external.at("format").as<std::string>(),
            external.at("coordinate_system").as<std::string>(),
            external.at("length_unit").as<std::string>(),
            external.at("flow_unit").as<std::string>(),
            {},
        };
        for (const auto& transformation : external.at("transformations").array_range()) {
            reference.transformations.push_back(transformation.as<std::string>());
        }
        result.external_data = std::move(reference);
    }
    if (document.contains("hemodynamics")) {
        result.hemodynamics = decode_hemodynamics(document);
    }
    return result;
}

[[nodiscard]] bool supported_schema_version(const std::string_view version) noexcept {
    return version == earliest_supported_lung_model_definition_schema_version ||
           version == "1.1.0" || version == latest_supported_lung_model_definition_schema_version;
}

void validate_evidence_source(const LungModelEvidenceQuantity& parameter,
                              const std::unordered_set<std::string>& source_ids) {
    if (!source_ids.contains(parameter.source_id)) {
        invalid(core::ErrorCode::data_invalid,
                "Hemodynamic evidence must reference a declared source ID");
    }
    const auto lower = parameter.uncertainty.lower_si;
    const auto upper = parameter.uncertainty.upper_si;
    if (!std::isfinite(parameter.value_si) ||
        (lower.has_value() && (!std::isfinite(*lower) || parameter.value_si < *lower)) ||
        (upper.has_value() && (!std::isfinite(*upper) || parameter.value_si > *upper)) ||
        (lower.has_value() && upper.has_value() && *lower > *upper)) {
        invalid(core::ErrorCode::data_invalid,
                "Hemodynamic evidence value and uncertainty bounds are inconsistent");
    }
}

[[nodiscard]] bool nearly_equal(const double left, const double right) noexcept {
    const auto scale = std::fmax(1.0, std::fmax(std::abs(left), std::abs(right)));
    return std::abs(left - right) <= scale * 1.0e-10;
}

void validate_definition(const LungModelDefinition& definition) {
    if (!supported_schema_version(definition.schema_version) || definition.definition_id.empty() ||
        definition.definition_version.empty() || definition.title.empty() ||
        definition.validity.population.empty() || definition.validity.physiological_state.empty() ||
        definition.validity.evidence_class.empty() || definition.validity.description.empty() ||
        definition.sources.empty() || definition.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Lung-model definition metadata is incomplete or invalid");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : definition.sources) {
        if (!source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Lung-model definition source IDs must be unique");
        }
    }
    if (definition.external_data.has_value() &&
        !source_ids.contains(definition.external_data->source_id)) {
        invalid(core::ErrorCode::data_invalid,
                "External pulmonary data must reference a declared source ID");
    }
    if (definition.model.variant == LungModelVariant::pulmonary_zero_dimensional) {
        if (!definition.hemodynamics.has_value() ||
            !definition.model.zero_dimensional_parameters.has_value()) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary 0D definition requires hemodynamic evidence");
        }
        const auto& evidence = *definition.hemodynamics;
        validate_evidence_source(evidence.baseline_cardiac_output, source_ids);
        validate_evidence_source(evidence.left_atrial_pressure, source_ids);
        validate_evidence_source(evidence.pulmonary_vascular_resistance, source_ids);
        validate_evidence_source(evidence.pulmonary_arterial_compliance, source_ids);
        validate_evidence_source(evidence.pulmonary_transit_time, source_ids);
        validate_evidence_source(evidence.right_lung_perfusion_fraction, source_ids);
        validate_evidence_source(evidence.mean_pulmonary_arterial_pressure_target, source_ids);
        if (evidence.flow_adaptation.has_value()) {
            validate_evidence_source(evidence.flow_adaptation->reference_cardiac_output,
                                     source_ids);
            validate_evidence_source(evidence.flow_adaptation->resistance_exponent, source_ids);
            validate_evidence_source(evidence.flow_adaptation->compliance_exponent, source_ids);
            validate_evidence_source(evidence.flow_adaptation->maximum_flow_ratio, source_ids);
        }

        const auto& parameters = *definition.model.zero_dimensional_parameters;
        const auto transit_seconds =
            static_cast<double>(parameters.pulmonary_transit_time.count()) / 1'000'000'000.0;
        const auto predicted_pressure = core::in_pascals(parameters.left_atrial_pressure +
                                                         parameters.pulmonary_vascular_resistance *
                                                             parameters.baseline_cardiac_output);
        if (!nearly_equal(evidence.pulmonary_transit_time.value_si, transit_seconds) ||
            !nearly_equal(evidence.mean_pulmonary_arterial_pressure_target.value_si,
                          predicted_pressure)) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary 0D timing or mean-pressure evidence is disconnected from the "
                    "executable parameterization");
        }
        if (evidence.flow_adaptation.has_value()) {
            if (!parameters.flow_adaptation.has_value() ||
                !nearly_equal(evidence.flow_adaptation->reference_cardiac_output.value_si,
                              core::in_cubic_meters_per_second(
                                  parameters.flow_adaptation->reference_cardiac_output)) ||
                !nearly_equal(evidence.flow_adaptation->resistance_exponent.value_si,
                              parameters.flow_adaptation->resistance_exponent) ||
                !nearly_equal(evidence.flow_adaptation->compliance_exponent.value_si,
                              parameters.flow_adaptation->compliance_exponent) ||
                !nearly_equal(evidence.flow_adaptation->maximum_flow_ratio.value_si,
                              parameters.flow_adaptation->maximum_flow_ratio.si_value())) {
                invalid(core::ErrorCode::data_invalid,
                        "Pulmonary flow-adaptation evidence is disconnected from the "
                        "executable parameterization");
            }
        } else if (parameters.flow_adaptation.has_value()) {
            invalid(core::ErrorCode::data_invalid,
                    "Pulmonary flow adaptation requires evidence metadata");
        }
    } else if (definition.hemodynamics.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "Only a pulmonary 0D definition may carry hemodynamic evidence");
    }

    static_cast<void>(make_lung_model(definition.model));
}

} // namespace

LungModelDefinition load_lung_model_definition(const LungModelDefinitionLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "lung-model schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto definition_document = read_json(request.definition_path, "lung-model definition");
    validate_document(definition_document, schema, request.definition_path);
    auto definition = decode(definition_document);
    validate_definition(definition);
    return definition;
}

} // namespace mehlissa::models::organ

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
    config.variant = variant_name == "effective_compartment"
                         ? LungModelVariant::effective_compartment
                         : LungModelVariant::regional_circulation;
    config.component_name = component.at("name").as<std::string>();
    config.model_id = component.at("model_id").as<std::string>();
    config.entry_port_id = component.at("entry_port_id").as<std::string>();
    config.exit_port_id = component.at("exit_port_id").as<std::string>();
    config.return_target_model_id = component.at("return_target_model_id").as<std::string>();
    config.return_target_port_id = component.at("return_target_port_id").as<std::string>();

    if (config.variant == LungModelVariant::effective_compartment) {
        config.compartment_transit_time =
            duration_from_seconds(timing.at("transit_time_s").as<double>());
    } else {
        for (const auto& region : timing.at("regions").array_range()) {
            config.regions.push_back(
                {region.at("id").as<std::string>(),
                 duration_from_seconds(region.at("transit_time_s").as<double>())});
        }
    }
    return config;
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
    return result;
}

void validate_definition(const LungModelDefinition& definition) {
    if (definition.schema_version != supported_lung_model_definition_schema_version ||
        definition.definition_id.empty() || definition.definition_version.empty() ||
        definition.title.empty() || definition.validity.population.empty() ||
        definition.validity.physiological_state.empty() ||
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

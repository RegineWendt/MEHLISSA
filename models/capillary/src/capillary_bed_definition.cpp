// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_bed_definition.hpp>

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

namespace mehlissa::models::capillary {
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
                "Invalid capillary-bed schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-bed definition does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration duration_from_seconds(const double seconds) {
    constexpr double nanoseconds_per_second = 1'000'000'000.0;
    const auto maximum =
        static_cast<double>(std::numeric_limits<std::int64_t>::max()) / nanoseconds_per_second;
    if (!std::isfinite(seconds) || seconds <= 0.0 || seconds > maximum) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-region duration must be positive, finite, and representable");
    }
    const auto nanoseconds =
        static_cast<std::int64_t>(std::llround(seconds * nanoseconds_per_second));
    if (nanoseconds <= 0) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-region duration is below simulation-clock resolution");
    }
    return core::SimulationClock::Duration{nanoseconds};
}

[[nodiscard]] CapillaryRegionKind decode_kind(const std::string_view kind) {
    if (kind == "arteriole") {
        return CapillaryRegionKind::arteriole;
    }
    if (kind == "capillary") {
        return CapillaryRegionKind::capillary;
    }
    if (kind == "venule") {
        return CapillaryRegionKind::venule;
    }
    invalid(core::ErrorCode::data_invalid, "Unsupported capillary-region kind");
}

[[nodiscard]] CapillaryBedDefinition decode(const Json& document) {
    const auto& identity = document.at("definition");
    const auto& component = document.at("component");
    const auto& network = document.at("network");
    const auto& validity = document.at("validity");

    CapillaryBedConfig config{
        component.at("name").as<std::string>(),
        component.at("model_id").as<std::string>(),
        component.at("entry_port_id").as<std::string>(),
        component.at("exit_port_id").as<std::string>(),
        component.at("return_target_model_id").as<std::string>(),
        component.at("return_target_port_id").as<std::string>(),
        network.at("total_parallel_path_count").as<std::uint64_t>(),
        network.at("perfused_path_count").as<std::uint64_t>(),
        {},
    };
    for (const auto& region : network.at("regions").array_range()) {
        config.regions.push_back({
            region.at("id").as<std::string>(),
            decode_kind(region.at("kind").as<std::string_view>()),
            duration_from_seconds(region.at("transit_time_s").as<double>()),
        });
    }

    CapillaryBedDefinition result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        std::move(config),
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

void validate_definition(const CapillaryBedDefinition& definition) {
    if (definition.schema_version != capillary_bed_definition_schema_version) {
        invalid(core::ErrorCode::data_invalid,
                "Unsupported capillary-bed definition schema version");
    }
    std::unordered_set<std::string> source_ids;
    for (const auto& source : definition.sources) {
        if (!source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-bed source identifiers must be unique");
        }
    }
    static_cast<void>(CapillaryBed{definition.model});
}

} // namespace

CapillaryBedDefinition
load_capillary_bed_definition(const CapillaryBedDefinitionLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "capillary-bed schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto definition_document = read_json(request.definition_path, "capillary-bed definition");
    validate_document(definition_document, schema, request.definition_path);
    auto definition = decode(definition_document);
    validate_definition(definition);
    return definition;
}

} // namespace mehlissa::models::capillary

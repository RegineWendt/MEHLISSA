// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/experiment_manifest.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace mehlissa::experiment {
namespace {

using Json = jsoncons::json;

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw ManifestError{"Cannot open " + std::string{role} + ": " + path.string()};
    }

    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw ManifestError{"Invalid JSON in " + std::string{role} + " '" + path.string() +
                            "': " + error.what()};
    }
}

void validate_against_schema(const Json& document, const Json& schema,
                             const std::filesystem::path& manifest_path) {
    try {
        const auto compiled_schema = jsoncons::jsonschema::make_json_schema(schema);
        compiled_schema.validate(document);
    } catch (const std::exception& error) {
        throw ManifestError{"Experiment manifest '" + manifest_path.string() +
                            "' does not satisfy its schema: " + error.what()};
    }
}

[[nodiscard]] std::uint64_t nanoseconds_per_unit(const std::string_view unit) {
    constexpr std::array units{
        std::pair{"ns", 1ULL},
        std::pair{"us", 1'000ULL},
        std::pair{"ms", 1'000'000ULL},
        std::pair{"s", 1'000'000'000ULL},
        std::pair{"min", 60'000'000'000ULL},
        std::pair{"h", 3'600'000'000'000ULL},
    };

    for (const auto& [name, multiplier] : units) {
        if (unit == name) {
            return multiplier;
        }
    }
    throw ManifestError{"Unsupported duration unit after schema validation: " + std::string{unit}};
}

[[nodiscard]] core::SimulationClock::Duration parse_duration(const Json& document) {
    const auto& duration = document.at("duration");
    const auto value = duration.at("value").as<std::uint64_t>();
    const auto unit = duration.at("unit").as<std::string>();
    const auto multiplier = nanoseconds_per_unit(unit);

    using Representation = core::SimulationClock::Duration::rep;
    constexpr auto maximum = static_cast<std::uint64_t>(std::numeric_limits<Representation>::max());
    if (value > maximum / multiplier) {
        throw ManifestError{"Experiment duration exceeds the supported nanosecond range"};
    }

    return core::SimulationClock::Duration{static_cast<Representation>(value * multiplier)};
}

[[nodiscard]] std::vector<std::string> parse_models(const Json& document) {
    std::vector<std::string> models;
    const auto& configured_models = document.at("models");
    models.reserve(configured_models.size());
    for (const auto& model : configured_models.array_range()) {
        models.push_back(model.as<std::string>());
    }
    return models;
}

} // namespace

ExperimentManifest load_experiment_manifest(const std::filesystem::path& manifest_path,
                                            const std::filesystem::path& schema_path) {
    const auto schema = read_json(schema_path, "experiment schema");
    const auto document = read_json(manifest_path, "experiment manifest");
    validate_against_schema(document, schema, manifest_path);

    try {
        ExperimentManifest manifest;
        manifest.schema_version = document.at("schema_version").as<std::string>();
        manifest.experiment_id = document.at("experiment_id").as<std::string>();
        manifest.duration = parse_duration(document);
        manifest.master_seed = document.at("random").at("master_seed").as<std::uint64_t>();
        manifest.models = parse_models(document);
        manifest.output_directory =
            std::filesystem::path{document.at("outputs").at("directory").as<std::string>()};
        return manifest;
    } catch (const ManifestError&) {
        throw;
    } catch (const std::exception& error) {
        throw ManifestError{"Cannot decode validated experiment manifest '" +
                            manifest_path.string() + "': " + error.what()};
    }
}

} // namespace mehlissa::experiment

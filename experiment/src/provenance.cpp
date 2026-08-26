// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/provenance.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <picosha2.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace mehlissa::experiment {
namespace {

using Json = jsoncons::json;

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string& role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw ProvenanceError{"Cannot open " + role + ": " + path.string()};
    }

    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw ProvenanceError{"Invalid JSON in " + role + " '" + path.string() +
                              "': " + error.what()};
    }
}

[[nodiscard]] Json make_provenance(const ExperimentManifest& manifest,
                                   const std::filesystem::path& manifest_path,
                                   const RunMetadata& run_metadata,
                                   const BuildMetadata& build_metadata) {
    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", supported_provenance_schema_version},
            {"experiment", Json{jsoncons::json_object_arg,
                                {
                                    {"id", manifest.experiment_id},
                                    {"schema_version", manifest.schema_version},
                                    {"manifest", Json{jsoncons::json_object_arg,
                                                      {
                                                          {"path", manifest_path.generic_string()},
                                                          {"sha256", sha256_file(manifest_path)},
                                                      }}},
                                }}},
            {"software",
             Json{jsoncons::json_object_arg,
                  {
                      {"name", "MEHLISSA"},
                      {"version", build_metadata.software_version},
                      {"git_commit", build_metadata.git_commit},
                      {"git_dirty", build_metadata.git_dirty},
                      {"build_type", build_metadata.build_type},
                      {"compiler", Json{jsoncons::json_object_arg,
                                        {
                                            {"id", build_metadata.compiler_id},
                                            {"version", build_metadata.compiler_version},
                                        }}},
                  }}},
            {"platform", Json{jsoncons::json_object_arg,
                              {
                                  {"os", build_metadata.operating_system},
                                  {"architecture", build_metadata.architecture},
                              }}},
            {"random", Json{jsoncons::json_object_arg,
                            {
                                {"master_seed", manifest.master_seed},
                            }}},
            {"run", Json{jsoncons::json_object_arg,
                         {
                             {"started_at_utc", run_metadata.started_at_utc},
                             {"completed_at_utc", run_metadata.completed_at_utc},
                             {"status", run_metadata.status},
                             {"simulation_time_ns", run_metadata.simulation_time.count()},
                         }}},
        }};
}

void write_json(const Json& document, const std::filesystem::path& path) {
    std::error_code error;
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, error);
        if (error) {
            throw ProvenanceError{"Cannot create provenance directory '" + parent.string() +
                                  "': " + error.message()};
        }
    }

    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        throw ProvenanceError{"Cannot write provenance file: " + path.string()};
    }
    document.dump_pretty(stream);
    stream.put('\n');
    if (!stream) {
        throw ProvenanceError{"Cannot complete provenance file: " + path.string()};
    }
}

} // namespace

BuildMetadata current_build_metadata() {
    return BuildMetadata{
        MEHLISSA_SOFTWARE_VERSION, MEHLISSA_GIT_COMMIT,
        MEHLISSA_GIT_DIRTY != 0,   MEHLISSA_BUILD_TYPE,
        MEHLISSA_COMPILER_ID,      MEHLISSA_COMPILER_VERSION,
        MEHLISSA_PLATFORM_OS,      MEHLISSA_PLATFORM_ARCHITECTURE,
    };
}

std::string current_utc_timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto calendar_time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_time{};
#ifdef _WIN32
    if (gmtime_s(&utc_time, &calendar_time) != 0) {
        throw ProvenanceError{"Cannot convert the current time to UTC"};
    }
#else
    if (gmtime_r(&calendar_time, &utc_time) == nullptr) {
        throw ProvenanceError{"Cannot convert the current time to UTC"};
    }
#endif

    std::ostringstream timestamp;
    timestamp << std::put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ");
    if (!timestamp) {
        throw ProvenanceError{"Cannot format the current UTC timestamp"};
    }
    return timestamp.str();
}

std::string sha256_file(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw ProvenanceError{"Cannot open file for SHA-256: " + path.string()};
    }

    std::vector<unsigned char> digest(picosha2::k_digest_size);
    picosha2::hash256(stream, digest.begin(), digest.end());
    if (stream.bad()) {
        throw ProvenanceError{"Cannot read file for SHA-256: " + path.string()};
    }
    return picosha2::bytes_to_hex_string(digest.begin(), digest.end());
}

void write_provenance(const ExperimentManifest& manifest, const ProvenanceRequest& request,
                      const BuildMetadata& build_metadata) {
    write_json(make_provenance(manifest, request.manifest_path, request.run, build_metadata),
               request.output_path);
}

void write_provenance(const ExperimentManifest& manifest, const ProvenanceRequest& request) {
    write_provenance(manifest, request, current_build_metadata());
}

void validate_provenance_file(const ProvenanceValidation& validation) {
    const auto schema = read_json(validation.schema_path, "provenance schema");
    const auto provenance = read_json(validation.document_path, "provenance file");
    try {
        const auto compiled_schema = jsoncons::jsonschema::make_json_schema(schema);
        compiled_schema.validate(provenance);
    } catch (const std::exception& error) {
        throw ProvenanceError{"Provenance file '" + validation.document_path.string() +
                              "' does not satisfy its schema: " + error.what()};
    }
}

} // namespace mehlissa::experiment

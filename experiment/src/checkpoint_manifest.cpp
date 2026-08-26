// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/checkpoint_manifest.hpp>

#include <mehlissa/experiment/provenance.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::experiment {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw CheckpointError{core::ErrorCode::input_unreadable,
                              "Cannot open " + std::string{role} + ": " + path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw CheckpointError{core::ErrorCode::json_invalid,
                              "Invalid JSON in " + std::string{role} + " '" + path.string() +
                                  "': " + error.what()};
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw CheckpointError{core::ErrorCode::schema_invalid,
                              "Invalid checkpoint schema '" + path.string() + "': " + error.what()};
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                              "Checkpoint manifest '" + path.string() +
                                  "' does not satisfy its schema: " + error.what()};
    }
}

void validate_state_file(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
        throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                              "Component snapshot file must be a non-empty relative path"};
    }
    for (const auto& part : path) {
        if (part == "..") {
            throw CheckpointError{
                core::ErrorCode::checkpoint_invalid,
                "Component snapshot file must remain inside the checkpoint directory"};
        }
    }
}

void validate_semantics(const CheckpointManifest& checkpoint) {
    std::unordered_set<std::string> stream_names;
    for (const auto& stream : checkpoint.random_streams) {
        if (!stream_names.insert(stream.name).second) {
            throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                                  "Duplicate random-stream checkpoint: " + stream.name};
        }
    }

    std::unordered_set<std::string> component_names;
    for (const auto& component : checkpoint.components) {
        if (!component_names.insert(component.name).second) {
            throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                                  "Duplicate component snapshot: " + component.name};
        }
        validate_state_file(component.state_file);
    }
}

void verify_component_snapshots(const CheckpointManifest& checkpoint,
                                const std::filesystem::path& checkpoint_directory) {
    if (checkpoint.components.empty()) {
        return;
    }

    std::error_code path_error;
    const auto directory = checkpoint_directory.empty() ? std::filesystem::current_path(path_error)
                                                        : checkpoint_directory;
    if (path_error) {
        throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                              "Cannot resolve checkpoint directory: " + path_error.message()};
    }
    const auto canonical_directory = std::filesystem::weakly_canonical(directory, path_error);
    if (path_error) {
        throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                              "Cannot resolve checkpoint directory '" + directory.string() +
                                  "': " + path_error.message()};
    }

    for (const auto& component : checkpoint.components) {
        const auto state_path = directory / component.state_file;
        const auto canonical_state_path = std::filesystem::weakly_canonical(state_path, path_error);
        if (path_error) {
            throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                                  "Cannot resolve component snapshot '" + state_path.string() +
                                      "': " + path_error.message()};
        }
        const auto relative_state_path =
            canonical_state_path.lexically_relative(canonical_directory);
        validate_state_file(relative_state_path);
        try {
            if (sha256_file(canonical_state_path) != component.sha256) {
                throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                                      "Component snapshot checksum mismatch: " +
                                          canonical_state_path.string()};
            }
        } catch (const CheckpointError&) {
            throw;
        } catch (const core::MehlissaError& error) {
            throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                                  "Cannot verify component snapshot '" +
                                      canonical_state_path.string() + "': " + error.what()};
        }
    }
}

[[nodiscard]] Json make_document(const CheckpointManifest& checkpoint) {
    auto random_streams = checkpoint.random_streams;
    std::ranges::sort(random_streams, {}, &core::RandomStreamState::name);
    Json random_stream_documents = Json::array();
    for (const auto& stream : random_streams) {
        random_stream_documents.push_back(Json{jsoncons::json_object_arg,
                                               {
                                                   {"name", stream.name},
                                                   {"draw_count", stream.draw_count},
                                               }});
    }

    auto components = checkpoint.components;
    std::ranges::sort(components, {}, &ComponentSnapshotReference::name);
    Json component_documents = Json::array();
    for (const auto& component : components) {
        component_documents.push_back(
            Json{jsoncons::json_object_arg,
                 {
                     {"name", component.name},
                     {"state_schema_version", component.state_schema_version},
                     {"state_file", component.state_file.generic_string()},
                     {"sha256", component.sha256},
                 }});
    }

    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", checkpoint.schema_version},
            {"experiment", Json{jsoncons::json_object_arg,
                                {
                                    {"id", checkpoint.experiment_id},
                                    {"manifest_sha256", checkpoint.experiment_manifest_sha256},
                                }}},
            {"software", Json{jsoncons::json_object_arg,
                              {
                                  {"version", checkpoint.software_version},
                              }}},
            {"checkpoint", Json{jsoncons::json_object_arg,
                                {
                                    {"sequence", checkpoint.sequence},
                                    {"created_at_utc", checkpoint.created_at_utc},
                                    {"simulation_time_ns", checkpoint.simulation_time.count()},
                                }}},
            {"random", Json{jsoncons::json_object_arg,
                            {
                                {"master_seed", checkpoint.master_seed},
                                {"streams", std::move(random_stream_documents)},
                            }}},
            {"components", std::move(component_documents)},
        }};
}

void write_json(const Json& document, const std::filesystem::path& path) {
    std::error_code error;
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, error);
        if (error) {
            throw CheckpointError{core::ErrorCode::output_unwritable,
                                  "Cannot create checkpoint directory '" + parent.string() +
                                      "': " + error.message()};
        }
    }

    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        throw CheckpointError{core::ErrorCode::output_unwritable,
                              "Cannot write checkpoint manifest: " + path.string()};
    }
    document.dump_pretty(stream);
    stream.put('\n');
    if (!stream) {
        throw CheckpointError{core::ErrorCode::output_unwritable,
                              "Cannot complete checkpoint manifest: " + path.string()};
    }
}

[[nodiscard]] CheckpointManifest decode(const Json& document) {
    CheckpointManifest result;
    result.schema_version = document.at("schema_version").as<std::string>();
    result.experiment_id = document.at("experiment").at("id").as<std::string>();
    result.experiment_manifest_sha256 =
        document.at("experiment").at("manifest_sha256").as<std::string>();
    result.software_version = document.at("software").at("version").as<std::string>();
    result.sequence = document.at("checkpoint").at("sequence").as<std::uint64_t>();
    result.created_at_utc = document.at("checkpoint").at("created_at_utc").as<std::string>();
    result.simulation_time = core::SimulationClock::Duration{
        document.at("checkpoint").at("simulation_time_ns").as<std::int64_t>()};
    result.master_seed = document.at("random").at("master_seed").as<std::uint64_t>();

    for (const auto& stream : document.at("random").at("streams").array_range()) {
        result.random_streams.push_back({
            stream.at("name").as<std::string>(),
            stream.at("draw_count").as<std::uint64_t>(),
        });
    }
    for (const auto& component : document.at("components").array_range()) {
        result.components.push_back({
            component.at("name").as<std::string>(),
            component.at("state_schema_version").as<std::string>(),
            component.at("state_file").as<std::string>(),
            component.at("sha256").as<std::string>(),
        });
    }
    return result;
}

} // namespace

void write_checkpoint_manifest(const CheckpointManifest& checkpoint,
                               const CheckpointWriteRequest& request) {
    validate_semantics(checkpoint);
    verify_component_snapshots(checkpoint, request.output_path.parent_path());
    const auto schema_document = read_json(request.schema_path, "checkpoint schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto document = make_document(checkpoint);
    validate_document(document, schema, request.output_path);
    write_json(document, request.output_path);
}

CheckpointManifest load_checkpoint_manifest(const CheckpointLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "checkpoint schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto document = read_json(request.checkpoint_path, "checkpoint manifest");
    validate_document(document, schema, request.checkpoint_path);
    try {
        auto checkpoint = decode(document);
        validate_semantics(checkpoint);
        verify_component_snapshots(checkpoint, request.checkpoint_path.parent_path());
        return checkpoint;
    } catch (const CheckpointError&) {
        throw;
    } catch (const std::exception& error) {
        throw CheckpointError{core::ErrorCode::checkpoint_invalid,
                              "Cannot decode checkpoint manifest '" +
                                  request.checkpoint_path.string() + "': " + error.what()};
    }
}

} // namespace mehlissa::experiment

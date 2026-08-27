// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/experiment/checkpoint_manifest.hpp>
#include <mehlissa/experiment/experiment_manifest.hpp>
#include <mehlissa/experiment/provenance.hpp>
#include <mehlissa/experiment/run_log.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace {

constexpr auto default_schema_path = "data/schemas/experiment/1.0.0.schema.json";
constexpr auto default_checkpoint_schema_path = "data/schemas/checkpoint/1.0.0.schema.json";
constexpr auto default_body_schema_path = "data/schemas/vascular-graph/1.0.0.schema.json";

struct CommandLine final {
    enum class Operation : std::uint8_t { run, validate, validate_body };

    Operation operation{};
    std::filesystem::path experiment_path;
    std::filesystem::path body_model_path;
    std::filesystem::path schema_path{default_schema_path};
    std::filesystem::path checkpoint_schema_path{default_checkpoint_schema_path};
};

void print_usage() {
    std::fputs("Usage:\n"
               "  mehlissa run --experiment <file> [--schema <file>] "
               "[--checkpoint-schema <file>]\n"
               "  mehlissa validate --experiment <file> [--schema <file>]\n"
               "  mehlissa validate-body --model <file> [--schema <file>]\n",
               stderr);
}

[[nodiscard]] CommandLine parse_command_line(const int argc, const char* const argv[]) {
    if (argc < 2) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                            "Missing command"};
    }

    CommandLine command;
    const std::string_view operation{argv[1]};
    if (operation == "run") {
        command.operation = CommandLine::Operation::run;
    } else if (operation == "validate") {
        command.operation = CommandLine::Operation::validate;
    } else if (operation == "validate-body") {
        command.operation = CommandLine::Operation::validate_body;
        command.schema_path = default_body_schema_path;
    } else {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                            "Unknown command: " + std::string{operation}};
    }

    for (int argument = 2; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Missing value for option: " +
                                                    std::string{argv[argument]}};
        }

        const std::string_view option{argv[argument]};
        if (option == "--experiment") {
            command.experiment_path = argv[argument + 1];
        } else if (option == "--model") {
            command.body_model_path = argv[argument + 1];
        } else if (option == "--schema") {
            command.schema_path = argv[argument + 1];
        } else if (option == "--checkpoint-schema") {
            command.checkpoint_schema_path = argv[argument + 1];
        } else {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Unknown option: " + std::string{option}};
        }
    }

    if (command.operation == CommandLine::Operation::validate_body) {
        if (command.body_model_path.empty()) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "--model is required"};
        }
    } else if (command.experiment_path.empty()) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                            "--experiment is required"};
    }
    return command;
}

void print_manifest(const mehlissa::experiment::ExperimentManifest& manifest) {
    const auto duration_ns = static_cast<std::int64_t>(manifest.duration.count());
    std::printf("experiment_id=%s\n", manifest.experiment_id.c_str());
    std::printf("schema_version=%s\n", manifest.schema_version.c_str());
    std::printf("duration_ns=%" PRId64 "\n", duration_ns);
    std::printf("master_seed=%" PRIu64 "\n", manifest.master_seed);
    std::printf("model_count=%zu\n", manifest.models.size());
    std::printf("output_directory=%s\n", manifest.output_directory.string().c_str());
}

[[nodiscard]] int exit_status(const mehlissa::core::ErrorCode code) noexcept {
    using mehlissa::core::ErrorCode;
    switch (code) {
    case ErrorCode::command_line_invalid:
        return 2;
    case ErrorCode::input_unreadable:
    case ErrorCode::json_invalid:
    case ErrorCode::schema_invalid:
    case ErrorCode::manifest_invalid:
    case ErrorCode::data_invalid:
        return 3;
    case ErrorCode::output_unwritable:
    case ErrorCode::provenance_invalid:
        return 4;
    case ErrorCode::lifecycle_invalid:
    case ErrorCode::invariant_violated:
    case ErrorCode::numeric_overflow:
        return 5;
    case ErrorCode::checkpoint_invalid:
    case ErrorCode::checkpoint_incompatible:
        return 6;
    case ErrorCode::internal_failure:
        return 1;
    }
    return 1;
}

void record_failure(mehlissa::experiment::JsonLinesRunLog& log,
                    const mehlissa::core::SimulationClock& clock,
                    const mehlissa::core::ErrorCode code, const std::string_view message) noexcept {
    try {
        log.write({
            mehlissa::experiment::current_utc_timestamp(),
            clock.now(),
            mehlissa::experiment::LogLevel::error,
            "runner",
            "run_failed",
            std::string{message},
            code,
        });
    } catch (...) { // NOLINT(bugprone-empty-catch)
        // Preserve the original failure when the best-effort diagnostic cannot be written.
    }
}

int execute(const CommandLine& command) {
    if (command.operation == CommandLine::Operation::validate_body) {
        const auto graph = mehlissa::models::body::load_vascular_graph(
            {command.body_model_path, command.schema_path});
        std::printf("Vascular graph is valid: %s (%zu segments)\n", graph.model_id.c_str(),
                    graph.segments.size());
        return 0;
    }

    const auto manifest = mehlissa::experiment::load_experiment_manifest(command.experiment_path,
                                                                         command.schema_path);

    if (command.operation == CommandLine::Operation::validate) {
        std::printf("Experiment manifest is valid: %s\n", manifest.experiment_id.c_str());
        return 0;
    }

    mehlissa::core::ComponentHost simulation{manifest.master_seed};
    const auto provenance_path = manifest.output_directory / "provenance.json";
    const auto checkpoint_path = manifest.output_directory / "checkpoint-000000.json";
    const auto run_log_path = manifest.output_directory / "run.log.jsonl";
    mehlissa::experiment::JsonLinesRunLog run_log{run_log_path};

    try {
        const auto started_at = mehlissa::experiment::current_utc_timestamp();
        run_log.write({
            started_at,
            simulation.context().clock().now(),
            mehlissa::experiment::LogLevel::info,
            "runner",
            "run_started",
            "Simulation run started",
            std::nullopt,
        });

        simulation.initialize();
        simulation.advance(manifest.duration);
        simulation.finalize();

        const auto completed_at = mehlissa::experiment::current_utc_timestamp();
        const auto build_metadata = mehlissa::experiment::current_build_metadata();
        const mehlissa::experiment::CheckpointManifest checkpoint{
            mehlissa::experiment::supported_checkpoint_schema_version,
            manifest.experiment_id,
            mehlissa::experiment::sha256_file(command.experiment_path),
            build_metadata.software_version,
            0,
            completed_at,
            simulation.context().clock().now(),
            simulation.context().master_seed(),
            simulation.context().random_stream_states(),
            {},
        };
        mehlissa::experiment::write_checkpoint_manifest(
            checkpoint, {checkpoint_path, command.checkpoint_schema_path});

        const mehlissa::experiment::ProvenanceRequest provenance_request{
            command.experiment_path,
            provenance_path,
            {
                started_at,
                completed_at,
                "completed",
                simulation.context().clock().now(),
            },
        };
        mehlissa::experiment::write_provenance(manifest, provenance_request);
        run_log.write({
            completed_at,
            simulation.context().clock().now(),
            mehlissa::experiment::LogLevel::info,
            "runner",
            "run_completed",
            "Simulation run completed",
            std::nullopt,
        });
    } catch (const mehlissa::core::MehlissaError& error) {
        record_failure(run_log, simulation.context().clock(), error.code(), error.what());
        throw;
    } catch (const std::exception& error) {
        record_failure(run_log, simulation.context().clock(),
                       mehlissa::core::ErrorCode::internal_failure, error.what());
        throw;
    }

    print_manifest(manifest);
    std::printf("simulation_time_ns=%" PRId64 "\n",
                static_cast<std::int64_t>(simulation.context().clock().now().count()));
    std::printf("provenance_file=%s\n", provenance_path.string().c_str());
    std::printf("checkpoint_file=%s\n", checkpoint_path.string().c_str());
    std::printf("run_log_file=%s\n", run_log_path.string().c_str());
    return 0;
}

} // namespace

int main(const int argc, const char* const argv[]) noexcept {
    try {
        return execute(parse_command_line(argc, argv));
    } catch (const mehlissa::core::MehlissaError& error) {
        const auto code_id = error.code_id();
        std::fprintf(stderr, "[%.*s] %s\n", static_cast<int>(code_id.size()), code_id.data(),
                     error.what());
        if (error.code() == mehlissa::core::ErrorCode::command_line_invalid) {
            print_usage();
        }
        return exit_status(error.code());
    } catch (const std::exception& error) {
        std::fprintf(stderr, "[MEHLISSA-E9001] MEHLISSA failed: %s\n", error.what());
    } catch (...) {
        std::fputs("[MEHLISSA-E9001] MEHLISSA failed with an unknown error\n", stderr);
    }
    return 1;
}

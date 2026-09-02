// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/experiment/checkpoint_manifest.hpp>
#include <mehlissa/experiment/experiment_manifest.hpp>
#include <mehlissa/experiment/provenance.hpp>
#include <mehlissa/experiment/run_log.hpp>
#include <mehlissa/models/body/body_state_profile.hpp>
#include <mehlissa/models/body/bvs_reference.hpp>
#include <mehlissa/models/body/legacy_95_migration.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include "campaign_cli.hpp"
#include "discovery_cli.hpp"
#include "reporting_cli.hpp"
#include "scenario_cli.hpp"

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
constexpr auto default_bvs_report_schema_path =
    "data/schemas/bvs-reference-report/1.0.0.schema.json";
constexpr auto default_body_state_schema_path = "data/schemas/body-state-profile/1.0.0.schema.json";

struct CommandLine final {
    enum class Operation : std::uint8_t {
        run,
        validate,
        validate_body,
        migrate_legacy_95,
        reference_bvs,
        apply_body_state
    };

    Operation operation{};
    std::filesystem::path experiment_path;
    std::filesystem::path body_model_path;
    std::filesystem::path vasculature_path;
    std::filesystem::path transitions_path;
    std::filesystem::path output_path;
    std::filesystem::path profile_path;
    std::filesystem::path schema_path{default_schema_path};
    std::filesystem::path checkpoint_schema_path{default_checkpoint_schema_path};
    std::filesystem::path report_schema_path{default_bvs_report_schema_path};
    std::filesystem::path profile_schema_path{default_body_state_schema_path};
};

void print_usage() {
    std::fputs("Usage:\n"
               "  mehlissa run --experiment <file> [--schema <file>] "
               "[--checkpoint-schema <file>]\n"
               "  mehlissa validate --experiment <file> [--schema <file>]\n"
               "  mehlissa validate-body --model <file> [--schema <file>]\n"
               "  mehlissa migrate-legacy-95 --vasculature <file> --transitions <file> "
               "--output <file> [--schema <file>]\n"
               "  mehlissa reference-bvs --model <file> --output <file> "
               "[--schema <file>] [--report-schema <file>]\n"
               "  mehlissa apply-body-state --model <file> --profile <file> --output <file> "
               "[--schema <file>] [--profile-schema <file>]\n",
               stderr);
    mehlissa::apps::print_usability_usage();
    mehlissa::apps::print_discovery_usage();
    mehlissa::apps::print_reporting_usage();
    mehlissa::apps::print_campaign_usage();
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
    } else if (operation == "migrate-legacy-95") {
        command.operation = CommandLine::Operation::migrate_legacy_95;
        command.schema_path = default_body_schema_path;
    } else if (operation == "reference-bvs") {
        command.operation = CommandLine::Operation::reference_bvs;
        command.schema_path = default_body_schema_path;
    } else if (operation == "apply-body-state") {
        command.operation = CommandLine::Operation::apply_body_state;
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
        } else if (option == "--vasculature") {
            command.vasculature_path = argv[argument + 1];
        } else if (option == "--transitions") {
            command.transitions_path = argv[argument + 1];
        } else if (option == "--output") {
            command.output_path = argv[argument + 1];
        } else if (option == "--profile") {
            command.profile_path = argv[argument + 1];
        } else if (option == "--schema") {
            command.schema_path = argv[argument + 1];
        } else if (option == "--checkpoint-schema") {
            command.checkpoint_schema_path = argv[argument + 1];
        } else if (option == "--report-schema") {
            command.report_schema_path = argv[argument + 1];
        } else if (option == "--profile-schema") {
            command.profile_schema_path = argv[argument + 1];
        } else {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Unknown option: " + std::string{option}};
        }
    }

    if (command.operation == CommandLine::Operation::migrate_legacy_95) {
        if (command.vasculature_path.empty() || command.transitions_path.empty() ||
            command.output_path.empty()) {
            throw mehlissa::core::MehlissaError{
                mehlissa::core::ErrorCode::command_line_invalid,
                "--vasculature, --transitions and --output are required"};
        }
    } else if (command.operation == CommandLine::Operation::apply_body_state) {
        if (command.body_model_path.empty() || command.profile_path.empty() ||
            command.output_path.empty()) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "--model, --profile and --output are required"};
        }
    } else if (command.operation == CommandLine::Operation::reference_bvs) {
        if (command.body_model_path.empty() || command.output_path.empty()) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "--model and --output are required"};
        }
    } else if (command.operation == CommandLine::Operation::validate_body) {
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
    if (command.operation == CommandLine::Operation::apply_body_state) {
        const auto graph = mehlissa::models::body::load_vascular_graph(
            {command.body_model_path, command.schema_path});
        const auto profile = mehlissa::models::body::load_body_state_profile(
            {command.profile_path, command.profile_schema_path});
        const auto derived = mehlissa::models::body::apply_body_state_profile(graph, profile);
        mehlissa::models::body::write_vascular_graph(derived, command.output_path);
        static_cast<void>(mehlissa::models::body::load_vascular_graph(
            {command.output_path, command.schema_path}));
        std::printf("Applied body-state profile: %s x %.6f -> %s (%zu segments) -> %s\n",
                    profile.profile_id.c_str(), profile.cardiac_output_multiplier,
                    derived.model_id.c_str(), derived.segments.size(),
                    command.output_path.string().c_str());
        return 0;
    }
    if (command.operation == CommandLine::Operation::reference_bvs) {
        const auto graph = mehlissa::models::body::load_vascular_graph(
            {command.body_model_path, command.schema_path});
        const auto report = mehlissa::models::body::run_bvs_reference(graph);
        mehlissa::models::body::write_bvs_reference_report(
            report, {command.output_path, command.report_schema_path});
        std::printf("M2.4 BVS reference %s: equilibrium=%.6f%%, injection-site=%.6f%%, "
                    "population-scale=%.6f%%, perfusion-mean=%.6f pp -> %s\n",
                    report.overall_passed ? "passed" : "failed",
                    report.minute_7_vs_minute_120.normalized_mean_difference_percent,
                    report.aorta_vs_popliteal_at_minute_7.normalized_mean_difference_percent,
                    report.population_scale_total_variation_percent,
                    report.mean_perfusion_error_vs_literature_percentage_points,
                    command.output_path.string().c_str());
        if (!report.overall_passed) {
            throw mehlissa::core::MehlissaError{
                mehlissa::core::ErrorCode::data_invalid,
                "M2.4 BVS reference failed one or more pre-defined acceptance gates"};
        }
        return 0;
    }
    if (command.operation == CommandLine::Operation::migrate_legacy_95) {
        const auto graph = mehlissa::models::body::migrate_legacy_95(
            {command.vasculature_path, command.transitions_path});
        mehlissa::models::body::write_vascular_graph(graph, command.output_path);
        static_cast<void>(mehlissa::models::body::load_vascular_graph(
            {command.output_path, command.schema_path}));
        std::printf("Migrated vascular graph: %s (%zu segments) -> %s\n", graph.model_id.c_str(),
                    graph.segments.size(), command.output_path.string().c_str());
        return 0;
    }
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
        if (argc >= 2 &&
            (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h")) {
            print_usage();
            return 0;
        }
        if (mehlissa::apps::handles_campaign_command(argc, argv)) {
            return mehlissa::apps::execute_campaign_command(argc, argv);
        }
        if (mehlissa::apps::handles_reporting_command(argc, argv)) {
            return mehlissa::apps::execute_reporting_command(argc, argv);
        }
        if (mehlissa::apps::handles_usability_command(argc, argv)) {
            return mehlissa::apps::execute_usability_command(argc, argv);
        }
        if (mehlissa::apps::handles_discovery_command(argc, argv)) {
            return mehlissa::apps::execute_discovery_command(argc, argv);
        }
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

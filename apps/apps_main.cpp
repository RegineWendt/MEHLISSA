// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/experiment/experiment_manifest.hpp>

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr auto default_schema_path = "data/schemas/experiment/1.0.0.schema.json";

struct CommandLine final {
    enum class Operation : std::uint8_t { run, validate };

    Operation operation{};
    std::filesystem::path experiment_path;
    std::filesystem::path schema_path{default_schema_path};
};

void print_usage() {
    std::fputs("Usage:\n"
               "  mehlissa run --experiment <file> [--schema <file>]\n"
               "  mehlissa validate --experiment <file> [--schema <file>]\n",
               stderr);
}

[[nodiscard]] CommandLine parse_command_line(const int argc, const char* const argv[]) {
    if (argc < 2) {
        throw std::invalid_argument{"Missing command"};
    }

    CommandLine command;
    const std::string_view operation{argv[1]};
    if (operation == "run") {
        command.operation = CommandLine::Operation::run;
    } else if (operation == "validate") {
        command.operation = CommandLine::Operation::validate;
    } else {
        throw std::invalid_argument{"Unknown command: " + std::string{operation}};
    }

    for (int argument = 2; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            throw std::invalid_argument{"Missing value for option: " + std::string{argv[argument]}};
        }

        const std::string_view option{argv[argument]};
        if (option == "--experiment") {
            command.experiment_path = argv[argument + 1];
        } else if (option == "--schema") {
            command.schema_path = argv[argument + 1];
        } else {
            throw std::invalid_argument{"Unknown option: " + std::string{option}};
        }
    }

    if (command.experiment_path.empty()) {
        throw std::invalid_argument{"--experiment is required"};
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

int execute(const CommandLine& command) {
    const auto manifest = mehlissa::experiment::load_experiment_manifest(command.experiment_path,
                                                                         command.schema_path);

    if (command.operation == CommandLine::Operation::validate) {
        std::printf("Experiment manifest is valid: %s\n", manifest.experiment_id.c_str());
        return 0;
    }

    mehlissa::core::SimulationClock clock;
    clock.advance(manifest.duration);
    print_manifest(manifest);
    std::printf("simulation_time_ns=%" PRId64 "\n", static_cast<std::int64_t>(clock.now().count()));
    return 0;
}

} // namespace

int main(const int argc, const char* const argv[]) noexcept {
    try {
        return execute(parse_command_line(argc, argv));
    } catch (const std::invalid_argument& error) {
        std::fprintf(stderr, "Command-line error: %s\n", error.what());
        print_usage();
        return 2;
    } catch (const mehlissa::experiment::ManifestError& error) {
        std::fprintf(stderr, "Experiment validation failed: %s\n", error.what());
        return 3;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "MEHLISSA failed: %s\n", error.what());
    } catch (...) {
        std::fputs("MEHLISSA failed with an unknown error\n", stderr);
    }
    return 1;
}

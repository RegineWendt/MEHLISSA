// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "scenario_cli.hpp"

#include <mehlissa/core/error.hpp>
#include <mehlissa/experiment/provenance.hpp>
#include <mehlissa/experiment/run_log.hpp>
#include <mehlissa/scenarios/fingerprinting/level_a_composer.hpp>
#include <mehlissa/scenarios/fingerprinting/level_e_analysis.hpp>
#include <mehlissa/scenarios/fingerprinting/result_report.hpp>
#include <mehlissa/scenarios/fingerprinting/scenario_profile.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace mehlissa::apps {
namespace {

using Json = jsoncons::json;
using core::ErrorCode;
using scenarios::fingerprinting::HolisticFingerprintingResultReport;
using scenarios::fingerprinting::LevelAPlan;
using scenarios::fingerprinting::ScenarioProfile;

constexpr auto scenario_schema_relative =
    "data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json";
constexpr auto result_schema_relative = "data/schemas/fingerprinting-result/2.0.0.schema.json";
constexpr auto provenance_schema_relative =
    "data/schemas/scenario-run-provenance/1.0.0.schema.json";
constexpr auto log_schema_relative = "data/schemas/log-record/1.0.0.schema.json";

enum class UsabilityOperation : std::uint8_t {
    scenario_list,
    scenario_validate,
    scenario_run,
    result_summarize
};

struct UsabilityCommand final {
    UsabilityOperation operation{};
    std::filesystem::path file;
    std::filesystem::path output{"results"};
    std::filesystem::path repository_root;
    std::filesystem::path schema;
    std::filesystem::path result_schema;
    std::filesystem::path provenance_schema;
    std::filesystem::path log_schema;
};

struct PreparedScenario final {
    std::filesystem::path repository_root;
    std::filesystem::path profile_path;
    std::filesystem::path scenario_schema_path;
    ScenarioProfile profile;
    LevelAPlan plan;
};

struct RunPaths final {
    std::filesystem::path directory;
    std::filesystem::path result;
    std::filesystem::path provenance;
    std::filesystem::path log;
    std::filesystem::path summary;
};

struct RunSchemas final {
    std::filesystem::path result;
    std::filesystem::path provenance;
    std::filesystem::path log;
};

[[noreturn]] void invalid_command(const std::string& message) {
    throw core::MehlissaError{ErrorCode::command_line_invalid, message};
}

[[noreturn]] void input_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::input_unreadable, message};
}

[[noreturn]] void output_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::output_unwritable, message};
}

[[nodiscard]] bool is_help(const std::string_view value) noexcept {
    return value == "--help" || value == "-h";
}

[[nodiscard]] UsabilityOperation parse_operation(const int argc, const char* const argv[]) {
    if (argc < 3) {
        invalid_command("The scenario or result command requires a subcommand");
    }
    const std::string_view family{argv[1]};
    const std::string_view action{argv[2]};
    if (family == "scenario" && action == "list") {
        return UsabilityOperation::scenario_list;
    }
    if (family == "scenario" && action == "validate") {
        return UsabilityOperation::scenario_validate;
    }
    if (family == "scenario" && action == "run") {
        return UsabilityOperation::scenario_run;
    }
    if (family == "result" && action == "summarize") {
        return UsabilityOperation::result_summarize;
    }
    invalid_command("Unknown scenario or result subcommand: " + std::string{action});
}

void assign_option(UsabilityCommand& command, const std::string_view option,
                   const std::filesystem::path& value) {
    if (option == "--file") {
        command.file = value;
    } else if (option == "--output") {
        command.output = value;
    } else if (option == "--repository-root") {
        command.repository_root = value;
    } else if (option == "--schema") {
        command.schema = value;
    } else if (option == "--result-schema") {
        command.result_schema = value;
    } else if (option == "--provenance-schema") {
        command.provenance_schema = value;
    } else if (option == "--log-schema") {
        command.log_schema = value;
    } else {
        invalid_command("Unknown option for scenario or result command: " + std::string{option});
    }
}

void reject_disallowed_options(const UsabilityCommand& command) {
    if (command.operation == UsabilityOperation::scenario_list) {
        if (!command.file.empty() || command.output != "results" || !command.schema.empty() ||
            !command.result_schema.empty() || !command.provenance_schema.empty() ||
            !command.log_schema.empty()) {
            invalid_command("scenario list only accepts --repository-root");
        }
        return;
    }
    if (command.file.empty()) {
        invalid_command("--file is required");
    }
    if (command.operation == UsabilityOperation::scenario_validate &&
        (command.output != "results" || !command.result_schema.empty() ||
         !command.provenance_schema.empty() || !command.log_schema.empty())) {
        invalid_command("scenario validate only accepts --file, --repository-root and --schema");
    }
    if (command.operation == UsabilityOperation::result_summarize &&
        (command.output != "results" || !command.schema.empty() ||
         !command.provenance_schema.empty() || !command.log_schema.empty())) {
        invalid_command(
            "result summarize only accepts --file, --repository-root and --result-schema");
    }
}

[[nodiscard]] UsabilityCommand parse_command(const int argc, const char* const argv[]) {
    UsabilityCommand command;
    command.operation = parse_operation(argc, argv);
    for (int argument = 3; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            invalid_command("Missing value for option: " + std::string{argv[argument]});
        }
        assign_option(command, argv[argument], argv[argument + 1]);
    }
    reject_disallowed_options(command);
    return command;
}

[[nodiscard]] bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path / "CMakeLists.txt", error) && !error &&
           std::filesystem::is_regular_file(path / scenario_schema_relative, error) && !error;
}

[[nodiscard]] std::optional<std::filesystem::path>
search_repository_ancestors(std::filesystem::path path) {
    std::error_code error;
    path = std::filesystem::absolute(path, error);
    if (error) {
        return std::nullopt;
    }
    if (!std::filesystem::is_directory(path, error)) {
        path = path.parent_path();
    }
    while (!path.empty()) {
        if (looks_like_repository_root(path)) {
            return std::filesystem::weakly_canonical(path, error);
        }
        const auto parent = path.parent_path();
        if (parent == path) {
            break;
        }
        path = parent;
    }
    return std::nullopt;
}

[[nodiscard]] std::filesystem::path locate_repository_root(const UsabilityCommand& command) {
    if (!command.repository_root.empty()) {
        std::error_code error;
        const auto root = std::filesystem::weakly_canonical(command.repository_root, error);
        if (error || !looks_like_repository_root(root)) {
            input_error("--repository-root is not a MEHLISSA source tree: " +
                        command.repository_root.string());
        }
        return root;
    }
    if (!command.file.empty()) {
        const auto from_file = search_repository_ancestors(command.file);
        if (from_file.has_value()) {
            return *from_file;
        }
    }
    const auto from_working_directory =
        search_repository_ancestors(std::filesystem::current_path());
    if (from_working_directory.has_value()) {
        return *from_working_directory;
    }
    input_error("Cannot locate the MEHLISSA repository; run from the source tree or pass "
                "--repository-root <directory>");
}

[[nodiscard]] std::filesystem::path resolve_input(const std::filesystem::path& value,
                                                  const std::filesystem::path& root) {
    if (value.is_absolute()) {
        return value;
    }
    std::error_code error;
    auto from_working_directory = std::filesystem::absolute(value, error);
    if (!error && std::filesystem::is_regular_file(from_working_directory, error) && !error) {
        return from_working_directory;
    }
    return root / value;
}

[[nodiscard]] std::filesystem::path option_or_default(const std::filesystem::path& option,
                                                      const std::filesystem::path& root,
                                                      const std::filesystem::path& fallback) {
    return option.empty() ? root / fallback : resolve_input(option, root);
}

[[nodiscard]] PreparedScenario prepare_scenario(const UsabilityCommand& command) {
    const auto root = locate_repository_root(command);
    const auto profile_path = resolve_input(command.file, root);
    const auto schema_path = option_or_default(command.schema, root, scenario_schema_relative);
    auto profile = scenarios::fingerprinting::load_scenario_profile({profile_path, schema_path});
    auto plan = scenarios::fingerprinting::compose_level_a_plan(profile, root);
    return {root, profile_path, schema_path, std::move(profile), std::move(plan)};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        input_error("Cannot open " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw core::MehlissaError{ErrorCode::json_invalid, "Invalid JSON in " + std::string{role} +
                                                               " '" + path.string() +
                                                               "': " + error.what()};
    }
}

[[nodiscard]] Json read_validated_json(const std::filesystem::path& document_path,
                                       const std::filesystem::path& schema_path,
                                       const std::string_view role) {
    const auto schema_document = read_json(schema_path, "schema for " + std::string{role});
    const auto document = read_json(document_path, role);
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        schema.validate(document);
        return document;
    } catch (const std::exception& error) {
        throw core::MehlissaError{ErrorCode::data_invalid,
                                  std::string{role} +
                                      " does not satisfy its schema: " + error.what()};
    }
}

void write_validated_json(const Json& document, const std::filesystem::path& output_path,
                          const std::string_view role, const std::filesystem::path& schema_path) {
    const auto schema_document = read_json(schema_path, "schema for " + std::string{role});
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        schema.validate(document);
    } catch (const std::exception& error) {
        throw core::MehlissaError{ErrorCode::provenance_invalid,
                                  std::string{role} +
                                      " does not satisfy its schema: " + error.what()};
    }
    std::ofstream stream{output_path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        output_error("Cannot write " + std::string{role} + ": " + output_path.string());
    }
    document.dump_pretty(stream);
    stream << '\n';
    if (!stream) {
        output_error("Failed while writing " + std::string{role} + ": " + output_path.string());
    }
}

[[nodiscard]] std::string portable_identifier(std::string value) {
    std::ranges::transform(value, value.begin(), [](const unsigned char character) {
        return std::isalnum(character) != 0 || character == '-' || character == '_'
                   ? static_cast<char>(character)
                   : '_';
    });
    return value;
}

[[nodiscard]] std::string compact_timestamp(std::string timestamp) {
    timestamp.erase(
        std::remove_if(timestamp.begin(), timestamp.end(),
                       [](const char character) { return character == '-' || character == ':'; }),
        timestamp.end());
    return timestamp;
}

[[nodiscard]] RunPaths create_run_paths(const std::filesystem::path& output_root,
                                        const ScenarioProfile& profile,
                                        const std::string& started_at) {
    std::error_code error;
    std::filesystem::create_directories(output_root, error);
    if (error) {
        output_error("Cannot create scenario output root '" + output_root.string() +
                     "': " + error.message());
    }
    const auto stem =
        portable_identifier(profile.scenario.id) + "-" + compact_timestamp(started_at);
    for (std::size_t suffix = 0; suffix < 1000; ++suffix) {
        auto directory = output_root / stem;
        if (suffix != 0) {
            directory += "-" + std::to_string(suffix);
        }
        if (std::filesystem::create_directory(directory, error)) {
            return {directory, directory / "result.json", directory / "provenance.json",
                    directory / "run.log.jsonl", directory / "summary.txt"};
        }
        if (error) {
            output_error("Cannot create unique scenario run directory '" + directory.string() +
                         "': " + error.message());
        }
    }
    output_error("Cannot allocate a unique scenario run directory below " + output_root.string());
}

[[nodiscard]] std::string percentage(const Json& interval) {
    if (interval.is_null()) {
        return "not estimable";
    }
    std::ostringstream output;
    output.setf(std::ios::fixed);
    output.precision(1);
    output << interval.at("estimate").as<double>() * 100.0 << "% (95% interval "
           << interval.at("lower_95").as<double>() * 100.0 << "-"
           << interval.at("upper_95").as<double>() * 100.0 << "%)";
    return output.str();
}

[[nodiscard]] std::string summarize_result(const Json& document) {
    const auto& scenario = document.at("scenario");
    const auto& run = document.at("run");
    const auto& target = document.at("target");
    const auto& detection = document.at("level_b_detection");
    const auto& assembly = document.at("level_c_assembly");
    const auto& communication = document.at("level_d_communication");
    const auto& analysis = document.at("level_e_analysis").at("summary");
    const auto& validity = document.at("validity");

    std::ostringstream output;
    output << "MEHLISSA fingerprinting scenario summary\n"
           << "Scenario: " << scenario.at("title").as<std::string>() << " ("
           << scenario.at("id").as<std::string>() << ")\n"
           << "Run identity: " << run.at("id").as<std::string>() << "\n"
           << "Target: " << target.at("fingerprint_id").as<std::string>() << " in "
           << target.at("tissue").as<std::string>() << "/"
           << target.at("region_id").as<std::string>() << "\n"
           << "Molecular target detected: " << (detection.at("detected").as<bool>() ? "yes" : "no")
           << "\n"
           << "Fingerprint assembled: " << (assembly.at("complete").as<bool>() ? "yes" : "no")
           << " (" << assembly.at("releases").size() << "/"
           << assembly.at("required_unique_tiles").as<std::uint64_t>() << " tiles)\n"
           << "Local messages delivered: "
           << communication.at("delivered_local_messages").as<std::uint64_t>() << "/"
           << communication.at("attempted_local_messages").as<std::uint64_t>() << "\n"
           << "Body-area-network frames delivered: "
           << communication.at("delivered_ban_frames").as<std::uint64_t>() << "/"
           << communication.at("attempted_ban_frames").as<std::uint64_t>() << "\n"
           << "Sensitivity in the four-case software analysis: "
           << percentage(analysis.at("sensitivity")) << "\n"
           << "Specificity in the four-case software analysis: "
           << percentage(analysis.at("specificity")) << "\n"
           << "Clinical validation claim: "
           << (validity.at("clinical_validation_claim").as<bool>() ? "yes" : "no") << "\n"
           << "Documented limitations: " << validity.at("limitations").size() << "\n";
    return output.str();
}

void write_text(const std::filesystem::path& path, const std::string_view content) {
    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        output_error("Cannot write scenario summary: " + path.string());
    }
    stream << content;
    if (!stream) {
        output_error("Failed while writing scenario summary: " + path.string());
    }
}

[[nodiscard]] Json make_provenance(const PreparedScenario& prepared,
                                   const HolisticFingerprintingResultReport& result,
                                   const RunPaths& paths, const std::string& started_at,
                                   const std::string& completed_at) {
    const auto build = experiment::current_build_metadata();
    std::error_code relative_error;
    const auto relative_profile =
        std::filesystem::relative(prepared.profile_path, prepared.repository_root, relative_error);
    const auto profile_path =
        !relative_error && !relative_profile.empty() &&
                std::ranges::none_of(relative_profile,
                                     [](const auto& part) { return part == ".."; })
            ? relative_profile
            : prepared.profile_path;
    return Json{
        jsoncons::json_object_arg,
        {{"schema_version", "1.0.0"},
         {"scenario", Json{jsoncons::json_object_arg,
                           {{"id", prepared.profile.scenario.id},
                            {"version", prepared.profile.scenario.version},
                            {"profile_path", profile_path.generic_string()},
                            {"profile_sha256", experiment::sha256_file(prepared.profile_path)}}}},
         {"run", Json{jsoncons::json_object_arg,
                      {{"id", result.reproducibility.run.id},
                       {"directory", paths.directory.filename().generic_string()},
                       {"started_at_utc", started_at},
                       {"completed_at_utc", completed_at},
                       {"status", "completed"}}}},
         {"result", Json{jsoncons::json_object_arg,
                         {{"path", paths.result.filename().generic_string()},
                          {"sha256", experiment::sha256_file(paths.result)}}}},
         {"software", Json{jsoncons::json_object_arg,
                           {{"name", "MEHLISSA"},
                            {"version", build.software_version},
                            {"git_commit", build.git_commit},
                            {"git_dirty", build.git_dirty},
                            {"build_type", build.build_type},
                            {"compiler_id", build.compiler_id},
                            {"compiler_version", build.compiler_version},
                            {"operating_system", build.operating_system},
                            {"architecture", build.architecture}}}}}};
}

[[nodiscard]] std::filesystem::path absolute_output_root(const UsabilityCommand& command) {
    std::error_code error;
    const auto output = std::filesystem::absolute(command.output, error);
    if (error) {
        output_error("Cannot resolve scenario output root: " + command.output.string());
    }
    return output;
}

void validate_created_artifacts(const RunPaths& paths, const RunSchemas& schemas) {
    static_cast<void>(read_validated_json(paths.result, schemas.result, "scenario result"));
    static_cast<void>(
        read_validated_json(paths.provenance, schemas.provenance, "scenario provenance"));
    experiment::validate_run_log({paths.log, schemas.log});
    std::error_code error;
    if (!std::filesystem::is_regular_file(paths.summary, error) || error) {
        output_error("Scenario summary was not created: " + paths.summary.string());
    }
}

void print_run_paths(const RunPaths& paths) {
    std::printf("scenario_status=completed\n");
    std::printf("run_directory=%s\n", paths.directory.string().c_str());
    std::printf("result_file=%s\n", paths.result.string().c_str());
    std::printf("provenance_file=%s\n", paths.provenance.string().c_str());
    std::printf("run_log_file=%s\n", paths.log.string().c_str());
    std::printf("summary_file=%s\n", paths.summary.string().c_str());
}

int list_scenarios(const UsabilityCommand& command) {
    const auto root = locate_repository_root(command);
    const auto schema = root / scenario_schema_relative;
    const auto directory = root / "examples" / "scenarios";
    std::vector<std::filesystem::path> candidates;
    for (const auto& entry : std::filesystem::directory_iterator{directory}) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            const auto document = read_json(entry.path(), "scenario example candidate");
            if (document.contains("scenario") && document.contains("artifacts") &&
                document.contains("acceptance")) {
                candidates.push_back(entry.path());
            }
        }
    }
    std::ranges::sort(candidates);
    if (candidates.empty()) {
        input_error("No runnable scenario profiles were found below " + directory.string());
    }
    std::printf("Available MEHLISSA scenarios (%zu):\n", candidates.size());
    for (const auto& path : candidates) {
        const auto profile = scenarios::fingerprinting::load_scenario_profile({path, schema});
        static_cast<void>(scenarios::fingerprinting::compose_level_a_plan(profile, root));
        std::printf("  %s\n", profile.scenario.id.c_str());
        std::printf("    title: %s\n", profile.scenario.title.c_str());
        std::printf("    file: %s\n",
                    std::filesystem::relative(path, root).generic_string().c_str());
        std::printf("    scope: historical Level A baseline plus executable Levels B-E\n");
    }
    return 0;
}

int validate_scenario(const UsabilityCommand& command) {
    const auto prepared = prepare_scenario(command);
    std::printf("Scenario is valid: %s (%zu artifacts)\n", prepared.profile.scenario.id.c_str(),
                prepared.plan.artifacts.size());
    std::printf("scenario_file=%s\n", prepared.profile_path.string().c_str());
    std::printf("repository_root=%s\n", prepared.repository_root.string().c_str());
    return 0;
}

[[nodiscard]] RunPaths run_scenario_impl(const UsabilityCommand& command, const bool emit_output) {
    const auto prepared = prepare_scenario(command);
    const RunSchemas schemas{
        option_or_default(command.result_schema, prepared.repository_root, result_schema_relative),
        option_or_default(command.provenance_schema, prepared.repository_root,
                          provenance_schema_relative),
        option_or_default(command.log_schema, prepared.repository_root, log_schema_relative)};
    const auto started_at = experiment::current_utc_timestamp();
    const auto paths =
        create_run_paths(absolute_output_root(command), prepared.profile, started_at);
    experiment::JsonLinesRunLog log{paths.log};
    log.write({started_at,
               {},
               experiment::LogLevel::info,
               "scenario-runner",
               "run_started",
               "Fingerprinting scenario validation completed; execution started",
               std::nullopt});
    try {
        const auto result = scenarios::fingerprinting::run_holistic_fingerprinting_scenario(
            prepared.plan, scenarios::fingerprinting::default_level_e_cases(prepared.plan));
        scenarios::fingerprinting::write_holistic_fingerprinting_result_report(
            result, {paths.result, schemas.result});
        const auto result_document =
            read_validated_json(paths.result, schemas.result, "scenario result");
        const auto summary = summarize_result(result_document);
        write_text(paths.summary, summary);
        const auto completed_at = experiment::current_utc_timestamp();
        write_validated_json(make_provenance(prepared, result, paths, started_at, completed_at),
                             paths.provenance, "scenario provenance", schemas.provenance);
        const auto simulation_time = result.reproducibility.runtime.stages.back().time;
        log.write({completed_at, simulation_time, experiment::LogLevel::info, "scenario-runner",
                   "run_completed", "Fingerprinting scenario execution completed", std::nullopt});
        validate_created_artifacts(paths, schemas);
        if (emit_output) {
            std::fputs(summary.c_str(), stdout);
            print_run_paths(paths);
        }
    } catch (const core::MehlissaError& error) {
        log.write({experiment::current_utc_timestamp(),
                   {},
                   experiment::LogLevel::error,
                   "scenario-runner",
                   "run_failed",
                   error.what(),
                   error.code()});
        throw;
    } catch (const std::exception& error) {
        log.write({experiment::current_utc_timestamp(),
                   {},
                   experiment::LogLevel::error,
                   "scenario-runner",
                   "run_failed",
                   error.what(),
                   ErrorCode::internal_failure});
        throw;
    }
    return paths;
}

int run_scenario(const UsabilityCommand& command) {
    static_cast<void>(run_scenario_impl(command, true));
    return 0;
}

int summarize_result_file(const UsabilityCommand& command) {
    const auto root = locate_repository_root(command);
    const auto result_path = resolve_input(command.file, root);
    const auto schema = option_or_default(command.result_schema, root, result_schema_relative);
    const auto document = read_validated_json(result_path, schema, "scenario result");
    std::fputs(summarize_result(document).c_str(), stdout);
    std::printf("result_file=%s\n", result_path.string().c_str());
    return 0;
}

} // namespace

bool handles_usability_command(const int argc, const char* const argv[]) noexcept {
    if (argc < 2) {
        return false;
    }
    const std::string_view command{argv[1]};
    return command == "scenario" || command == "result";
}

int execute_usability_command(const int argc, const char* const argv[]) {
    for (int argument = 2; argument < argc; ++argument) {
        if (is_help(argv[argument])) {
            print_usability_usage();
            return 0;
        }
    }
    const auto command = parse_command(argc, argv);
    switch (command.operation) {
    case UsabilityOperation::scenario_list:
        return list_scenarios(command);
    case UsabilityOperation::scenario_validate:
        return validate_scenario(command);
    case UsabilityOperation::scenario_run:
        return run_scenario(command);
    case UsabilityOperation::result_summarize:
        return summarize_result_file(command);
    }
    throw core::MehlissaError{ErrorCode::internal_failure, "Unhandled scenario or result command"};
}

ScenarioRunOutput run_scenario_workflow(const ScenarioRunRequest& request,
                                        const bool print_output) {
    UsabilityCommand command;
    command.operation = UsabilityOperation::scenario_run;
    command.file = request.file;
    command.output = request.output;
    command.repository_root = request.repository_root;
    command.schema = request.schema;
    command.result_schema = request.result_schema;
    command.provenance_schema = request.provenance_schema;
    command.log_schema = request.log_schema;
    if (command.file.empty()) {
        invalid_command("Scenario workflow requires a profile file");
    }
    const auto paths = run_scenario_impl(command, print_output);
    return {paths.directory, paths.result, paths.provenance, paths.log, paths.summary};
}

void print_usability_usage() {
    std::fputs("  mehlissa scenario list [--repository-root <directory>]\n"
               "  mehlissa scenario validate --file <file> [--repository-root <directory>] "
               "[--schema <file>]\n"
               "  mehlissa scenario run --file <file> [--output <directory>] "
               "[--repository-root <directory>] [--schema <file>] [--result-schema <file>] "
               "[--provenance-schema <file>] [--log-schema <file>]\n"
               "  mehlissa result summarize --file <file> [--repository-root <directory>] "
               "[--result-schema <file>]\n",
               stderr);
}

} // namespace mehlissa::apps

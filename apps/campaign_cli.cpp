// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "campaign_cli.hpp"

#include "scenario_cli.hpp"

#include <mehlissa/core/error.hpp>
#include <mehlissa/experiment/provenance.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace mehlissa::apps {
namespace {

using Json = jsoncons::json;
using core::ErrorCode;

constexpr auto campaign_schema_relative = "data/schemas/campaign/1.0.0.schema.json";
constexpr auto campaign_result_schema_relative = "data/schemas/campaign-result/1.0.0.schema.json";
constexpr auto scenario_schema_relative =
    "data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json";
constexpr auto scenario_result_schema_relative =
    "data/schemas/fingerprinting-result/2.0.0.schema.json";

enum class Operation : std::uint8_t { validate, run };

struct Command final {
    Operation operation{};
    std::filesystem::path file;
    std::filesystem::path output;
    std::filesystem::path repository_root;
    std::filesystem::path schema;
    std::filesystem::path campaign_result_schema;
    std::filesystem::path scenario_schema;
    std::filesystem::path scenario_result_schema;
};

struct RunSpec final {
    std::string id;
    std::string design;
    std::string group;
    std::string role;
    std::uint64_t replicate_index{};
    std::uint64_t seed{};
    std::uint64_t collector_count{};
};

[[noreturn]] void invalid_command(const std::string& message) {
    throw core::MehlissaError{ErrorCode::command_line_invalid, message};
}

[[noreturn]] void input_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::input_unreadable, message};
}

[[noreturn]] void data_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::data_invalid, message};
}

[[nodiscard]] std::uint64_t checked_seed(const std::uint64_t first, const std::uint64_t offset) {
    if (offset > std::numeric_limits<std::uint64_t>::max() - first) {
        data_error("Campaign seed sequence exceeds the unsigned 64-bit range");
    }
    return first + offset;
}

[[noreturn]] void output_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::output_unwritable, message};
}

[[nodiscard]] bool is_help(const std::string_view value) noexcept {
    return value == "--help" || value == "-h";
}

[[nodiscard]] Command parse_command(const int argc, const char* const argv[]) {
    if (argc < 3) {
        invalid_command("The campaign command requires validate or run");
    }
    Command command;
    const std::string_view action{argv[2]};
    if (action == "validate") {
        command.operation = Operation::validate;
    } else if (action == "run") {
        command.operation = Operation::run;
    } else {
        invalid_command("Unknown campaign subcommand: " + std::string{action});
    }
    for (int argument = 3; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            invalid_command("Missing value for campaign option: " + std::string{argv[argument]});
        }
        const std::string_view option{argv[argument]};
        const std::filesystem::path value{argv[argument + 1]};
        if (option == "--file") {
            command.file = value;
        } else if (option == "--output") {
            command.output = value;
        } else if (option == "--repository-root") {
            command.repository_root = value;
        } else if (option == "--schema") {
            command.schema = value;
        } else if (option == "--campaign-result-schema") {
            command.campaign_result_schema = value;
        } else if (option == "--scenario-schema") {
            command.scenario_schema = value;
        } else if (option == "--scenario-result-schema") {
            command.scenario_result_schema = value;
        } else {
            invalid_command("Unknown campaign option: " + std::string{option});
        }
    }
    if (command.file.empty()) {
        invalid_command("campaign validate and campaign run require --file <campaign.json>");
    }
    if (command.operation == Operation::run && command.output.empty()) {
        invalid_command("campaign run requires --output <new-directory>");
    }
    if (command.operation == Operation::validate &&
        (!command.output.empty() || !command.campaign_result_schema.empty() ||
         !command.scenario_result_schema.empty())) {
        invalid_command("campaign validate does not accept output or result-schema options");
    }
    return command;
}

[[nodiscard]] bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path / "CMakeLists.txt", error) && !error &&
           std::filesystem::is_regular_file(path / campaign_schema_relative, error) && !error;
}

[[nodiscard]] std::filesystem::path locate_repository_root(const Command& command) {
    std::error_code error;
    auto current =
        command.repository_root.empty() ? std::filesystem::current_path() : command.repository_root;
    current = std::filesystem::absolute(current, error);
    if (error) {
        input_error("Cannot resolve the repository root");
    }
    if (!std::filesystem::is_directory(current, error)) {
        current = current.parent_path();
    }
    while (!current.empty()) {
        if (looks_like_repository_root(current)) {
            return std::filesystem::weakly_canonical(current, error);
        }
        const auto parent = current.parent_path();
        if (parent == current) {
            break;
        }
        current = parent;
    }
    input_error("Cannot locate the MEHLISSA repository; pass --repository-root <directory>");
}

[[nodiscard]] std::filesystem::path resolve_input(const std::filesystem::path& value,
                                                  const std::filesystem::path& root) {
    if (value.is_absolute()) {
        return value;
    }
    std::error_code error;
    const auto local = std::filesystem::absolute(value, error);
    if (!error && std::filesystem::is_regular_file(local, error) && !error) {
        return local;
    }
    return root / value;
}

[[nodiscard]] std::filesystem::path option_or_default(const std::filesystem::path& option,
                                                      const std::filesystem::path& root,
                                                      const std::filesystem::path& fallback) {
    return option.empty() ? root / fallback : resolve_input(option, root);
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

void validate_document(const Json& document, const std::filesystem::path& schema_path,
                       const std::string_view role) {
    const auto schema_document = read_json(schema_path, "schema for " + std::string{role});
    try {
        jsoncons::jsonschema::make_json_schema(schema_document).validate(document);
    } catch (const std::exception& error) {
        data_error(std::string{role} + " does not satisfy its schema: " + error.what());
    }
}

[[nodiscard]] Json read_validated_json(const std::filesystem::path& document_path,
                                       const std::filesystem::path& schema_path,
                                       const std::string_view role) {
    auto document = read_json(document_path, role);
    validate_document(document, schema_path, role);
    return document;
}

void write_json(const Json& document, const std::filesystem::path& path,
                const std::filesystem::path& schema, const std::string_view role) {
    validate_document(document, schema, role);
    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        output_error("Cannot write " + std::string{role} + ": " + path.string());
    }
    document.dump_pretty(stream);
    stream << '\n';
    if (!stream) {
        output_error("Failed while writing " + std::string{role} + ": " + path.string());
    }
}

[[nodiscard]] std::vector<RunSpec> make_specs(const Json& campaign,
                                              const std::uint64_t base_collectors) {
    std::vector<RunSpec> specs;
    const auto& design = campaign.at("design");
    const auto& replicate_plan = design.at("replicates");
    const auto replicate_count = replicate_plan.at("count").as<std::uint64_t>();
    const auto first_replicate_seed = replicate_plan.at("first_seed").as<std::uint64_t>();
    for (std::uint64_t index = 0; index < replicate_count; ++index) {
        specs.push_back({"replicate-r" + std::to_string(index + 1), "replicate", "replicates",
                         "member", index + 1, checked_seed(first_replicate_seed, index),
                         base_collectors});
    }
    for (const auto& sweep : design.at("sweeps").array_range()) {
        const auto parameter = sweep.at("parameter").as<std::string>();
        if (parameter != "run.collector_count") {
            data_error("Unsafe or unsupported sweep parameter '" + parameter +
                       "'; UX-4 permits only run.collector_count");
        }
        const auto group = sweep.at("id").as<std::string>();
        const auto repetitions = sweep.at("replicates").as<std::uint64_t>();
        const auto first_seed = sweep.at("first_seed").as<std::uint64_t>();
        std::uint64_t value_index = 0;
        for (const auto& value : sweep.at("values").array_range()) {
            const auto collectors = value.as<std::uint64_t>();
            for (std::uint64_t replicate = 0; replicate < repetitions; ++replicate) {
                specs.push_back({group + "-v" + std::to_string(value_index + 1) + "-r" +
                                     std::to_string(replicate + 1),
                                 "sweep", group, "member", replicate + 1,
                                 checked_seed(first_seed, value_index * repetitions + replicate),
                                 collectors});
            }
            ++value_index;
        }
    }
    for (const auto& comparison : design.at("paired_comparisons").array_range()) {
        const auto parameter = comparison.at("parameter").as<std::string>();
        if (parameter != "run.collector_count") {
            data_error("Unsafe or unsupported comparison parameter '" + parameter +
                       "'; UX-4 permits only run.collector_count");
        }
        const auto group = comparison.at("id").as<std::string>();
        const auto repetitions = comparison.at("replicates").as<std::uint64_t>();
        const auto first_seed = comparison.at("first_seed").as<std::uint64_t>();
        const auto baseline = comparison.at("baseline").as<std::uint64_t>();
        const auto alternative = comparison.at("comparison").as<std::uint64_t>();
        for (std::uint64_t replicate = 0; replicate < repetitions; ++replicate) {
            const auto seed = checked_seed(first_seed, replicate);
            specs.push_back({group + "-baseline-r" + std::to_string(replicate + 1),
                             "paired_comparison", group, "baseline", replicate + 1, seed,
                             baseline});
            specs.push_back({group + "-comparison-r" + std::to_string(replicate + 1),
                             "paired_comparison", group, "comparison", replicate + 1, seed,
                             alternative});
        }
    }
    if (specs.empty()) {
        data_error("Campaign design does not contain any runs");
    }
    return specs;
}

void ensure_unique_ids(const std::vector<RunSpec>& specs) {
    for (auto left = specs.begin(); left != specs.end(); ++left) {
        for (auto right = left + 1; right != specs.end(); ++right) {
            if (left->id == right->id) {
                data_error("Campaign creates duplicate derived run id: " + left->id);
            }
        }
    }
}

[[nodiscard]] std::filesystem::path
create_output_directory(const std::filesystem::path& requested) {
    std::error_code error;
    const auto output = std::filesystem::absolute(requested, error);
    if (error) {
        output_error("Cannot resolve campaign output directory: " + requested.string());
    }
    if (std::filesystem::exists(output, error)) {
        output_error("Campaign output directory already exists; refusing to overwrite: " +
                     output.string());
    }
    if (!std::filesystem::create_directories(output / "manifests", error) || error) {
        output_error("Cannot create campaign output directory: " + output.string());
    }
    std::filesystem::create_directory(output / "runs", error);
    if (error) {
        output_error("Cannot create campaign run directory: " + error.message());
    }
    return output;
}

[[nodiscard]] std::string relative_path(const std::filesystem::path& path,
                                        const std::filesystem::path& root) {
    std::error_code error;
    const auto relative = std::filesystem::relative(path, root, error);
    return error ? path.generic_string() : relative.generic_string();
}

[[nodiscard]] std::optional<double> interval_estimate(const Json& result,
                                                      const std::string_view metric) {
    const auto& interval = result.at("level_e_analysis").at("summary").at(metric);
    if (interval.is_null()) {
        return std::nullopt;
    }
    return interval.at("estimate").as<double>();
}

[[nodiscard]] Json nullable_number(const std::optional<double> value) {
    return value.has_value() ? Json{*value} : Json::null();
}

[[nodiscard]] Json make_run_record(const RunSpec& spec, const std::filesystem::path& manifest,
                                   const ScenarioRunOutput& output,
                                   const std::filesystem::path& campaign_root) {
    const auto result = read_json(output.result, "derived scenario result");
    return Json{jsoncons::json_object_arg,
                {{"id", spec.id},
                 {"design", spec.design},
                 {"group", spec.group},
                 {"role", spec.role},
                 {"replicate_index", spec.replicate_index},
                 {"seed", spec.seed},
                 {"parameter", "run.collector_count"},
                 {"value", spec.collector_count},
                 {"manifest_path", relative_path(manifest, campaign_root)},
                 {"manifest_sha256", experiment::sha256_file(manifest)},
                 {"result_path", relative_path(output.result, campaign_root)},
                 {"result_sha256", experiment::sha256_file(output.result)},
                 {"detected", result.at("level_b_detection").at("detected")},
                 {"assembled", result.at("level_c_assembly").at("complete")},
                 {"sensitivity", nullable_number(interval_estimate(result, "sensitivity"))},
                 {"specificity", nullable_number(interval_estimate(result, "specificity"))}}};
}

void write_csv(const Json& runs, const std::filesystem::path& path) {
    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        output_error("Cannot write campaign CSV: " + path.string());
    }
    stream << "id,design,group,role,replicate_index,seed,parameter,value,detected,assembled,"
              "sensitivity,specificity,result_path\n";
    for (const auto& run : runs.array_range()) {
        stream << run.at("id").as<std::string>() << ',' << run.at("design").as<std::string>() << ','
               << run.at("group").as<std::string>() << ',' << run.at("role").as<std::string>()
               << ',' << run.at("replicate_index").as<std::uint64_t>() << ','
               << run.at("seed").as<std::uint64_t>() << ',' << run.at("parameter").as<std::string>()
               << ',' << run.at("value").as<std::uint64_t>() << ','
               << (run.at("detected").as<bool>() ? "true" : "false") << ','
               << (run.at("assembled").as<bool>() ? "true" : "false") << ',';
        for (const auto metric : {"sensitivity", "specificity"}) {
            if (!run.at(metric).is_null()) {
                stream << std::setprecision(17) << run.at(metric).as<double>();
            }
            stream << ',';
        }
        stream << run.at("result_path").as<std::string>() << '\n';
    }
    if (!stream) {
        output_error("Failed while writing campaign CSV: " + path.string());
    }
}

struct PreparedCampaign final {
    std::filesystem::path root;
    std::filesystem::path campaign_path;
    std::filesystem::path campaign_schema;
    std::filesystem::path scenario_path;
    std::filesystem::path scenario_schema;
    Json campaign;
    Json base_scenario;
    std::vector<RunSpec> specs;
};

[[nodiscard]] PreparedCampaign prepare(const Command& command) {
    const auto root = locate_repository_root(command);
    const auto campaign_path = resolve_input(command.file, root);
    const auto campaign_schema = option_or_default(command.schema, root, campaign_schema_relative);
    auto campaign = read_validated_json(campaign_path, campaign_schema, "campaign manifest");
    const auto scenario_path = resolve_input(campaign.at("base_scenario").as<std::string>(), root);
    const auto scenario_schema =
        option_or_default(command.scenario_schema, root, scenario_schema_relative);
    auto base_scenario =
        read_validated_json(scenario_path, scenario_schema, "base scenario profile");
    auto specs =
        make_specs(campaign, base_scenario.at("run").at("collector_count").as<std::uint64_t>());
    ensure_unique_ids(specs);
    return {root,
            campaign_path,
            campaign_schema,
            scenario_path,
            scenario_schema,
            std::move(campaign),
            std::move(base_scenario),
            std::move(specs)};
}

int validate_campaign(const Command& command) {
    const auto prepared = prepare(command);
    std::printf("Campaign is valid: %s (%zu derived runs)\n",
                prepared.campaign.at("campaign").at("id").as<std::string>().c_str(),
                prepared.specs.size());
    std::printf("campaign_file=%s\n", prepared.campaign_path.string().c_str());
    std::printf("base_scenario=%s\n", prepared.scenario_path.string().c_str());
    return 0;
}

int run_campaign(const Command& command) {
    const auto prepared = prepare(command);
    const auto output = create_output_directory(command.output);
    Json run_records = Json::array();
    for (const auto& spec : prepared.specs) {
        auto derived = prepared.base_scenario;
        derived.at("run")["id"] = spec.id;
        derived.at("run")["master_seed"] = spec.seed;
        derived.at("run")["collector_count"] = spec.collector_count;
        const auto manifest = output / "manifests" / (spec.id + ".json");
        write_json(derived, manifest, prepared.scenario_schema, "derived scenario manifest");
        const auto scenario_output =
            run_scenario_workflow({manifest,
                                   output / "runs",
                                   prepared.root,
                                   prepared.scenario_schema,
                                   option_or_default(command.scenario_result_schema, prepared.root,
                                                     scenario_result_schema_relative),
                                   {},
                                   {}},
                                  false);
        run_records.push_back(make_run_record(spec, manifest, scenario_output, output));
    }
    Json hooks = Json::array();
    hooks.push_back(
        Json{jsoncons::json_object_arg,
             {{"parameter", "run.collector_count"},
              {"response_metrics", Json{jsoncons::json_array_arg,
                                        {"detected", "assembled", "sensitivity", "specificity"}}},
              {"qualification", "Descriptive campaign output; no global-sensitivity or clinical "
                                "inference claim"}}});
    const auto& identity = prepared.campaign.at("campaign");
    Json aggregate{
        jsoncons::json_object_arg,
        {{"schema_version", "1.0.0"},
         {"campaign", Json{jsoncons::json_object_arg,
                           {{"id", identity.at("id")},
                            {"title", identity.at("title")},
                            {"source_manifest", prepared.campaign_path.generic_string()},
                            {"source_sha256", experiment::sha256_file(prepared.campaign_path)},
                            {"base_scenario", prepared.scenario_path.generic_string()}}}},
         {"run_count", run_records.size()},
         {"runs", run_records},
         {"sensitivity_hooks", std::move(hooks)},
         {"limitations", prepared.campaign.at("limitations")}}};
    const auto aggregate_path = output / "campaign-result.json";
    const auto aggregate_schema = option_or_default(command.campaign_result_schema, prepared.root,
                                                    campaign_result_schema_relative);
    write_json(aggregate, aggregate_path, aggregate_schema, "campaign result");
    write_csv(run_records, output / "campaign-results.csv");
    std::printf("campaign_status=completed\n");
    std::printf("campaign_directory=%s\n", output.string().c_str());
    std::printf("campaign_result=%s\n", aggregate_path.string().c_str());
    std::printf("campaign_csv=%s\n", (output / "campaign-results.csv").string().c_str());
    std::printf("derived_runs=%zu\n", prepared.specs.size());
    return 0;
}

} // namespace

bool handles_campaign_command(const int argc, const char* const argv[]) noexcept {
    return argc >= 2 && std::string_view{argv[1]} == "campaign";
}

int execute_campaign_command(const int argc, const char* const argv[]) {
    for (int argument = 2; argument < argc; ++argument) {
        if (is_help(argv[argument])) {
            print_campaign_usage();
            return 0;
        }
    }
    const auto command = parse_command(argc, argv);
    return command.operation == Operation::validate ? validate_campaign(command)
                                                    : run_campaign(command);
}

void print_campaign_usage() {
    std::fputs(
        "  mehlissa campaign validate --file <campaign.json> [--repository-root <directory>] "
        "[--schema <file>] [--scenario-schema <file>]\n"
        "  mehlissa campaign run --file <campaign.json> --output <new-directory> "
        "[--repository-root <directory>] [--schema <file>] "
        "[--campaign-result-schema <file>] [--scenario-schema <file>] "
        "[--scenario-result-schema <file>]\n",
        stderr);
}

} // namespace mehlissa::apps

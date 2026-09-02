// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "discovery_cli.hpp"

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>

namespace mehlissa::apps {
namespace {

using Json = jsoncons::json;
using core::ErrorCode;

constexpr auto default_catalog_relative = "data/catalog/model-catalog-v1.json";
constexpr auto catalog_schema_relative = "data/schemas/model-catalog/1.0.0.schema.json";

enum class DiscoveryOperation : std::uint8_t {
    model_list,
    model_describe,
    example_list,
    example_copy
};

struct DiscoveryCommand final {
    DiscoveryOperation operation{};
    std::string id;
    std::string layer;
    std::string model_id;
    std::filesystem::path output;
    std::filesystem::path repository_root;
    std::filesystem::path catalog;
};

struct CatalogContext final {
    std::filesystem::path repository_root;
    std::filesystem::path catalog_path;
    std::filesystem::path schema_path;
    Json document;
};

[[noreturn]] void invalid_command(const std::string& message) {
    throw core::MehlissaError{ErrorCode::command_line_invalid, message};
}

[[noreturn]] void input_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::input_unreadable, message};
}

[[noreturn]] void catalog_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::data_invalid, message};
}

[[noreturn]] void output_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::output_unwritable, message};
}

[[nodiscard]] bool is_help(const std::string_view value) noexcept {
    return value == "--help" || value == "-h";
}

[[nodiscard]] DiscoveryOperation parse_operation(const int argc, const char* const argv[]) {
    if (argc < 3) {
        invalid_command("The model or example command requires a subcommand");
    }
    const std::string_view family{argv[1]};
    const std::string_view action{argv[2]};
    if (family == "model" && action == "list") {
        return DiscoveryOperation::model_list;
    }
    if (family == "model" && action == "describe") {
        return DiscoveryOperation::model_describe;
    }
    if (family == "example" && action == "list") {
        return DiscoveryOperation::example_list;
    }
    if (family == "example" && action == "copy") {
        return DiscoveryOperation::example_copy;
    }
    invalid_command("Unknown model or example subcommand: " + std::string{action});
}

struct NamedOption final {
    std::string_view name;
    std::string_view value;
};

void assign_option(DiscoveryCommand& command, const NamedOption option) {
    if (option.name == "--id") {
        command.id = option.value;
    } else if (option.name == "--layer") {
        command.layer = option.value;
    } else if (option.name == "--model") {
        command.model_id = option.value;
    } else if (option.name == "--output") {
        command.output = option.value;
    } else if (option.name == "--repository-root") {
        command.repository_root = option.value;
    } else if (option.name == "--catalog") {
        command.catalog = option.value;
    } else {
        invalid_command("Unknown discovery option: " + std::string{option.name});
    }
}

void reject_disallowed_options(const DiscoveryCommand& command) {
    switch (command.operation) {
    case DiscoveryOperation::model_list:
        if (!command.id.empty() || !command.model_id.empty() || !command.output.empty()) {
            invalid_command("model list accepts only --layer, --catalog, and --repository-root");
        }
        break;
    case DiscoveryOperation::model_describe:
        if (command.id.empty()) {
            invalid_command("model describe requires --id <model-id>");
        }
        if (!command.layer.empty() || !command.model_id.empty() || !command.output.empty()) {
            invalid_command("model describe accepts only --id, --catalog, and --repository-root");
        }
        break;
    case DiscoveryOperation::example_list:
        if (!command.id.empty() || !command.layer.empty() || !command.output.empty()) {
            invalid_command("example list accepts only --model, --catalog, and --repository-root");
        }
        break;
    case DiscoveryOperation::example_copy:
        if (command.id.empty() || command.output.empty()) {
            invalid_command("example copy requires --id <example-id> and --output <directory>");
        }
        if (!command.layer.empty() || !command.model_id.empty()) {
            invalid_command(
                "example copy accepts only --id, --output, --catalog, and --repository-root");
        }
        break;
    }
}

[[nodiscard]] DiscoveryCommand parse_command(const int argc, const char* const argv[]) {
    DiscoveryCommand command;
    command.operation = parse_operation(argc, argv);
    for (int argument = 3; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            invalid_command("Missing value for option: " + std::string{argv[argument]});
        }
        assign_option(command, {argv[argument], argv[argument + 1]});
    }
    reject_disallowed_options(command);
    return command;
}

[[nodiscard]] bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path / "CMakeLists.txt", error) && !error &&
           std::filesystem::is_regular_file(path / catalog_schema_relative, error) && !error;
}

[[nodiscard]] std::filesystem::path search_repository_ancestors(std::filesystem::path path) {
    std::error_code error;
    path = std::filesystem::absolute(path, error);
    if (error) {
        input_error("Cannot resolve the current path while locating the repository");
    }
    if (!std::filesystem::is_directory(path, error)) {
        path = path.parent_path();
    }
    while (!path.empty()) {
        if (looks_like_repository_root(path)) {
            auto root = std::filesystem::weakly_canonical(path, error);
            if (error) {
                input_error("Cannot canonicalize the MEHLISSA repository root");
            }
            return root;
        }
        const auto parent = path.parent_path();
        if (parent == path) {
            break;
        }
        path = parent;
    }
    input_error("Cannot locate the MEHLISSA repository; run from the source tree or pass "
                "--repository-root <directory>");
}

[[nodiscard]] std::filesystem::path locate_repository_root(const DiscoveryCommand& command) {
    if (command.repository_root.empty()) {
        return search_repository_ancestors(std::filesystem::current_path());
    }
    std::error_code error;
    auto root = std::filesystem::weakly_canonical(command.repository_root, error);
    if (error || !looks_like_repository_root(root)) {
        input_error("--repository-root is not a MEHLISSA source tree: " +
                    command.repository_root.string());
    }
    return root;
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        input_error("Cannot read " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw core::MehlissaError{ErrorCode::json_invalid,
                                  std::string{role} + " is not valid JSON: " + error.what()};
    }
}

[[nodiscard]] std::filesystem::path catalog_asset(const CatalogContext& context,
                                                  const std::string_view relative_value) {
    const std::filesystem::path relative{relative_value};
    const auto normalized = relative.lexically_normal();
    if (normalized.empty() || normalized.is_absolute() || *normalized.begin() == "..") {
        catalog_error("Catalog path must stay inside the repository: " + relative.string());
    }
    const auto resolved = context.repository_root / normalized;
    std::error_code error;
    if (!std::filesystem::is_regular_file(resolved, error) || error) {
        catalog_error("Catalog references a missing file: " + normalized.generic_string());
    }
    const auto canonical = std::filesystem::weakly_canonical(resolved, error);
    if (error) {
        catalog_error("Cannot canonicalize catalog path: " + normalized.generic_string());
    }
    const auto relative_to_root = canonical.lexically_relative(context.repository_root);
    if (relative_to_root.empty() || relative_to_root.is_absolute() ||
        *relative_to_root.begin() == "..") {
        catalog_error("Catalog path must stay inside the repository: " + relative.string());
    }
    return canonical;
}

void validate_catalog_semantics(const CatalogContext& context) {
    std::unordered_set<std::string> model_ids;
    for (const auto& model : context.document.at("models").array_range()) {
        const auto id = model.at("id").as<std::string>();
        if (!model_ids.insert(id).second) {
            catalog_error("Duplicate model id in catalog: " + id);
        }
        for (const auto& path : model.at("artifacts").array_range()) {
            static_cast<void>(catalog_asset(context, path.as<std::string>()));
        }
        for (const auto& path : model.at("documentation").array_range()) {
            static_cast<void>(catalog_asset(context, path.as<std::string>()));
        }
        for (const auto& parameter : model.at("parameters").array_range()) {
            static_cast<void>(
                catalog_asset(context, parameter.at("configured_in").as<std::string>()));
        }
    }

    std::unordered_set<std::string> example_ids;
    for (const auto& example : context.document.at("examples").array_range()) {
        const auto id = example.at("id").as<std::string>();
        if (!example_ids.insert(id).second) {
            catalog_error("Duplicate example id in catalog: " + id);
        }
        const auto path = example.at("path").as<std::string>();
        static_cast<void>(catalog_asset(context, path));
        static_cast<void>(catalog_asset(context, path + ".license"));
        for (const auto& model_id : example.at("model_ids").array_range()) {
            const auto referenced_id = model_id.as<std::string>();
            if (!model_ids.contains(referenced_id)) {
                catalog_error("Example " + id + " references unknown model: " + referenced_id);
            }
        }
    }
}

[[nodiscard]] CatalogContext load_catalog(const DiscoveryCommand& command) {
    CatalogContext context;
    context.repository_root = locate_repository_root(command);
    context.catalog_path =
        command.catalog.empty()
            ? context.repository_root / default_catalog_relative
            : (command.catalog.is_absolute() ? command.catalog
                                             : context.repository_root / command.catalog);
    context.schema_path = context.repository_root / catalog_schema_relative;
    context.document = read_json(context.catalog_path, "model catalog");
    const auto schema_document = read_json(context.schema_path, "model catalog schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        schema.validate(context.document);
    } catch (const std::exception& error) {
        throw core::MehlissaError{ErrorCode::data_invalid,
                                  std::string{"Model catalog does not satisfy its schema: "} +
                                      error.what()};
    }
    validate_catalog_semantics(context);
    return context;
}

[[nodiscard]] const Json& find_model(const CatalogContext& context,
                                     const std::string_view requested_id) {
    for (const auto& model : context.document.at("models").array_range()) {
        if (model.at("id").as<std::string>() == requested_id) {
            return model;
        }
    }
    catalog_error("Unknown model id: " + std::string{requested_id} +
                  ". Run 'mehlissa model list' to see available ids.");
}

[[nodiscard]] const Json& find_example(const CatalogContext& context,
                                       const std::string_view requested_id) {
    for (const auto& example : context.document.at("examples").array_range()) {
        if (example.at("id").as<std::string>() == requested_id) {
            return example;
        }
    }
    catalog_error("Unknown example id: " + std::string{requested_id} +
                  ". Run 'mehlissa example list' to see available ids.");
}

[[nodiscard]] bool example_uses_model(const Json& example, const std::string_view model_id) {
    for (const auto& candidate : example.at("model_ids").array_range()) {
        if (candidate.as<std::string>() == model_id) {
            return true;
        }
    }
    return false;
}

int list_models(const DiscoveryCommand& command, const CatalogContext& context) {
    std::size_t count{};
    for (const auto& model : context.document.at("models").array_range()) {
        const auto layer = model.at("layer").as<std::string>();
        if (!command.layer.empty() && command.layer != layer) {
            continue;
        }
        std::printf("%s | %s | %s | %s\n", model.at("id").as<std::string>().c_str(), layer.c_str(),
                    model.at("maturity").as<std::string>().c_str(),
                    model.at("title").as<std::string>().c_str());
        ++count;
    }
    if (count == 0U && !command.layer.empty()) {
        catalog_error("No models found for layer: " + command.layer);
    }
    std::printf("model_count=%zu\n", count);
    return 0;
}

int describe_model(const DiscoveryCommand& command, const CatalogContext& context) {
    const auto& model = find_model(context, command.id);
    std::printf("Model: %s\n", model.at("title").as<std::string>().c_str());
    std::printf("ID: %s\n", model.at("id").as<std::string>().c_str());
    std::printf("Layer: %s\n", model.at("layer").as<std::string>().c_str());
    std::printf("Maturity: %s\n", model.at("maturity").as<std::string>().c_str());
    std::printf("Summary: %s\n", model.at("summary").as<std::string>().c_str());
    std::printf("Validity scope: %s\n", model.at("validity_scope").as<std::string>().c_str());
    std::printf("Evidence: %s\n", model.at("evidence_summary").as<std::string>().c_str());

    std::fputs("Artifacts:\n", stdout);
    for (const auto& path : model.at("artifacts").array_range()) {
        std::printf("  - %s\n", path.as<std::string>().c_str());
    }
    std::fputs("Documentation:\n", stdout);
    for (const auto& path : model.at("documentation").array_range()) {
        std::printf("  - %s\n", path.as<std::string>().c_str());
    }
    std::fputs("Configurable parameters:\n", stdout);
    for (const auto& parameter : model.at("parameters").array_range()) {
        std::printf("  - %s [%s]: %s\n    configured in: %s\n",
                    parameter.at("path").as<std::string>().c_str(),
                    parameter.at("unit").as<std::string>().c_str(),
                    parameter.at("meaning").as<std::string>().c_str(),
                    parameter.at("configured_in").as<std::string>().c_str());
    }
    std::fputs("Limitations:\n", stdout);
    for (const auto& limitation : model.at("limitations").array_range()) {
        std::printf("  - %s\n", limitation.as<std::string>().c_str());
    }
    std::fputs("Starter examples:\n", stdout);
    std::size_t example_count{};
    for (const auto& example : context.document.at("examples").array_range()) {
        if (example_uses_model(example, command.id)) {
            std::printf("  - %s: %s\n", example.at("id").as<std::string>().c_str(),
                        example.at("title").as<std::string>().c_str());
            ++example_count;
        }
    }
    std::printf("starter_example_count=%zu\n", example_count);
    return 0;
}

int list_examples(const DiscoveryCommand& command, const CatalogContext& context) {
    if (!command.model_id.empty()) {
        static_cast<void>(find_model(context, command.model_id));
    }
    std::size_t count{};
    for (const auto& example : context.document.at("examples").array_range()) {
        if (!command.model_id.empty() && !example_uses_model(example, command.model_id)) {
            continue;
        }
        std::printf("%s | %s | %s\n", example.at("id").as<std::string>().c_str(),
                    example.at("path").as<std::string>().c_str(),
                    example.at("title").as<std::string>().c_str());
        ++count;
    }
    std::printf("example_count=%zu\n", count);
    return 0;
}

int copy_example(const DiscoveryCommand& command, const CatalogContext& context) {
    const auto& example = find_example(context, command.id);
    const auto relative_source = example.at("path").as<std::string>();
    const auto source = catalog_asset(context, relative_source);

    std::error_code error;
    const auto output_directory = std::filesystem::absolute(command.output, error);
    if (error) {
        output_error("Cannot resolve example output directory: " + command.output.string());
    }
    std::filesystem::create_directories(output_directory, error);
    if (error) {
        output_error("Cannot create example output directory: " + output_directory.string());
    }
    const auto destination = output_directory / source.filename();
    if (std::filesystem::exists(destination, error) || error) {
        output_error("Example destination already exists: " + destination.string());
    }
    if (!std::filesystem::copy_file(source, destination, error) || error) {
        output_error("Cannot copy starter example to: " + destination.string());
    }

    const auto license_source = catalog_asset(context, relative_source + ".license");
    const std::filesystem::path license_destination{destination.string() + ".license"};
    if (std::filesystem::exists(license_destination, error) || error) {
        output_error("Example license destination already exists: " + license_destination.string());
    }
    if (!std::filesystem::copy_file(license_source, license_destination, error) || error) {
        std::error_code cleanup_error;
        static_cast<void>(std::filesystem::remove(destination, cleanup_error));
        output_error("Cannot copy starter-example license to: " + license_destination.string());
    }

    std::printf("Copied starter example: %s\n", command.id.c_str());
    std::printf("source=%s\n", relative_source.c_str());
    std::printf("output=%s\n", destination.string().c_str());
    if (example.contains("command")) {
        std::printf("suggested_command=%s\n", example.at("command").as<std::string>().c_str());
    }
    return 0;
}

} // namespace

bool handles_discovery_command(const int argc, const char* const argv[]) noexcept {
    if (argc < 2) {
        return false;
    }
    const std::string_view family{argv[1]};
    return family == "model" || family == "example";
}

void print_discovery_usage() {
    std::fputs("Model and example discovery (UX-2):\n"
               "  mehlissa model list [--layer <body|organ|capillary|cell|nano-iot>] "
               "[--repository-root <directory>] [--catalog <file>]\n"
               "  mehlissa model describe --id <model-id> [--repository-root <directory>] "
               "[--catalog <file>]\n"
               "  mehlissa example list [--model <model-id>] [--repository-root <directory>] "
               "[--catalog <file>]\n"
               "  mehlissa example copy --id <example-id> --output <directory> "
               "[--repository-root <directory>] [--catalog <file>]\n",
               stderr);
}

int execute_discovery_command(const int argc, const char* const argv[]) {
    for (int argument = 2; argument < argc; ++argument) {
        if (is_help(argv[argument])) {
            print_discovery_usage();
            return 0;
        }
    }
    const auto command = parse_command(argc, argv);
    const auto context = load_catalog(command);
    switch (command.operation) {
    case DiscoveryOperation::model_list:
        return list_models(command, context);
    case DiscoveryOperation::model_describe:
        return describe_model(command, context);
    case DiscoveryOperation::example_list:
        return list_examples(command, context);
    case DiscoveryOperation::example_copy:
        return copy_example(command, context);
    }
    throw core::MehlissaError{ErrorCode::internal_failure, "Unhandled model discovery operation"};
}

} // namespace mehlissa::apps

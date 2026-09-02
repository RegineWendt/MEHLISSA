// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "reporting_cli.hpp"

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace mehlissa::apps {
namespace {

using Json = jsoncons::json;
using core::ErrorCode;

constexpr auto result_schema_relative = "data/schemas/fingerprinting-result/2.0.0.schema.json";

struct ReportCommand final {
    std::filesystem::path file;
    std::filesystem::path output;
    std::filesystem::path repository_root;
    std::filesystem::path result_schema;
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

[[noreturn]] void output_error(const std::string& message) {
    throw core::MehlissaError{ErrorCode::output_unwritable, message};
}

[[nodiscard]] bool is_help(const std::string_view value) noexcept {
    return value == "--help" || value == "-h";
}

[[nodiscard]] ReportCommand parse_command(const int argc, const char* const argv[]) {
    ReportCommand command;
    for (int argument = 3; argument < argc; argument += 2) {
        if (argument + 1 >= argc) {
            invalid_command("Missing value for reporting option: " + std::string{argv[argument]});
        }
        const std::string_view option{argv[argument]};
        const std::filesystem::path value{argv[argument + 1]};
        if (option == "--file") {
            command.file = value;
        } else if (option == "--output") {
            command.output = value;
        } else if (option == "--repository-root") {
            command.repository_root = value;
        } else if (option == "--result-schema") {
            command.result_schema = value;
        } else {
            invalid_command("Unknown result report option: " + std::string{option});
        }
    }
    if (command.file.empty() || command.output.empty()) {
        invalid_command("result report requires --file <result.json> and --output <directory>");
    }
    return command;
}

[[nodiscard]] bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path / "CMakeLists.txt", error) && !error &&
           std::filesystem::is_regular_file(path / result_schema_relative, error) && !error;
}

[[nodiscard]] std::filesystem::path locate_repository_root(const ReportCommand& command) {
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
            auto root = std::filesystem::weakly_canonical(current, error);
            if (error) {
                input_error("Cannot canonicalize the repository root");
            }
            return root;
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

[[nodiscard]] Json load_result(const ReportCommand& command, const std::filesystem::path& root) {
    const auto result_path = resolve_input(command.file, root);
    const auto schema_path = command.result_schema.empty()
                                 ? root / result_schema_relative
                                 : resolve_input(command.result_schema, root);
    const auto document = read_json(result_path, "scenario result");
    const auto schema_document = read_json(schema_path, "scenario result schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        schema.validate(document);
    } catch (const std::exception& error) {
        data_error(std::string{"Scenario result does not satisfy its schema: "} + error.what());
    }
    return document;
}

[[nodiscard]] std::string html_escape(const std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char character : value) {
        switch (character) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&#39;";
            break;
        default:
            escaped += character;
        }
    }
    return escaped;
}

[[nodiscard]] std::string csv_field(const std::string_view value) {
    std::string escaped{"\""};
    for (const char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += '"';
    return escaped;
}

[[nodiscard]] std::string decimal(const double value, const int precision = 6) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(precision) << value;
    return output.str();
}

[[nodiscard]] std::string percent(const Json& interval) {
    return decimal(interval.at("estimate").as<double>() * 100.0, 1) + "%";
}

[[nodiscard]] std::string make_summary(const Json& document) {
    const auto& analysis = document.at("level_e_analysis").at("summary");
    std::ostringstream output;
    output << "MEHLISSA result report\n"
           << "Scenario: " << document.at("scenario").at("title").as<std::string>() << "\n"
           << "Run: " << document.at("run").at("id").as<std::string>() << "\n"
           << "Target detected: "
           << (document.at("level_b_detection").at("detected").as<bool>() ? "yes" : "no")
           << "\nFingerprint assembled: "
           << (document.at("level_c_assembly").at("complete").as<bool>() ? "yes" : "no")
           << "\nSensitivity: " << percent(analysis.at("sensitivity"))
           << "\nSpecificity: " << percent(analysis.at("specificity"))
           << "\nClinical validation claim: "
           << (document.at("validity").at("clinical_validation_claim").as<bool>() ? "yes" : "no")
           << "\nLimitations: " << document.at("validity").at("limitations").size() << "\n";
    return output.str();
}

[[nodiscard]] std::string make_overview_csv(const Json& document) {
    const auto& analysis = document.at("level_e_analysis").at("summary");
    std::ostringstream output;
    output << "metric,value,unit\n"
           << "scenario_id," << csv_field(document.at("scenario").at("id").as<std::string>())
           << ",\nrun_id," << csv_field(document.at("run").at("id").as<std::string>())
           << ",\nmaster_seed," << document.at("run").at("master_seed").as<std::uint64_t>()
           << ",count\ncollector_count,"
           << document.at("run").at("collector_count").as<std::uint64_t>() << ",count\ndetected,"
           << (document.at("level_b_detection").at("detected").as<bool>() ? "true" : "false")
           << ",boolean\nassembled,"
           << (document.at("level_c_assembly").at("complete").as<bool>() ? "true" : "false")
           << ",boolean\nsensitivity,"
           << decimal(analysis.at("sensitivity").at("estimate").as<double>())
           << ",fraction\nspecificity,"
           << decimal(analysis.at("specificity").at("estimate").as<double>())
           << ",fraction\nclinical_validation_claim,false,boolean\n";
    return output.str();
}

[[nodiscard]] std::string make_stages_csv(const Json& document) {
    std::ostringstream output;
    output << "ordinal,stage,time_ns,basis,component_id,input_identity,output_identity,"
              "qualification\n";
    for (const auto& stage : document.at("runtime").at("stages").array_range()) {
        output << stage.at("ordinal").as<std::uint64_t>() << ','
               << csv_field(stage.at("stage").as<std::string>()) << ','
               << stage.at("time_ns").as<std::uint64_t>() << ','
               << csv_field(stage.at("basis").as<std::string>()) << ','
               << csv_field(stage.at("component_id").as<std::string>()) << ','
               << csv_field(stage.at("input_identity").as<std::string>()) << ','
               << csv_field(stage.at("output_identity").as<std::string>()) << ','
               << csv_field(stage.at("qualification").as<std::string>()) << '\n';
    }
    return output.str();
}

[[nodiscard]] std::string make_cases_csv(const Json& document) {
    std::ostringstream output;
    output << "case_id,target_present,detected,classification,ligand_concentration_mol_m3,"
              "exposure_duration_ns,final_bound_fraction\n";
    for (const auto& item : document.at("level_e_analysis").at("cases").array_range()) {
        output << csv_field(item.at("case_id").as<std::string>()) << ','
               << (item.at("target_present").as<bool>() ? "true" : "false") << ','
               << (item.at("detected").as<bool>() ? "true" : "false") << ','
               << csv_field(item.at("classification").as<std::string>()) << ','
               << decimal(item.at("ligand_concentration_mol_m3").as<double>(), 12) << ','
               << item.at("exposure_duration_ns").as<std::uint64_t>() << ','
               << decimal(item.at("final_bound_fraction").as<double>(), 12) << '\n';
    }
    return output.str();
}

[[nodiscard]] std::string make_html(const Json& document) {
    const auto& scenario = document.at("scenario");
    const auto& analysis = document.at("level_e_analysis").at("summary");
    std::ostringstream output;
    output
        << "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
           "<title>MEHLISSA result report</title><style>"
           "body{font:16px/1.5 "
           "system-ui,sans-serif;color:#183247;max-width:1100px;margin:auto;padding:2rem}"
           "h1,h2{color:#123b56}.notice{background:#fff3cd;border-left:5px solid "
           "#d39e00;padding:1rem}"
           ".cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:1rem}"
           ".card{border:1px solid "
           "#b9cbd6;border-radius:8px;padding:1rem}table{border-collapse:collapse;width:100%}"
           "th,td{border:1px solid "
           "#b9cbd6;padding:.5rem;text-align:left}th{background:#123b56;color:white}"
           "code{overflow-wrap:anywhere}footer{margin-top:2rem;color:#526b7a}</style></head><body>"
        << "<h1>" << html_escape(scenario.at("title").as<std::string>()) << "</h1>"
        << "<p>Scenario <code>" << html_escape(scenario.at("id").as<std::string>())
        << "</code>, run <code>" << html_escape(document.at("run").at("id").as<std::string>())
        << "</code></p><div class=\"notice\"><strong>Research software only.</strong> "
           "This result makes no clinical validation claim and must not be used for diagnosis or "
           "treatment.</div>"
        << "<h2>Outcome</h2><div class=\"cards\"><div class=\"card\"><strong>Detection</strong><br>"
        << (document.at("level_b_detection").at("detected").as<bool>() ? "Detected"
                                                                       : "Not detected")
        << "</div><div class=\"card\"><strong>Assembly</strong><br>"
        << (document.at("level_c_assembly").at("complete").as<bool>() ? "Complete" : "Incomplete")
        << "</div><div class=\"card\"><strong>Sensitivity</strong><br>"
        << percent(analysis.at("sensitivity"))
        << "</div><div class=\"card\"><strong>Specificity</strong><br>"
        << percent(analysis.at("specificity"))
        << "</div></div>"
           "<h2>Runtime stages</h2><table><thead><tr><th>#</th><th>Stage</th><th>Time (ns)</th>"
           "<th>Basis</th><th>Qualification</th></tr></thead><tbody>";
    for (const auto& stage : document.at("runtime").at("stages").array_range()) {
        output << "<tr><td>" << stage.at("ordinal").as<std::uint64_t>() << "</td><td>"
               << html_escape(stage.at("stage").as<std::string>()) << "</td><td>"
               << stage.at("time_ns").as<std::uint64_t>() << "</td><td>"
               << html_escape(stage.at("basis").as<std::string>()) << "</td><td>"
               << html_escape(stage.at("qualification").as<std::string>()) << "</td></tr>";
    }
    output
        << "</tbody></table><h2>Evidence and reproducibility</h2><p>The complete machine-readable "
           "result is included as <a href=\"result.json\">result.json</a>. Selected "
           "artifacts:</p><ul>";
    for (const auto& artifact : document.at("reproducibility").at("artifacts").array_range()) {
        output << "<li><strong>" << html_escape(artifact.at("role").as<std::string>())
               << ":</strong> <code>"
               << html_escape(artifact.at("definition_path").as<std::string>())
               << "</code> - SHA-256 <code>"
               << html_escape(artifact.at("definition_sha256").as<std::string>()) << "</code></li>";
    }
    output << "</ul><h2>Limitations</h2><ul>";
    for (const auto& limitation : document.at("validity").at("limitations").array_range()) {
        output << "<li>" << html_escape(limitation.as<std::string>()) << "</li>";
    }
    output << "</ul><footer>Generated by MEHLISSA UX-3 from schema-valid result version "
           << html_escape(document.at("schema_version").as<std::string>())
           << ". CSV exports: <a href=\"overview.csv\">overview</a>, "
              "<a href=\"runtime-stages.csv\">runtime stages</a>, and "
              "<a href=\"analysis-cases.csv\">analysis cases</a>.</footer></body></html>\n";
    return output.str();
}

void write_text(const std::filesystem::path& path, const std::string_view content) {
    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        output_error("Cannot write report artifact: " + path.string());
    }
    stream << content;
    if (!stream) {
        output_error("Failed while writing report artifact: " + path.string());
    }
}

int create_report(const ReportCommand& command) {
    const auto root = locate_repository_root(command);
    const auto result_path = resolve_input(command.file, root);
    const auto document = load_result(command, root);
    std::error_code error;
    const auto output_directory = std::filesystem::absolute(command.output, error);
    if (error) {
        output_error("Cannot resolve report output directory: " + command.output.string());
    }
    if (std::filesystem::exists(output_directory, error) || error) {
        output_error("Report output directory already exists: " + output_directory.string());
    }
    if (!std::filesystem::create_directories(output_directory, error) || error) {
        output_error("Cannot create report output directory: " + output_directory.string());
    }
    std::filesystem::copy_file(result_path, output_directory / "result.json", error);
    if (error) {
        output_error("Cannot copy the machine-readable result into the report bundle");
    }
    const auto summary = make_summary(document);
    write_text(output_directory / "summary.txt", summary);
    write_text(output_directory / "overview.csv", make_overview_csv(document));
    write_text(output_directory / "runtime-stages.csv", make_stages_csv(document));
    write_text(output_directory / "analysis-cases.csv", make_cases_csv(document));
    write_text(output_directory / "report.html", make_html(document));
    std::fputs(summary.c_str(), stdout);
    std::printf("report_directory=%s\n", output_directory.string().c_str());
    std::printf("html_report=%s\n", (output_directory / "report.html").string().c_str());
    return 0;
}

} // namespace

bool handles_reporting_command(const int argc, const char* const argv[]) noexcept {
    return argc >= 3 && std::string_view{argv[1]} == "result" &&
           std::string_view{argv[2]} == "report";
}

int execute_reporting_command(const int argc, const char* const argv[]) {
    for (int argument = 2; argument < argc; ++argument) {
        if (is_help(argv[argument])) {
            print_reporting_usage();
            return 0;
        }
    }
    return create_report(parse_command(argc, argv));
}

void print_reporting_usage() {
    std::fputs("  mehlissa result report --file <result.json> --output <directory> "
               "[--repository-root <directory>] [--result-schema <file>]\n",
               stderr);
}

} // namespace mehlissa::apps

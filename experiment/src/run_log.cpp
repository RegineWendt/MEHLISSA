// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/run_log.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace mehlissa::experiment {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

[[nodiscard]] std::string_view level_name(const LogLevel level) noexcept {
    switch (level) {
    case LogLevel::debug:
        return "debug";
    case LogLevel::info:
        return "info";
    case LogLevel::warning:
        return "warning";
    case LogLevel::error:
        return "error";
    }
    return "error";
}

[[nodiscard]] Json make_document(const std::uint64_t sequence, const RunLogRecord& record) {
    if (record.error_code.has_value()) {
        const auto code = *record.error_code;
        return Json{jsoncons::json_object_arg,
                    {
                        {"schema_version", supported_log_record_schema_version},
                        {"sequence", sequence},
                        {"timestamp_utc", record.timestamp_utc},
                        {"simulation_time_ns", record.simulation_time.count()},
                        {"level", level_name(record.level)},
                        {"source", record.source},
                        {"event", record.event},
                        {"message", record.message},
                        {"error", Json{jsoncons::json_object_arg,
                                       {
                                           {"code", static_cast<std::uint16_t>(code)},
                                           {"id", core::error_code_id(code)},
                                       }}},
                    }};
    }
    return Json{jsoncons::json_object_arg,
                {
                    {"schema_version", supported_log_record_schema_version},
                    {"sequence", sequence},
                    {"timestamp_utc", record.timestamp_utc},
                    {"simulation_time_ns", record.simulation_time.count()},
                    {"level", level_name(record.level)},
                    {"source", record.source},
                    {"event", record.event},
                    {"message", record.message},
                }};
}

[[nodiscard]] Json read_schema(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw RunLogError{core::ErrorCode::input_unreadable,
                          "Cannot open log-record schema: " + path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw RunLogError{core::ErrorCode::json_invalid, "Invalid JSON in log-record schema '" +
                                                             path.string() + "': " + error.what()};
    }
}

// jsoncons ownership is RAII-managed; the MSVC-bundled analyzer cannot model its tagged storage.
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw RunLogError{core::ErrorCode::schema_invalid,
                          "Invalid log-record schema '" + path.string() + "': " + error.what()};
    }
}
// NOLINTEND(clang-analyzer-cplusplus.NewDelete)

} // namespace

JsonLinesRunLog::JsonLinesRunLog(std::filesystem::path output_path)
    : path_{std::move(output_path)} {
    std::error_code error;
    const auto parent = path_.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, error);
        if (error) {
            throw RunLogError{core::ErrorCode::output_unwritable,
                              "Cannot create run-log directory '" + parent.string() +
                                  "': " + error.message()};
        }
    }

    stream_.open(path_, std::ios::binary | std::ios::trunc);
    if (!stream_) {
        throw RunLogError{core::ErrorCode::output_unwritable,
                          "Cannot write run log: " + path_.string()};
    }
}

// jsoncons ownership is RAII-managed; the MSVC-bundled analyzer reports a false leak here.
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
void JsonLinesRunLog::write(const RunLogRecord& record) {
    if (sequence_ == std::numeric_limits<std::uint64_t>::max()) {
        throw RunLogError{core::ErrorCode::numeric_overflow, "Run-log sequence overflow"};
    }

    const auto document = make_document(sequence_, record);
    document.dump(stream_);
    stream_.put('\n');
    stream_.flush();
    if (!stream_) {
        throw RunLogError{core::ErrorCode::output_unwritable,
                          "Cannot complete run log: " + path_.string()};
    }
    ++sequence_;
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

const std::filesystem::path& JsonLinesRunLog::path() const noexcept { return path_; }

std::uint64_t JsonLinesRunLog::next_sequence() const noexcept { return sequence_; }

void validate_run_log(const RunLogValidation& validation) {
    const auto schema_document = read_schema(validation.schema_path);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    const auto schema = compile_schema(schema_document, validation.schema_path);

    std::ifstream stream{validation.log_path, std::ios::binary};
    if (!stream) {
        throw RunLogError{core::ErrorCode::input_unreadable,
                          "Cannot open run log: " + validation.log_path.string()};
    }

    std::string line;
    std::uint64_t line_number{};
    while (std::getline(stream, line)) {
        ++line_number;
        try {
            const auto record = Json::parse(line);
            schema.validate(record);
            const auto expected_sequence = line_number - 1;
            if (record.at("sequence").as<std::uint64_t>() != expected_sequence) {
                throw RunLogError{core::ErrorCode::json_invalid,
                                  "Run-log sequence does not match its line position"};
            }
        } catch (const std::exception& error) {
            throw RunLogError{core::ErrorCode::json_invalid,
                              "Invalid run-log record at line " + std::to_string(line_number) +
                                  " in '" + validation.log_path.string() + "': " + error.what()};
        }
    }
    if (stream.bad()) {
        throw RunLogError{core::ErrorCode::input_unreadable,
                          "Cannot read run log: " + validation.log_path.string()};
    }
    if (line_number == 0) {
        throw RunLogError{core::ErrorCode::json_invalid,
                          "Run log is empty: " + validation.log_path.string()};
    }
}

} // namespace mehlissa::experiment

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/run_log.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] std::filesystem::path log_schema_path() {
    return root_path() / "data" / "schemas" / "log-record" / "1.0.0.schema.json";
}

[[nodiscard]] std::filesystem::path make_test_directory() {
    const auto unique_value = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("mehlissa-run-log-test-" + std::to_string(unique_value));
}

} // namespace

TEST_CASE("A run log writes ordered schema-valid JSON Lines records", "[experiment][log]") {
    using namespace std::chrono_literals;

    const auto directory = make_test_directory();
    const auto log_path = directory / "run.log.jsonl";
    {
        mehlissa::experiment::JsonLinesRunLog log{log_path};
        log.write({
            "2026-08-26T21:00:00Z",
            0ns,
            mehlissa::experiment::LogLevel::info,
            "runner",
            "run_started",
            "Run started",
            std::nullopt,
        });
        log.write({
            "2026-08-26T21:00:01Z",
            5ns,
            mehlissa::experiment::LogLevel::error,
            "runner",
            "run_failed",
            "Manifest rejected",
            mehlissa::core::ErrorCode::manifest_invalid,
        });

        REQUIRE(log.path() == log_path);
        REQUIRE(log.next_sequence() == 2);
    }

    mehlissa::experiment::validate_run_log({log_path, log_schema_path()});

    std::ifstream input{log_path, std::ios::binary};
    REQUIRE(input);
    const std::string contents{std::istreambuf_iterator<char>{input}, {}};
    REQUIRE(contents.find("\"sequence\":0") != std::string::npos);
    REQUIRE(contents.find("\"sequence\":1") != std::string::npos);
    REQUIRE(contents.find("\"id\":\"MEHLISSA-E2004\"") != std::string::npos);
    input.close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("A run-log validator reports the failing line", "[experiment][log]") {
    const auto directory = make_test_directory();
    const auto log_path = directory / "invalid.jsonl";
    std::filesystem::create_directories(directory);
    {
        std::ofstream output{log_path, std::ios::binary};
        REQUIRE(output);
        output << "{}\nnot-json\n";
    }

    REQUIRE_THROWS_WITH(mehlissa::experiment::validate_run_log({log_path, log_schema_path()}),
                        Catch::Matchers::StartsWith("Invalid run-log record at line 1"));

    std::filesystem::remove_all(directory);
}

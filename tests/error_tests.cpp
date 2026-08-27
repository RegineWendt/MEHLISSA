// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <string_view>
#include <utility>

TEST_CASE("Machine-readable error identifiers are stable", "[core][error]") {
    using mehlissa::core::ErrorCode;
    constexpr std::array expected{
        std::pair{ErrorCode::command_line_invalid, std::string_view{"MEHLISSA-E1001"}},
        std::pair{ErrorCode::input_unreadable, std::string_view{"MEHLISSA-E2001"}},
        std::pair{ErrorCode::json_invalid, std::string_view{"MEHLISSA-E2002"}},
        std::pair{ErrorCode::schema_invalid, std::string_view{"MEHLISSA-E2003"}},
        std::pair{ErrorCode::manifest_invalid, std::string_view{"MEHLISSA-E2004"}},
        std::pair{ErrorCode::data_invalid, std::string_view{"MEHLISSA-E2005"}},
        std::pair{ErrorCode::output_unwritable, std::string_view{"MEHLISSA-E3001"}},
        std::pair{ErrorCode::provenance_invalid, std::string_view{"MEHLISSA-E3002"}},
        std::pair{ErrorCode::lifecycle_invalid, std::string_view{"MEHLISSA-E4001"}},
        std::pair{ErrorCode::invariant_violated, std::string_view{"MEHLISSA-E4002"}},
        std::pair{ErrorCode::numeric_overflow, std::string_view{"MEHLISSA-E4003"}},
        std::pair{ErrorCode::checkpoint_invalid, std::string_view{"MEHLISSA-E5001"}},
        std::pair{ErrorCode::checkpoint_incompatible, std::string_view{"MEHLISSA-E5002"}},
        std::pair{ErrorCode::internal_failure, std::string_view{"MEHLISSA-E9001"}},
    };

    for (const auto& [code, identifier] : expected) {
        REQUIRE(mehlissa::core::error_code_id(code) == identifier);
    }
}

TEST_CASE("A structured error retains code and diagnostic", "[core][error]") {
    const mehlissa::core::MehlissaError error{mehlissa::core::ErrorCode::manifest_invalid,
                                              "Manifest rejected"};

    REQUIRE(error.code() == mehlissa::core::ErrorCode::manifest_invalid);
    REQUIRE(error.code_id() == "MEHLISSA-E2004");
    REQUIRE(std::string{error.what()} == "Manifest rejected");
}

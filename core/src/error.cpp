// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>

namespace mehlissa::core {

std::string_view error_code_id(const ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::command_line_invalid:
        return "MEHLISSA-E1001";
    case ErrorCode::input_unreadable:
        return "MEHLISSA-E2001";
    case ErrorCode::json_invalid:
        return "MEHLISSA-E2002";
    case ErrorCode::schema_invalid:
        return "MEHLISSA-E2003";
    case ErrorCode::manifest_invalid:
        return "MEHLISSA-E2004";
    case ErrorCode::output_unwritable:
        return "MEHLISSA-E3001";
    case ErrorCode::provenance_invalid:
        return "MEHLISSA-E3002";
    case ErrorCode::lifecycle_invalid:
        return "MEHLISSA-E4001";
    case ErrorCode::invariant_violated:
        return "MEHLISSA-E4002";
    case ErrorCode::numeric_overflow:
        return "MEHLISSA-E4003";
    case ErrorCode::checkpoint_invalid:
        return "MEHLISSA-E5001";
    case ErrorCode::checkpoint_incompatible:
        return "MEHLISSA-E5002";
    case ErrorCode::internal_failure:
        return "MEHLISSA-E9001";
    }
    return "MEHLISSA-E9001";
}

MehlissaError::MehlissaError(const ErrorCode code, const std::string_view message)
    : std::runtime_error{std::string{message}}, code_{code} {}

ErrorCode MehlissaError::code() const noexcept { return code_; }

std::string_view MehlissaError::code_id() const noexcept { return error_code_id(code_); }

} // namespace mehlissa::core

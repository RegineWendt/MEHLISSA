// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_ERROR_HPP
#define MEHLISSA_CORE_ERROR_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace mehlissa::core {

enum class ErrorCode : std::uint16_t {
    command_line_invalid = 1001,
    input_unreadable = 2001,
    json_invalid = 2002,
    schema_invalid = 2003,
    manifest_invalid = 2004,
    output_unwritable = 3001,
    provenance_invalid = 3002,
    lifecycle_invalid = 4001,
    invariant_violated = 4002,
    numeric_overflow = 4003,
    checkpoint_invalid = 5001,
    checkpoint_incompatible = 5002,
    internal_failure = 9001,
};

[[nodiscard]] std::string_view error_code_id(ErrorCode code) noexcept;

class MehlissaError : public std::runtime_error {
  public:
    MehlissaError(ErrorCode code, std::string_view message);

    [[nodiscard]] ErrorCode code() const noexcept;
    [[nodiscard]] std::string_view code_id() const noexcept;

  private:
    ErrorCode code_;
};

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_ERROR_HPP

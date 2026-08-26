// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_EXPERIMENT_RUN_LOG_HPP
#define MEHLISSA_EXPERIMENT_RUN_LOG_HPP

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace mehlissa::experiment {

inline constexpr auto supported_log_record_schema_version = "1.0.0";

enum class LogLevel : std::uint8_t { debug, info, warning, error };

struct RunLogRecord final {
    std::string timestamp_utc;
    core::SimulationClock::Duration simulation_time;
    LogLevel level{LogLevel::info};
    std::string source;
    std::string event;
    std::string message;
    std::optional<core::ErrorCode> error_code;
};

struct RunLogValidation final {
    std::filesystem::path log_path;
    std::filesystem::path schema_path;
};

class RunLogError final : public core::MehlissaError {
  public:
    using core::MehlissaError::MehlissaError;
};

class JsonLinesRunLog final {
  public:
    explicit JsonLinesRunLog(std::filesystem::path output_path);

    JsonLinesRunLog(const JsonLinesRunLog&) = delete;
    JsonLinesRunLog& operator=(const JsonLinesRunLog&) = delete;
    JsonLinesRunLog(JsonLinesRunLog&&) = delete;
    JsonLinesRunLog& operator=(JsonLinesRunLog&&) = delete;

    void write(const RunLogRecord& record);

    [[nodiscard]] const std::filesystem::path& path() const noexcept;
    [[nodiscard]] std::uint64_t next_sequence() const noexcept;

  private:
    std::filesystem::path path_;
    std::ofstream stream_;
    std::uint64_t sequence_{};
};

void validate_run_log(const RunLogValidation& validation);

} // namespace mehlissa::experiment

#endif // MEHLISSA_EXPERIMENT_RUN_LOG_HPP

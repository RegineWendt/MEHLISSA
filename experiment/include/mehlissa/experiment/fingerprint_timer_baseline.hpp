// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_EXPERIMENT_FINGERPRINT_TIMER_BASELINE_HPP
#define MEHLISSA_EXPERIMENT_FINGERPRINT_TIMER_BASELINE_HPP

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::experiment {

inline constexpr auto supported_fingerprint_timer_baseline_schema_version = "1.0.0";

enum class FingerprintTimerEventKind : std::uint8_t {
    injection,
    first_localization,
    message_active,
    external_report
};

[[nodiscard]] std::string_view
fingerprint_timer_event_kind_id(FingerprintTimerEventKind kind) noexcept;

struct FingerprintCollectorCohortReference final {
    std::uint64_t collector_count{};
    core::SimulationClock::Duration external_report_time{};
};

struct FingerprintTimerBaseline final {
    std::string schema_version;
    std::string baseline_id;
    std::string fingerprint_id;
    std::string target_tissue;
    std::string target_region_id;
    std::uint64_t historical_organ_index{};
    std::string injection_site;
    std::uint64_t historical_injection_segment_index{};
    core::SimulationClock::Duration injection_time{};
    std::uint64_t locator_count{};
    std::uint64_t configured_target_count{};
    std::uint64_t target_locator_count{};
    core::SimulationClock::Duration first_localization_time{};
    core::SimulationClock::Duration assembly_duration{};
    std::vector<FingerprintCollectorCohortReference> collector_cohorts;
};

struct FingerprintTimerEvent final {
    FingerprintTimerEventKind kind{};
    core::SimulationClock::Duration time{};
    std::string fingerprint_id;
    std::string target_region_id;

    [[nodiscard]] bool operator==(const FingerprintTimerEvent&) const noexcept = default;
};

struct FingerprintTimerRun final {
    std::string baseline_id;
    std::uint64_t collector_count{};
    std::vector<FingerprintTimerEvent> events;
    core::SimulationClock::Duration post_assembly_collection_and_return_duration{};
};

class FingerprintTimerBaselineError final : public core::MehlissaError {
  public:
    using core::MehlissaError::MehlissaError;
};

[[nodiscard]] FingerprintTimerBaseline
load_fingerprint_timer_baseline(const std::filesystem::path& baseline_path,
                                const std::filesystem::path& schema_path);

[[nodiscard]] FingerprintTimerRun
run_fingerprint_timer_baseline(const FingerprintTimerBaseline& baseline,
                               std::uint64_t collector_count);

} // namespace mehlissa::experiment

#endif // MEHLISSA_EXPERIMENT_FINGERPRINT_TIMER_BASELINE_HPP

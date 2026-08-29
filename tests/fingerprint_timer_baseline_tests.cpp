// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/fingerprint_timer_baseline.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::experiment::FingerprintTimerEvent;
using mehlissa::experiment::FingerprintTimerEventKind;

[[nodiscard]] std::filesystem::path root() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::experiment::FingerprintTimerBaseline load_baseline() {
    return mehlissa::experiment::load_fingerprint_timer_baseline(
        root() / "examples" / "scenarios" / "fp9-lung-historical-timer-v1.json",
        root() / "data" / "schemas" / "fingerprint-timer-baseline" / "1.0.0.schema.json");
}

[[nodiscard]] FingerprintTimerEvent event(const FingerprintTimerEventKind kind,
                                          const std::chrono::nanoseconds time,
                                          const std::string& fingerprint_id = "FP9",
                                          const std::string& target_region_id = "lung") {
    return {kind, time, fingerprint_id, target_region_id};
}

} // namespace

TEST_CASE("The historical FP9 lung timer reproduces both published collector cohorts",
          "[m3][experiment][fingerprint][fp9]") {
    const auto baseline = load_baseline();

    REQUIRE(baseline.baseline_id == "fp9-lung-historical-timer-v1");
    REQUIRE(baseline.fingerprint_id == "FP9");
    REQUIRE(baseline.target_tissue == "lung");
    REQUIRE(baseline.target_region_id == "lung");
    REQUIRE(baseline.historical_organ_index == 61);
    REQUIRE(baseline.historical_injection_segment_index == 64);
    REQUIRE(baseline.locator_count == 1000);
    REQUIRE(baseline.configured_target_count == 9);
    REQUIRE(baseline.target_locator_count == 111);

    const auto one_thousand = mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 1000);
    const auto ten_thousand = mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 10000);
    const std::vector shared_prefix{
        event(FingerprintTimerEventKind::injection, 0s),
        event(FingerprintTimerEventKind::first_localization, 25s),
        event(FingerprintTimerEventKind::message_active, 40'990ms),
    };

    REQUIRE(one_thousand.events.size() == 4);
    REQUIRE(ten_thousand.events.size() == 4);
    CHECK(std::vector(one_thousand.events.begin(), one_thousand.events.begin() + 3) ==
          shared_prefix);
    CHECK(std::vector(ten_thousand.events.begin(), ten_thousand.events.begin() + 3) ==
          shared_prefix);
    CHECK(one_thousand.events.back() == event(FingerprintTimerEventKind::external_report, 209s));
    CHECK(ten_thousand.events.back() == event(FingerprintTimerEventKind::external_report, 91s));
    CHECK(one_thousand.post_assembly_collection_and_return_duration == 168'010ms);
    CHECK(ten_thousand.post_assembly_collection_and_return_duration == 50'010ms);
}

TEST_CASE("The fingerprint timer is target-data driven and contains no lung branch",
          "[m3][experiment][fingerprint][architecture]") {
    auto baseline = load_baseline();
    baseline.baseline_id = "synthetic-target-timer";
    baseline.fingerprint_id = "FP-test";
    baseline.target_tissue = "synthetic tissue";
    baseline.target_region_id = "organ-x";
    baseline.historical_organ_index = 999;
    baseline.first_localization_time = 2s;
    baseline.assembly_duration = 3s;
    baseline.collector_cohorts = {{7, 8s}};

    const auto result = mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 7);
    REQUIRE(result.baseline_id == "synthetic-target-timer");
    REQUIRE(result.events ==
            std::vector{
                event(FingerprintTimerEventKind::injection, 0s, "FP-test", "organ-x"),
                event(FingerprintTimerEventKind::first_localization, 2s, "FP-test", "organ-x"),
                event(FingerprintTimerEventKind::message_active, 5s, "FP-test", "organ-x"),
                event(FingerprintTimerEventKind::external_report, 8s, "FP-test", "organ-x"),
            });
    CHECK(result.post_assembly_collection_and_return_duration == 3s);
}

TEST_CASE("The fingerprint timer rejects acausal and undefined report cohorts",
          "[m3][experiment][fingerprint][invariant]") {
    auto baseline = load_baseline();

    REQUIRE_THROWS_WITH(
        mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 20000),
        "Fingerprint-timer baseline does not define the requested collector cohort");

    baseline.collector_cohorts.front().external_report_time = 40s;
    REQUIRE_THROWS_WITH(
        mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 1000),
        "Fingerprint-timer reports require a positive collector cohort and an active message");

    baseline = load_baseline();
    baseline.configured_target_count = 10;
    REQUIRE_THROWS_WITH(
        mehlissa::experiment::run_fingerprint_timer_baseline(baseline, 1000),
        "Fingerprint-timer populations and target allocation must be positive and bounded");
}

TEST_CASE("Fingerprint event identifiers are stable for reports and logs",
          "[m3][experiment][fingerprint][event]") {
    CHECK(mehlissa::experiment::fingerprint_timer_event_kind_id(
              FingerprintTimerEventKind::injection) == "injection");
    CHECK(mehlissa::experiment::fingerprint_timer_event_kind_id(
              FingerprintTimerEventKind::first_localization) == "first_localization");
    CHECK(mehlissa::experiment::fingerprint_timer_event_kind_id(
              FingerprintTimerEventKind::message_active) == "message_active");
    CHECK(mehlissa::experiment::fingerprint_timer_event_kind_id(
              FingerprintTimerEventKind::external_report) == "external_report");
}

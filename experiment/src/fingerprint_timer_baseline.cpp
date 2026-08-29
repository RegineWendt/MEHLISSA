// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/fingerprint_timer_baseline.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::experiment {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;
using Duration = core::SimulationClock::Duration;

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw FingerprintTimerBaselineError{core::ErrorCode::input_unreadable,
                                            "Cannot open " + std::string{role} + ": " +
                                                path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw FingerprintTimerBaselineError{core::ErrorCode::json_invalid,
                                            "Invalid JSON in " + std::string{role} + " '" +
                                                path.string() + "': " + error.what()};
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw FingerprintTimerBaselineError{core::ErrorCode::schema_invalid,
                                            "Invalid fingerprint-timer schema '" + path.string() +
                                                "': " + error.what()};
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw FingerprintTimerBaselineError{core::ErrorCode::data_invalid,
                                            "Fingerprint-timer baseline '" + path.string() +
                                                "' does not satisfy its schema: " + error.what()};
    }
}

[[nodiscard]] Duration milliseconds(const Json& object, const std::string_view key) {
    constexpr auto nanoseconds_per_millisecond = std::uint64_t{1'000'000};
    const auto value = object.at(key).as<std::uint64_t>();
    using Representation = Duration::rep;
    constexpr auto maximum = static_cast<std::uint64_t>(std::numeric_limits<Representation>::max());
    if (value > maximum / nanoseconds_per_millisecond) {
        throw FingerprintTimerBaselineError{
            core::ErrorCode::numeric_overflow,
            "Fingerprint-timer duration exceeds the supported nanosecond range"};
    }
    return std::chrono::duration_cast<Duration>(std::chrono::milliseconds{value});
}

[[nodiscard]] Duration checked_add(const Duration left, const Duration right) {
    const auto maximum = std::numeric_limits<Duration::rep>::max();
    if (right.count() > maximum - left.count()) {
        throw FingerprintTimerBaselineError{core::ErrorCode::numeric_overflow,
                                            "Fingerprint-timer event time overflow"};
    }
    return left + right;
}

[[nodiscard]] Duration validate_baseline(const FingerprintTimerBaseline& baseline) {
    if (baseline.schema_version != supported_fingerprint_timer_baseline_schema_version) {
        throw FingerprintTimerBaselineError{
            core::ErrorCode::data_invalid, "Unsupported fingerprint-timer baseline schema version"};
    }
    if (baseline.injection_time < Duration::zero() ||
        baseline.first_localization_time < baseline.injection_time ||
        baseline.assembly_duration <= Duration::zero()) {
        throw FingerprintTimerBaselineError{core::ErrorCode::data_invalid,
                                            "Fingerprint-timer events require non-negative, causal "
                                            "localization and assembly times"};
    }
    if (baseline.locator_count == 0 || baseline.configured_target_count == 0 ||
        baseline.target_locator_count == 0 ||
        baseline.target_locator_count > baseline.locator_count ||
        baseline.target_locator_count > baseline.locator_count / baseline.configured_target_count ||
        baseline.collector_cohorts.empty()) {
        throw FingerprintTimerBaselineError{
            core::ErrorCode::data_invalid,
            "Fingerprint-timer populations and target allocation must be positive and bounded"};
    }

    const auto message_active_time =
        checked_add(baseline.first_localization_time, baseline.assembly_duration);
    std::unordered_set<std::uint64_t> collector_counts;
    for (const auto& cohort : baseline.collector_cohorts) {
        if (cohort.collector_count == 0 || cohort.external_report_time < message_active_time) {
            throw FingerprintTimerBaselineError{core::ErrorCode::data_invalid,
                                                "Fingerprint-timer reports require a positive "
                                                "collector cohort and an active message"};
        }
        if (!collector_counts.insert(cohort.collector_count).second) {
            throw FingerprintTimerBaselineError{
                core::ErrorCode::data_invalid,
                "Fingerprint collector cohort counts must be unique"};
        }
    }
    return message_active_time;
}

[[nodiscard]] std::vector<FingerprintCollectorCohortReference>
decode_collector_cohorts(const Json& timing) {
    std::vector<FingerprintCollectorCohortReference> cohorts;
    const auto& configured = timing.at("collector_cohorts");
    cohorts.reserve(configured.size());
    for (const auto& cohort : configured.array_range()) {
        cohorts.push_back({cohort.at("collector_count").as<std::uint64_t>(),
                           milliseconds(cohort, "external_report_time_ms")});
    }
    return cohorts;
}

} // namespace

std::string_view fingerprint_timer_event_kind_id(const FingerprintTimerEventKind kind) noexcept {
    switch (kind) {
    case FingerprintTimerEventKind::injection:
        return "injection";
    case FingerprintTimerEventKind::first_localization:
        return "first_localization";
    case FingerprintTimerEventKind::message_active:
        return "message_active";
    case FingerprintTimerEventKind::external_report:
        return "external_report";
    }
    return "unknown";
}

FingerprintTimerBaseline load_fingerprint_timer_baseline(const std::filesystem::path& baseline_path,
                                                         const std::filesystem::path& schema_path) {
    const auto schema_document = read_json(schema_path, "fingerprint-timer schema");
    const auto baseline_document = read_json(baseline_path, "fingerprint-timer baseline");
    const auto schema = compile_schema(schema_document, schema_path);
    validate_document(baseline_document, schema, baseline_path);

    try {
        const auto& baseline = baseline_document.at("baseline");
        const auto& fingerprint = baseline_document.at("fingerprint");
        const auto& injection = baseline_document.at("injection");
        const auto& population = baseline_document.at("population");
        const auto& timing = baseline_document.at("timing");
        FingerprintTimerBaseline result{
            baseline_document.at("schema_version").as<std::string>(),
            baseline.at("id").as<std::string>(),
            fingerprint.at("id").as<std::string>(),
            fingerprint.at("target_tissue").as<std::string>(),
            fingerprint.at("target_region_id").as<std::string>(),
            fingerprint.at("historical_organ_index").as<std::uint64_t>(),
            injection.at("site").as<std::string>(),
            injection.at("historical_segment_index").as<std::uint64_t>(),
            milliseconds(injection, "time_ms"),
            population.at("locator_count").as<std::uint64_t>(),
            population.at("configured_target_count").as<std::uint64_t>(),
            population.at("target_locator_count").as<std::uint64_t>(),
            milliseconds(timing, "first_localization_time_ms"),
            milliseconds(timing, "assembly_duration_ms"),
            decode_collector_cohorts(timing),
        };
        static_cast<void>(validate_baseline(result));
        return result;
    } catch (const FingerprintTimerBaselineError&) {
        throw;
    } catch (const std::exception& error) {
        throw FingerprintTimerBaselineError{core::ErrorCode::data_invalid,
                                            "Cannot decode validated fingerprint-timer baseline '" +
                                                baseline_path.string() + "': " + error.what()};
    }
}

FingerprintTimerRun run_fingerprint_timer_baseline(const FingerprintTimerBaseline& baseline,
                                                   const std::uint64_t collector_count) {
    const auto message_active_time = validate_baseline(baseline);
    const auto cohort = std::ranges::find(baseline.collector_cohorts, collector_count,
                                          &FingerprintCollectorCohortReference::collector_count);
    if (cohort == baseline.collector_cohorts.end()) {
        throw FingerprintTimerBaselineError{
            core::ErrorCode::data_invalid,
            "Fingerprint-timer baseline does not define the requested collector cohort"};
    }

    const auto event = [&](const FingerprintTimerEventKind kind, const Duration time) {
        return FingerprintTimerEvent{kind, time, baseline.fingerprint_id,
                                     baseline.target_region_id};
    };
    return {
        baseline.baseline_id,
        collector_count,
        {
            event(FingerprintTimerEventKind::injection, baseline.injection_time),
            event(FingerprintTimerEventKind::first_localization, baseline.first_localization_time),
            event(FingerprintTimerEventKind::message_active, message_active_time),
            event(FingerprintTimerEventKind::external_report, cohort->external_report_time),
        },
        cohort->external_report_time - message_active_time,
    };
}

} // namespace mehlissa::experiment

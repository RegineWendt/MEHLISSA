// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/provenance.hpp>
#include <mehlissa/scenarios/fingerprinting/level_a_composer.hpp>
#include <mehlissa/scenarios/fingerprinting/level_b_detection.hpp>
#include <mehlissa/scenarios/fingerprinting/result_report.hpp>
#include <mehlissa/scenarios/fingerprinting/runtime_coordinator.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <ranges>
#include <string>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::scenarios::fingerprinting::ArtifactRole;
using mehlissa::scenarios::fingerprinting::StageKind;

[[nodiscard]] std::filesystem::path root() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::scenarios::fingerprinting::ScenarioProfile load_profile() {
    return mehlissa::scenarios::fingerprinting::load_scenario_profile(
        {root() / "examples" / "scenarios" / "fp9-lung-level-a-v1.json",
         root() / "data" / "schemas" / "fingerprinting-scenario-profile" / "1.0.0.schema.json"});
}

} // namespace

TEST_CASE("M7.1 composes one schema-valid artifact for every M2-M6 role",
          "[m7][fingerprinting][composition]") {
    const auto profile = load_profile();
    const auto plan = mehlissa::scenarios::fingerprinting::compose_level_a_plan(profile, root());

    REQUIRE(plan.profile.scenario.id == "fp9-lung-level-a-v1");
    REQUIRE(plan.profile.scenario.acceptance_level == "A");
    REQUIRE(plan.profile.run.id == "fp9-lung-level-a-collectors-1000");
    REQUIRE(plan.profile.run.master_seed == 20260901);
    REQUIRE(plan.artifacts.size() == 12);
    CHECK(std::ranges::all_of(plan.artifacts, [](const auto& artifact) {
        return std::filesystem::is_regular_file(artifact.definition_path) &&
               std::filesystem::is_regular_file(artifact.schema_path);
    }));
    CHECK(std::ranges::count(plan.artifacts, ArtifactRole::timer_baseline,
                             &mehlissa::scenarios::fingerprinting::ResolvedArtifact::role) == 1);

    REQUIRE(plan.timer_baseline.baseline_id == "fp9-lung-historical-timer-v1");
    REQUIRE(plan.timer_run.collector_count == 1000);
    REQUIRE(plan.timer_run.events.size() == 4);
    CHECK(plan.timer_run.events.front().time == 0s);
    CHECK(plan.timer_run.events.back().time == 209s);
    CHECK(plan.timer_run.post_assembly_collection_and_return_duration == 168'010ms);
}

TEST_CASE("M7.1 exposes the complete causal identity-flow contract",
          "[m7][fingerprinting][stage-contract]") {
    const auto profile = load_profile();
    const std::vector expected{StageKind::injection,
                               StageKind::body_transport,
                               StageKind::organ_transfer,
                               StageKind::capillary_localization,
                               StageKind::molecular_recognition,
                               StageKind::fingerprint_assembly,
                               StageKind::local_collection,
                               StageKind::collector_return,
                               StageKind::gateway_measurement,
                               StageKind::external_report};

    REQUIRE(profile.acceptance.required_stage_order == expected);
    CHECK(profile.target.fingerprint_id == "FP9");
    CHECK(profile.target.tissue == "lung");
    CHECK(profile.target.region_id == "lung");
    CHECK(profile.acceptance.deterministic_replay_required);
    CHECK_FALSE(profile.acceptance.clinical_validation_claim);
}

TEST_CASE("M7.1 composition is deterministic for the same profile and repository",
          "[m7][fingerprinting][determinism]") {
    const auto profile = load_profile();
    const auto first = mehlissa::scenarios::fingerprinting::compose_level_a_plan(profile, root());
    const auto second = mehlissa::scenarios::fingerprinting::compose_level_a_plan(profile, root());

    REQUIRE(first.profile.run.master_seed == second.profile.run.master_seed);
    REQUIRE(first.artifacts.size() == second.artifacts.size());
    for (std::size_t index = 0; index < first.artifacts.size(); ++index) {
        CHECK(first.artifacts[index].role == second.artifacts[index].role);
        CHECK(first.artifacts[index].definition_path == second.artifacts[index].definition_path);
        CHECK(first.artifacts[index].schema_path == second.artifacts[index].schema_path);
    }
    CHECK(first.timer_run.events == second.timer_run.events);
    CHECK(first.timer_run.post_assembly_collection_and_return_duration ==
          second.timer_run.post_assembly_collection_and_return_duration);
}

TEST_CASE("M7.1 rejects unsafe, incomplete, and identity-inconsistent compositions",
          "[m7][fingerprinting][invariant]") {
    auto profile = load_profile();
    profile.artifacts.front().definition_path = "../outside.json";
    REQUIRE_THROWS_WITH(
        mehlissa::scenarios::fingerprinting::validate_scenario_profile(profile),
        "M7.1 artifacts must have unique roles and safe repository-relative JSON paths");

    profile = load_profile();
    profile.artifacts.back().role = ArtifactRole::body_model;
    REQUIRE_THROWS_WITH(
        mehlissa::scenarios::fingerprinting::validate_scenario_profile(profile),
        "M7.1 artifacts must have unique roles and safe repository-relative JSON paths");

    profile = load_profile();
    profile.acceptance.required_stage_order[1] = StageKind::organ_transfer;
    REQUIRE_THROWS_WITH(
        mehlissa::scenarios::fingerprinting::validate_scenario_profile(profile),
        "M7.1 scenario must preserve the canonical causal fingerprinting stage order");

    profile = load_profile();
    profile.target.region_id = "kidney";
    REQUIRE_THROWS_WITH(mehlissa::scenarios::fingerprinting::compose_level_a_plan(profile, root()),
                        "M7.1 target identity does not match the selected timer baseline");
}

TEST_CASE("M7.2 initializes the selected stack and emits one causal identity trace",
          "[m7][fingerprinting][runtime][M7.2]") {
    const auto plan =
        mehlissa::scenarios::fingerprinting::compose_level_a_plan(load_profile(), root());
    const auto result = mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan);

    REQUIRE(result.contract_version == "1.0.0");
    REQUIRE(result.components.size() == 11);
    CHECK(std::ranges::count(
              result.components, true,
              &mehlissa::scenarios::fingerprinting::RuntimeComponentIdentity::advanced) == 4);
    REQUIRE(result.stages.size() == 10);
    CHECK(result.stages.front().stage == StageKind::injection);
    CHECK(result.stages.front().time == 0s);
    CHECK(result.stages.back().stage == StageKind::external_report);
    CHECK(result.stages.back().time == 209s);
    for (std::size_t index = 1; index < result.stages.size(); ++index) {
        CHECK(result.stages[index - 1].time <= result.stages[index].time);
        CHECK(result.stages[index - 1].output_identity == result.stages[index].input_identity);
    }
}

TEST_CASE("M7.2 runtime trace is deterministic for the same manifest and seed",
          "[m7][fingerprinting][runtime][determinism][M7.2]") {
    const auto plan =
        mehlissa::scenarios::fingerprinting::compose_level_a_plan(load_profile(), root());
    CHECK(mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan) ==
          mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan));
}

TEST_CASE("M7.3 writes a strict manifest-complete reproducible result",
          "[m7][fingerprinting][result][M7.3]") {
    const auto plan =
        mehlissa::scenarios::fingerprinting::compose_level_a_plan(load_profile(), root());
    const auto runtime = mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan);
    const auto report =
        mehlissa::scenarios::fingerprinting::make_fingerprinting_result_report(plan, runtime);
    const auto first = root() / "build" / "windows-msvc" / "m7-result-first.json";
    const auto second = root() / "build" / "windows-msvc" / "m7-result-second.json";
    const auto schema = root() / "data" / "schemas" / "fingerprinting-result" / "1.0.0.schema.json";

    mehlissa::scenarios::fingerprinting::write_fingerprinting_result_report(report, first, schema);
    mehlissa::scenarios::fingerprinting::write_fingerprinting_result_report(report, second, schema);

    REQUIRE(report.artifacts.size() == 12);
    CHECK(report.runtime.stages.size() == 10);
    CHECK(report.deterministic_replay_required);
    CHECK_FALSE(report.clinical_validation_claim);
    CHECK(mehlissa::experiment::sha256_file(first) == mehlissa::experiment::sha256_file(second));
    std::filesystem::remove(first);
    std::filesystem::remove(second);
}

TEST_CASE("M7.4 replaces immediate recognition with concentration-driven receptor binding",
          "[m7][fingerprinting][binding][M7.4]") {
    const auto plan =
        mehlissa::scenarios::fingerprinting::compose_level_a_plan(load_profile(), root());
    auto runtime = mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan);
    const auto detection = mehlissa::scenarios::fingerprinting::run_level_b_detection(
        plan, mehlissa::scenarios::fingerprinting::default_level_b_detection_input(plan));
    mehlissa::scenarios::fingerprinting::apply_level_b_detection(runtime, detection);

    REQUIRE(detection.detected);
    CHECK(detection.final_bound_fraction == Catch::Approx(0.7362632708334493));
    REQUIRE(detection.absolute_detection_time.has_value());
    CHECK(detection.absolute_detection_time.value() > 25s);
    CHECK(runtime.stages[4].basis ==
          mehlissa::scenarios::fingerprinting::ExecutionBasis::model_execution);
    CHECK(runtime.stages[4].output_identity == detection.event_id);
    CHECK(runtime.stages[5].input_identity == detection.event_id);
}

TEST_CASE("M7.4 reports a below-threshold negative control without fabricating an event",
          "[m7][fingerprinting][binding][negative-control][M7.4]") {
    const auto plan =
        mehlissa::scenarios::fingerprinting::compose_level_a_plan(load_profile(), root());
    auto input = mehlissa::scenarios::fingerprinting::default_level_b_detection_input(plan);
    input.ligand_concentration = mehlissa::core::moles_per_cubic_meter(1.0e-9);
    const auto detection = mehlissa::scenarios::fingerprinting::run_level_b_detection(plan, input);
    auto runtime = mehlissa::scenarios::fingerprinting::run_level_a_runtime(plan);

    CHECK_FALSE(detection.detected);
    CHECK_FALSE(detection.relative_threshold_crossing.has_value());
    CHECK_FALSE(detection.absolute_detection_time.has_value());
    CHECK_THROWS_AS(
        mehlissa::scenarios::fingerprinting::apply_level_b_detection(runtime, detection),
        mehlissa::core::MehlissaError);
}

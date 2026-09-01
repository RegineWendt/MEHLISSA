// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_a_composer.hpp>

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

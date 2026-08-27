// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/body_state_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::models::body::VascularGraph load_bvs95() {
    return mehlissa::models::body::load_vascular_graph({
        root_path() / "data" / "body-models" / "bvs95-dissertation-rest-v1.json",
        root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::body::VascularGraph load_synthetic() {
    return mehlissa::models::body::load_vascular_graph({
        root_path() / "examples" / "body-models" / "synthetic-branching-circuit.json",
        root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::body::BodyStateProfile load_profile(const std::string& filename) {
    return mehlissa::models::body::load_body_state_profile({
        root_path() / "data" / "body-states" / filename,
        root_path() / "data" / "schemas" / "body-state-profile" / "1.0.0.schema.json",
    });
}

[[nodiscard]] double flow(const mehlissa::models::body::VascularGraph& graph,
                          const std::string& segment_id) {
    const auto* segment = graph.find_segment(segment_id);
    REQUIRE(segment != nullptr);
    return mehlissa::core::in_cubic_meters_per_second(segment->hemodynamics.flow_rate);
}

} // namespace

TEST_CASE("Checked-in rest exercise and posture profiles load without recompilation",
          "[body][state][schema]") {
    const auto base = load_bvs95();
    const auto rest = mehlissa::models::body::apply_body_state_profile(
        base, load_profile("bvs95-rest-supine-v1.json"));
    const auto exercise = mehlissa::models::body::apply_body_state_profile(
        base, load_profile("bvs95-supine-cycle-exercise-1.9x-v1.json"));
    const auto upright = mehlissa::models::body::apply_body_state_profile(
        base, load_profile("bvs95-head-up-tilt-70deg-v1.json"));

    REQUIRE(flow(rest, "bvs95-002") == Catch::Approx(0.0001));
    REQUIRE(flow(exercise, "bvs95-002") == Catch::Approx(0.00019));
    REQUIRE(flow(upright, "bvs95-002") == Catch::Approx(0.00007833333333333333));
    REQUIRE(exercise.validity.physiological_state.find("bicycle exercise") != std::string::npos);
    REQUIRE(upright.validity.physiological_state.find("head-up tilt") != std::string::npos);
    REQUIRE_NOTHROW(mehlissa::models::body::validate_vascular_graph(rest));
    REQUIRE_NOTHROW(mehlissa::models::body::validate_vascular_graph(exercise));
    REQUIRE_NOTHROW(mehlissa::models::body::validate_vascular_graph(upright));
}

TEST_CASE("State application preserves geometry and consistently changes velocity",
          "[body][state][invariants]") {
    const auto base = load_bvs95();
    const auto exercise = mehlissa::models::body::apply_body_state_profile(
        base, load_profile("bvs95-supine-cycle-exercise-1.9x-v1.json"));

    REQUIRE(exercise.segments.size() == base.segments.size());
    for (std::size_t index = 0; index < base.segments.size(); ++index) {
        REQUIRE(mehlissa::core::in_square_meters(
                    exercise.segments[index].geometry.cross_section_area) ==
                mehlissa::core::in_square_meters(base.segments[index].geometry.cross_section_area));
        REQUIRE(flow(exercise, exercise.segments[index].id) ==
                Catch::Approx(flow(base, base.segments[index].id) * 1.9).epsilon(1.0e-9));
        REQUIRE(mehlissa::core::in_meters_per_second(
                    exercise.segments[index].hemodynamics.mean_velocity) ==
                Catch::Approx(mehlissa::core::in_meters_per_second(
                                  base.segments[index].hemodynamics.mean_velocity) *
                              1.9)
                    .epsilon(1.0e-9));
    }
}

TEST_CASE("Transition overrides recompute the complete conserved stationary flow",
          "[body][state][redistribution]") {
    const mehlissa::models::body::BodyStateProfile profile{
        "1.0.0",
        "synthetic-redistribution",
        "1.0.0",
        "Synthetic redistribution contract test",
        "synthetic-branching-circuit",
        "1.0.0",
        {"synthetic", "redistribution test", "Software verification only."},
        "artery-10",
        2.0,
        {{"artery-10", {{"organ-a", 0.25}, {"organ-b", 0.75}}}},
        {{"synthetic-state-test", "MEHLISSA M2.6 contract test", "CC-BY-4.0"}},
        {"Not physiological."},
    };

    const auto derived =
        mehlissa::models::body::apply_body_state_profile(load_synthetic(), profile);
    REQUIRE(flow(derived, "artery-10") == Catch::Approx(0.00002));
    REQUIRE(flow(derived, "organ-a") == Catch::Approx(0.000005));
    REQUIRE(flow(derived, "organ-b") == Catch::Approx(0.000015));
    REQUIRE(flow(derived, "vein-90") == Catch::Approx(0.00002));
    REQUIRE_NOTHROW(mehlissa::models::body::validate_vascular_graph(derived));
}

TEST_CASE("A state profile cannot be applied to an incompatible body model",
          "[body][state][validation]") {
    auto profile = load_profile("bvs95-rest-supine-v1.json");
    profile.compatible_model_version = "9.9.9";
    REQUIRE_THROWS_WITH(mehlissa::models::body::apply_body_state_profile(load_bvs95(), profile),
                        Catch::Matchers::ContainsSubstring("requires model"));
}

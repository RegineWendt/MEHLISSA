// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/cell/analytical_receptor_ligand_model.hpp>
#include <mehlissa/models/cell/time_varying_receptor_ligand_model.hpp>
#include <mehlissa/models/cell/time_varying_receptor_ligand_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <string_view>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] mehlissa::models::cell::TimeVaryingReceptorLigandProfile load_profile() {
    const auto root = project_root();
    return mehlissa::models::cell::load_time_varying_receptor_ligand_profile({
        root / "examples/cell-models/synthetic-time-varying-receptor-ligand-v1.json",
        root / "data/schemas/time-varying-receptor-ligand-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] const mehlissa::models::cell::TimeVaryingReceptorLigandReferenceCase&
reference_by_role(const mehlissa::models::cell::TimeVaryingReceptorLigandProfile& profile,
                  const std::string_view role) {
    for (const auto& reference : profile.reference_cases) {
        if (reference.role == role) {
            return reference;
        }
    }
    throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::data_invalid,
                                        "Missing time-varying reference role"};
}

[[nodiscard]] double seconds(const mehlissa::core::SimulationClock::Duration duration) {
    return std::chrono::duration<double>{duration}.count();
}

} // namespace

TEST_CASE("A strict time-varying binding profile carries constant and pulse references",
          "[m5][cell][binding][ode][schema]") {
    const auto profile = load_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-time-varying-receptor-ligand-v1");
    CHECK(profile.implementation_kind == mehlissa::models::cell::time_varying_receptor_ligand_kind);
    CHECK(profile.model.binding.model_id == "cell.receptor-ligand.time-varying.synthetic.v1");
    CHECK(profile.model.integration_step == 50ms);
    CHECK(profile.model.maximum_integration_steps == 10'000);
    CHECK(profile.reference_cases.size() == 2);
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 7);
}

TEST_CASE("RK4 constant exposure converges to the independent M5.1 analytical limit",
          "[m5][cell][binding][ode][analytical][convergence]") {
    const auto profile = load_profile();
    const auto& reference =
        reference_by_role(profile, mehlissa::models::cell::constant_analytical_limit_role);
    const mehlissa::models::cell::AnalyticalReceptorLigandModel analytical{profile.model.binding};
    const auto analytical_response = analytical.evaluate(
        {reference.request.request_id, reference.request.ligand_id,
         reference.request.compartment_id,
         reference.request.ligand_trajectory.front().concentration,
         reference.request.observation_time, reference.request.initial_bound_fraction});

    auto error_at = [&](const mehlissa::core::SimulationClock::Duration step) {
        auto config = profile.model;
        config.integration_step = step;
        const mehlissa::models::cell::TimeVaryingReceptorLigandModel numerical{config};
        return std::abs(numerical.evaluate(reference.request).final_bound_fraction -
                        analytical_response.final_bound_fraction);
    };
    const auto coarse_error = error_at(500ms);
    const auto medium_error = error_at(250ms);
    const auto fine_error = error_at(125ms);

    CHECK(medium_error < coarse_error);
    CHECK(fine_error < medium_error);
    CHECK(fine_error < coarse_error / 100.0);

    const auto model = mehlissa::models::cell::make_time_varying_receptor_ligand_model(profile);
    const auto response = model->evaluate(reference.request);
    CHECK(model->kind() == mehlissa::models::cell::time_varying_receptor_ligand_kind);
    CHECK(analytical_response.final_bound_fraction ==
          Catch::Approx(reference.expected_final_bound_fraction)
              .margin(reference.fraction_absolute_tolerance));
    CHECK(response.final_bound_fraction == Catch::Approx(analytical_response.final_bound_fraction)
                                               .margin(reference.fraction_absolute_tolerance));
    CHECK(response.peak_bound_fraction == Catch::Approx(reference.expected_peak_bound_fraction)
                                              .margin(reference.fraction_absolute_tolerance));
    REQUIRE(response.first_threshold_crossing_time.has_value());
    CHECK(seconds(response.first_threshold_crossing_time.value_or(
              mehlissa::core::SimulationClock::Duration::min())) ==
          Catch::Approx(reference.expected_threshold_crossing_seconds)
              .margin(reference.time_absolute_tolerance_seconds));
    CHECK(response.integration_steps == 200);
    CHECK(response.samples.size() == 201);
}

TEST_CASE("A prescribed ligand pulse produces binding then dissociation with a checked event",
          "[m5][cell][binding][ode][pulse][conservation]") {
    const auto profile = load_profile();
    const auto& reference =
        reference_by_role(profile, mehlissa::models::cell::piecewise_analytical_pulse_role);
    const auto model = mehlissa::models::cell::make_time_varying_receptor_ligand_model(profile);
    const auto response = model->evaluate(reference.request);

    CHECK(response.final_bound_fraction == Catch::Approx(reference.expected_final_bound_fraction)
                                               .margin(reference.fraction_absolute_tolerance));
    CHECK(response.peak_bound_fraction == Catch::Approx(reference.expected_peak_bound_fraction)
                                              .margin(reference.fraction_absolute_tolerance));
    CHECK(response.peak_bound_fraction > response.final_bound_fraction);
    REQUIRE(response.detection_threshold_reached);
    REQUIRE(response.first_threshold_crossing_time.has_value());
    CHECK(seconds(response.first_threshold_crossing_time.value_or(
              mehlissa::core::SimulationClock::Duration::min())) ==
          Catch::Approx(reference.expected_threshold_crossing_seconds)
              .margin(reference.time_absolute_tolerance_seconds));
    CHECK(response.integration_steps == 240);
    CHECK(response.samples.front().offset == 0s);
    CHECK(response.samples.back().offset == 12s);
    CHECK(mehlissa::models::cell::receptor_balance_error_moles(response) ==
          Catch::Approx(0.0).margin(1.0e-32));
}

TEST_CASE("Time-varying binding rejects unsafe trajectories and bounded-solver violations",
          "[m5][cell][binding][ode][validation]") {
    const auto profile = load_profile();
    const auto& reference =
        reference_by_role(profile, mehlissa::models::cell::piecewise_analytical_pulse_role);
    const mehlissa::models::cell::TimeVaryingReceptorLigandModel model{profile.model};

    auto unordered = reference.request;
    unordered.ligand_trajectory[1].offset = unordered.ligand_trajectory[0].offset;
    CHECK_THROWS_AS(model.evaluate(unordered), mehlissa::core::MehlissaError);

    auto missing_origin = reference.request;
    missing_origin.ligand_trajectory.front().offset = 1s;
    CHECK_THROWS_AS(model.evaluate(missing_origin), mehlissa::core::MehlissaError);

    auto unstable_config = profile.model;
    unstable_config.integration_step = 3s;
    const mehlissa::models::cell::TimeVaryingReceptorLigandModel unstable{unstable_config};
    CHECK_THROWS_AS(unstable.evaluate(reference.request), mehlissa::core::MehlissaError);

    auto bounded_config = profile.model;
    bounded_config.maximum_integration_steps = 1;
    const mehlissa::models::cell::TimeVaryingReceptorLigandModel bounded{bounded_config};
    CHECK_THROWS_AS(bounded.evaluate(reference.request), mehlissa::core::MehlissaError);

    auto no_signal = reference.request;
    for (auto& knot : no_signal.ligand_trajectory) {
        knot.concentration = mehlissa::core::moles_per_cubic_meter(0.0);
    }
    const auto no_detection = model.evaluate(no_signal);
    CHECK(no_detection.final_bound_fraction == 0.0);
    CHECK(no_detection.detection_threshold_reached == false);
    CHECK(no_detection.first_threshold_crossing_time.has_value() == false);

    auto duplicate_role = profile;
    duplicate_role.reference_cases[1].role = duplicate_role.reference_cases[0].role;
    CHECK_THROWS_AS(
        mehlissa::models::cell::validate_time_varying_receptor_ligand_profile(duplicate_role),
        mehlissa::core::MehlissaError);
}

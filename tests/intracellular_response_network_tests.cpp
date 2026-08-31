// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/intracellular_response_network.hpp>
#include <mehlissa/models/cell/intracellular_response_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] mehlissa::models::cell::IntracellularResponseProfile load_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_intracellular_response_profile(
        {root / "examples/cell-models/synthetic-intracellular-response-v1.json",
         root / "data/schemas/intracellular-response-profile/1.0.0.schema.json"});
}

[[nodiscard]] double seconds(const mehlissa::core::SimulationClock::Duration duration) {
    return std::chrono::duration<double>{duration}.count();
}

} // namespace

TEST_CASE("The intracellular profile binds one network to ODE and SSA variants",
          "[m5][cell][intracellular][schema]") {
    const auto profile = load_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.ode.kinetics.network_id == profile.ssa.kinetics.network_id);
    CHECK(profile.ssa.messenger_molecule_count == 200);
    CHECK(profile.reference.population_size == 1000);
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 7);
}

TEST_CASE("The intracellular ODE produces the checked messenger and effector response",
          "[m5][cell][intracellular][ode][reference]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::IntracellularOdeModel model{profile.ode};
    const auto response = model.evaluate(profile.reference.request);
    CHECK(model.kind() == mehlissa::models::cell::intracellular_ode_kind);
    CHECK(response.final_active_messenger_fraction ==
          Catch::Approx(profile.reference.expected_ode_messenger_fraction)
              .margin(profile.reference.ode_fraction_tolerance));
    CHECK(response.final_active_effector_fraction ==
          Catch::Approx(profile.reference.expected_ode_effector_fraction)
              .margin(profile.reference.ode_fraction_tolerance));
    REQUIRE(response.first_response_time.has_value());
    CHECK(seconds(response.first_response_time.value_or(0s)) ==
          Catch::Approx(profile.reference.expected_ode_response_seconds)
              .margin(profile.reference.ode_time_tolerance_seconds));
    CHECK(response.integration_steps == 2000);
    CHECK(response.samples.size() == 512);
    CHECK(response.dropped_samples == 1489);
}

TEST_CASE("The intracellular SSA is exactly replayable and conserves both pools",
          "[m5][cell][intracellular][ssa][determinism][conservation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::IntracellularSsaModel model{profile.ssa};
    mehlissa::core::RandomStream first{31, "m5.5.cell.31"};
    mehlissa::core::RandomStream repeat{31, "m5.5.cell.31"};
    const auto lhs = model.evaluate(profile.reference.request, first);
    const auto rhs = model.evaluate(profile.reference.request, repeat);
    CHECK(model.kind() == mehlissa::models::cell::intracellular_ssa_kind);
    CHECK(lhs.active_messenger_molecules == rhs.active_messenger_molecules);
    CHECK(lhs.active_effector_molecules == rhs.active_effector_molecules);
    CHECK(lhs.reaction_events == rhs.reaction_events);
    CHECK(lhs.random_draws == rhs.random_draws);
    CHECK(lhs.active_messenger_molecules <= lhs.total_messenger_molecules);
    CHECK(lhs.active_effector_molecules <= lhs.total_effector_molecules);
    CHECK(lhs.samples.back().offset == 10s);
}

TEST_CASE("The SSA population agrees with the shared deterministic network reference",
          "[m5][cell][intracellular][ode][ssa][population]") {
    const auto profile = load_profile();
    const auto comparison = mehlissa::models::cell::compare_intracellular_ode_ssa(profile);
    CHECK(comparison.mean_messenger_fraction ==
          Catch::Approx(comparison.deterministic.final_active_messenger_fraction)
              .margin(profile.reference.maximum_ssa_messenger_mean_error));
    CHECK(comparison.mean_effector_fraction ==
          Catch::Approx(comparison.deterministic.final_active_effector_fraction)
              .margin(profile.reference.maximum_ssa_effector_mean_error));
    CHECK(comparison.mean_messenger_fraction == Catch::Approx(0.75017).margin(0.0000005));
    CHECK(comparison.variance_messenger_fraction ==
          Catch::Approx(0.000847521).margin(0.0000000005));
    CHECK(comparison.mean_effector_fraction == Catch::Approx(0.74938).margin(0.0000005));
    CHECK(comparison.variance_effector_fraction == Catch::Approx(0.000931816).margin(0.0000000005));
    CHECK(comparison.responding_cells == comparison.population_size);
    CHECK(comparison.total_reaction_events > 0);
    CHECK(comparison.total_random_draws > comparison.total_reaction_events);
}

TEST_CASE("Absent receptor activation leaves an initially inactive network silent",
          "[m5][cell][intracellular][zero]") {
    const auto profile = load_profile();
    auto request = profile.reference.request;
    request.receptor_trajectory.front().bound_fraction = 0.0;
    const mehlissa::models::cell::IntracellularOdeModel ode{profile.ode};
    const auto deterministic = ode.evaluate(request);
    CHECK(deterministic.final_active_messenger_fraction == 0.0);
    CHECK(deterministic.final_active_effector_fraction == 0.0);
    CHECK(deterministic.response_threshold_reached == false);

    const mehlissa::models::cell::IntracellularSsaModel ssa{profile.ssa};
    mehlissa::core::RandomStream random{9, "m5.5.zero"};
    const auto stochastic = ssa.evaluate(request, random);
    CHECK(stochastic.reaction_events == 0);
    CHECK(stochastic.random_draws == 0);
    CHECK(stochastic.response_threshold_reached == false);
}

TEST_CASE("Intracellular models reject malformed inputs and exhausted budgets",
          "[m5][cell][intracellular][validation]") {
    const auto profile = load_profile();
    auto malformed = profile.reference.request;
    malformed.receptor_trajectory.front().bound_fraction = 1.1;
    const mehlissa::models::cell::IntracellularOdeModel ode{profile.ode};
    CHECK_THROWS_AS(ode.evaluate(malformed), mehlissa::core::MehlissaError);

    auto bounded = profile.ssa;
    bounded.maximum_reaction_events = 1;
    const mehlissa::models::cell::IntracellularSsaModel ssa{bounded};
    mehlissa::core::RandomStream random{4, "m5.5.bounded"};
    CHECK_THROWS_AS(ssa.evaluate(profile.reference.request, random), mehlissa::core::MehlissaError);

    auto duplicate_source = profile;
    duplicate_source.sources[1].id = duplicate_source.sources[0].id;
    CHECK_THROWS_AS(
        mehlissa::models::cell::validate_intracellular_response_profile(duplicate_source),
        mehlissa::core::MehlissaError);

    auto inconsistent = profile;
    inconsistent.ssa.kinetics.response_threshold_fraction = 0.7;
    CHECK_THROWS_AS(mehlissa::models::cell::validate_intracellular_response_profile(inconsistent),
                    mehlissa::core::MehlissaError);
}

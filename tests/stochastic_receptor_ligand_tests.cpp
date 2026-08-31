// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/stochastic_receptor_ligand_model.hpp>
#include <mehlissa/models/cell/stochastic_receptor_ligand_population.hpp>
#include <mehlissa/models/cell/stochastic_receptor_ligand_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] mehlissa::models::cell::StochasticReceptorLigandProfile load_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_stochastic_receptor_ligand_profile(
        {root / "examples/cell-models/synthetic-stochastic-receptor-ligand-v1.json",
         root / "data/schemas/stochastic-receptor-ligand-profile/1.0.0.schema.json"});
}

} // namespace

TEST_CASE("The stochastic binding profile is strict and evidence scoped",
          "[m5][cell][binding][ssa][schema]") {
    const auto profile = load_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.implementation_kind == mehlissa::models::cell::stochastic_receptor_ligand_kind);
    CHECK(profile.model.receptor_count == 40);
    CHECK(profile.reference.ensemble.cells_per_cohort == 2000);
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 7);
}

TEST_CASE("A named random stream reproduces the finite-receptor trajectory exactly",
          "[m5][cell][binding][ssa][determinism][conservation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::StochasticReceptorLigandModel model{profile.model};
    mehlissa::core::RandomStream first{77, "m5.4.cell.7"};
    mehlissa::core::RandomStream repeat{77, "m5.4.cell.7"};
    const auto lhs = model.evaluate(profile.reference.request.signal_positive, first);
    const auto rhs = model.evaluate(profile.reference.request.signal_positive, repeat);

    CHECK(lhs.bound_receptors == rhs.bound_receptors);
    CHECK(lhs.reaction_events == rhs.reaction_events);
    CHECK(lhs.random_draws == rhs.random_draws);
    CHECK(lhs.samples.size() == rhs.samples.size());
    CHECK(lhs.total_receptors == lhs.free_receptors + lhs.bound_receptors);
    CHECK(lhs.final_bound_fraction ==
          static_cast<double>(lhs.bound_receptors) / lhs.total_receptors);
    CHECK(lhs.peak_bound_fraction >= lhs.final_bound_fraction);
    CHECK(lhs.samples.back().offset == 10s);
}

TEST_CASE("No ligand leaves initially free receptors unbound without detection",
          "[m5][cell][binding][ssa][zero]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::StochasticReceptorLigandModel model{profile.model};
    auto request = profile.reference.request.signal_negative;
    request.ligand_trajectory.front().concentration = mehlissa::core::moles_per_cubic_meter(0.0);
    mehlissa::core::RandomStream random{19, "m5.4.no-signal"};
    const auto response = model.evaluate(request, random);
    CHECK(response.bound_receptors == 0);
    CHECK(response.reaction_events == 0);
    CHECK(response.random_draws == 0);
    CHECK(response.detection_threshold_reached == false);
}

TEST_CASE("The ensemble recovers analytical binomial moments and declared detection gates",
          "[m5][cell][binding][ssa][population][classification]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::StochasticReceptorLigandModel model{profile.model};
    const auto response = mehlissa::models::cell::evaluate_stochastic_population(
        model, profile.reference.ensemble, profile.reference.request);
    const auto& gates = profile.reference;

    CHECK(response.signal_positive.mean_final_bound_fraction ==
          Catch::Approx(gates.expected_positive_mean).margin(gates.mean_absolute_tolerance));
    CHECK(response.signal_negative.mean_final_bound_fraction ==
          Catch::Approx(gates.expected_negative_mean).margin(gates.mean_absolute_tolerance));
    CHECK(
        response.signal_positive.variance_final_bound_fraction ==
        Catch::Approx(gates.expected_positive_variance).margin(gates.variance_absolute_tolerance));
    CHECK(
        response.signal_negative.variance_final_bound_fraction ==
        Catch::Approx(gates.expected_negative_variance).margin(gates.variance_absolute_tolerance));
    CHECK(response.classification.false_negative_rate <= gates.maximum_false_negative_rate);
    CHECK(response.classification.false_positive_rate <= gates.maximum_false_positive_rate);
    CHECK(response.classification.true_positive + response.classification.false_negative ==
          gates.ensemble.cells_per_cohort);
    CHECK(response.classification.true_negative + response.classification.false_positive ==
          gates.ensemble.cells_per_cohort);
}

TEST_CASE("Population reports are invariant under exact replay",
          "[m5][cell][binding][ssa][population][determinism]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::StochasticReceptorLigandModel model{profile.model};
    const auto first = mehlissa::models::cell::evaluate_stochastic_population(
        model, profile.reference.ensemble, profile.reference.request);
    const auto repeat = mehlissa::models::cell::evaluate_stochastic_population(
        model, profile.reference.ensemble, profile.reference.request);
    CHECK(first.signal_positive.mean_final_bound_fraction ==
          repeat.signal_positive.mean_final_bound_fraction);
    CHECK(first.signal_negative.variance_final_bound_fraction ==
          repeat.signal_negative.variance_final_bound_fraction);
    CHECK(first.classification.false_negative == repeat.classification.false_negative);
    CHECK(first.classification.false_positive == repeat.classification.false_positive);
}

TEST_CASE("Stochastic binding rejects malformed trajectories and exhausted event budgets",
          "[m5][cell][binding][ssa][validation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::StochasticReceptorLigandModel model{profile.model};
    auto malformed = profile.reference.request.signal_positive;
    malformed.initial_bound_receptors = profile.model.receptor_count + 1;
    mehlissa::core::RandomStream random{5, "m5.4.invalid"};
    CHECK_THROWS_AS(model.evaluate(malformed, random), mehlissa::core::MehlissaError);

    auto bounded = profile.model;
    bounded.maximum_reaction_events = 1;
    const mehlissa::models::cell::StochasticReceptorLigandModel bounded_model{bounded};
    mehlissa::core::RandomStream bounded_random{5, "m5.4.bounded"};
    CHECK_THROWS_AS(
        bounded_model.evaluate(profile.reference.request.signal_positive, bounded_random),
        mehlissa::core::MehlissaError);

    auto duplicate_source = profile;
    duplicate_source.sources[1].id = duplicate_source.sources[0].id;
    CHECK_THROWS_AS(
        mehlissa::models::cell::validate_stochastic_receptor_ligand_profile(duplicate_source),
        mehlissa::core::MehlissaError);
}

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cosimulation/dynamic_capillary_tissue_cell_model.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

namespace cosim = mehlissa::models::cosimulation;
namespace core = mehlissa::core;

[[nodiscard]] double moles(const core::Amount value) { return core::in_moles(value); }

} // namespace

TEST_CASE("DCCQ coupling owns VEGF-A165a once and preserves the open-system ledger",
          "[dccq][dynamic-coupling][balance]") {
    cosim::DynamicCapillaryTissueCellModel model{cosim::dccq1_reference_parameters(),
                                                 cosim::dccq1_reference_initial_state()};
    const auto samples = model.run(core::seconds(14'400.0));

    REQUIRE(samples.size() == 241);
    CHECK(samples.front().ledger.blood_free == samples.front().ledger.initial_amount);
    CHECK(moles(samples.back().ledger.blood_free) < moles(samples.front().ledger.blood_free));
    CHECK(moles(samples.back().ledger.interstitium_free) > 0.0);
    CHECK(moles(samples.back().ledger.receptor_bound) > 0.0);
    CHECK(moles(samples.back().ledger.internalized) > 0.0);
    CHECK(moles(samples.back().ledger.cleared_or_degraded) > 0.0);
    CHECK(moles(samples.back().ledger.cumulative_outlet) > 0.0);
    CHECK(std::all_of(samples.begin(), samples.end(), [](const auto& sample) {
        return cosim::is_dynamically_balanced(sample.ledger, 1.0e-10);
    }));
}

TEST_CASE("DCCQ coupling rejects identity unit and ownership shortcuts",
          "[dccq][dynamic-coupling][negative-control]") {
    auto parameters = cosim::dccq1_reference_parameters();
    parameters.ligand_id = "oxygen";
    CHECK_THROWS_AS(
        cosim::DynamicCapillaryTissueCellModel(parameters, cosim::dccq1_reference_initial_state()),
        mehlissa::core::MehlissaError);

    parameters = cosim::dccq1_reference_parameters();
    parameters.interstitium_volume = core::cubic_meters(0.0);
    CHECK_THROWS_AS(
        cosim::DynamicCapillaryTissueCellModel(parameters, cosim::dccq1_reference_initial_state()),
        mehlissa::core::MehlissaError);

    auto initial = cosim::dccq1_reference_initial_state();
    initial.receptor_bound = core::moles(1.0e-20);
    CHECK_THROWS_AS(
        cosim::DynamicCapillaryTissueCellModel(cosim::dccq1_reference_parameters(), initial),
        mehlissa::core::MehlissaError);
}

TEST_CASE("DCCQ zero-flux and zero-binding limits are recovered",
          "[dccq][dynamic-coupling][limits]") {
    auto zero_flux = cosim::dccq1_reference_parameters();
    zero_flux.blood_to_endothelium = core::per_second(0.0);
    zero_flux.endothelium_to_blood = core::per_second(0.0);
    zero_flux.endothelium_to_interstitium = core::per_second(0.0);
    zero_flux.interstitium_to_endothelium = core::per_second(0.0);
    zero_flux.blood_outlet = core::per_second(0.0);
    zero_flux.interstitial_clearance = core::per_second(0.0);
    cosim::DynamicCapillaryTissueCellModel isolated{zero_flux,
                                                    cosim::dccq1_reference_initial_state()};
    const auto isolated_result = isolated.run(core::seconds(600.0));
    CHECK(moles(isolated_result.back().ledger.blood_free) ==
          Catch::Approx(moles(isolated_result.front().ledger.blood_free)).epsilon(1.0e-14));

    auto zero_binding = cosim::dccq1_reference_parameters();
    zero_binding.association = core::cubic_meters_per_mole_second(0.0);
    cosim::DynamicCapillaryTissueCellModel unbound{zero_binding,
                                                   cosim::dccq1_reference_initial_state()};
    const auto unbound_result = unbound.run(core::seconds(600.0));
    CHECK(moles(unbound_result.back().ledger.receptor_bound) == 0.0);
    CHECK(moles(unbound_result.back().ledger.internalized) == 0.0);
}

TEST_CASE("DCCQ feedback is bounded and applied only at the following synchronization",
          "[dccq][dynamic-coupling][causality]") {
    auto parameters = cosim::dccq1_reference_parameters();
    parameters.feedback_occupancy_threshold = 0.0;
    cosim::DynamicCapillaryTissueCellModel model{parameters,
                                                 cosim::dccq1_reference_initial_state()};

    const auto first = model.advance_one_synchronization_interval();
    CHECK(first.applied_feedback_multiplier == 1.0);
    CHECK(first.scheduled_feedback_multiplier <= 1.0);
    CHECK(first.scheduled_feedback_multiplier >= parameters.minimum_feedback_multiplier);
    const auto second = model.advance_one_synchronization_interval();
    CHECK(second.applied_feedback_multiplier == Catch::Approx(first.scheduled_feedback_multiplier));
}

TEST_CASE("DCCQ deterministic replay and RK4 refinement agree",
          "[dccq][dynamic-coupling][replay][convergence]") {
    cosim::DynamicCapillaryTissueCellModel first{cosim::dccq1_reference_parameters(),
                                                 cosim::dccq1_reference_initial_state()};
    cosim::DynamicCapillaryTissueCellModel replay{cosim::dccq1_reference_parameters(),
                                                  cosim::dccq1_reference_initial_state()};
    const auto first_result = first.run(core::seconds(3600.0));
    const auto replay_result = replay.run(core::seconds(3600.0));
    CHECK(moles(first_result.back().ledger.interstitium_free) ==
          moles(replay_result.back().ledger.interstitium_free));
    CHECK(moles(first_result.back().ledger.receptor_bound) ==
          moles(replay_result.back().ledger.receptor_bound));

    auto coarse_parameters = cosim::dccq1_reference_parameters();
    coarse_parameters.internal_step = core::seconds(4.0);
    auto fine_parameters = cosim::dccq1_reference_parameters();
    fine_parameters.internal_step = core::seconds(1.0);
    cosim::DynamicCapillaryTissueCellModel coarse{coarse_parameters,
                                                  cosim::dccq1_reference_initial_state()};
    cosim::DynamicCapillaryTissueCellModel fine{fine_parameters,
                                                cosim::dccq1_reference_initial_state()};
    const auto coarse_result = coarse.run(core::seconds(3600.0));
    const auto fine_result = fine.run(core::seconds(3600.0));
    CHECK(moles(coarse_result.back().ledger.receptor_bound) ==
          Catch::Approx(moles(fine_result.back().ledger.receptor_bound)).epsilon(1.0e-7));
}

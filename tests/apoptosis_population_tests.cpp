// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/apoptosis_population.hpp>
#include <mehlissa/models/cell/apoptosis_population_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <limits>

namespace {

[[nodiscard]] mehlissa::models::cell::ApoptosisPopulationProfile load_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_apoptosis_population_profile(
        {root / "examples/cell-models/synthetic-apoptosis-population-v1.json",
         root / "data/schemas/apoptosis-population-profile/1.0.0.schema.json"});
}

} // namespace

TEST_CASE("A strict population profile binds scaling sensitivity evidence and scope",
          "[m5][cell][apoptosis-population][schema]") {
    const auto profile = load_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.implementation_kind ==
          mehlissa::models::cell::cohort_compressed_apoptosis_population_kind);
    CHECK(profile.reference_request.cohorts.size() == 4);
    CHECK(profile.sensitivity_cases.size() == 3);
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("One trillion cells are evaluated exactly through four bounded cohorts",
          "[m5][cell][apoptosis-population][reference]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::CohortCompressedApoptosisPopulationModel model{profile.model};
    const auto response = model.evaluate(profile.reference_request);

    CHECK(model.kind() == mehlissa::models::cell::cohort_compressed_apoptosis_population_kind);
    CHECK(response.evaluated_cohorts == 4);
    CHECK(response.cohort_results.size() == 2);
    CHECK(response.omitted_cohort_results == 2);
    CHECK(response.total_cells == 1'000'000'000'000ULL);
    CHECK(response.viable_cells == 750'000'000'000ULL);
    CHECK(response.apoptosis_committed_cells == 250'000'000'000ULL);
    CHECK(response.apoptosis_committed_fraction == Catch::Approx(0.25).margin(1.0e-15));
    CHECK(response.cell_weighted_mean_effect_fraction == Catch::Approx(0.385).margin(1.0e-15));
}

TEST_CASE("Population fractions are invariant to represented cell-count scale",
          "[m5][cell][apoptosis-population][scaling]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::CohortCompressedApoptosisPopulationModel model{profile.model};
    const auto large = model.evaluate(profile.reference_request);

    auto small_request = profile.reference_request;
    small_request.request_id = "m5-8-small-equivalent";
    for (auto& cohort : small_request.cohorts) {
        cohort.cell_count /= 1'000'000'000ULL;
    }
    const auto small = model.evaluate(small_request);

    CHECK(small.total_cells == 1'000);
    CHECK(small.apoptosis_committed_cells == 250);
    CHECK(small.evaluated_cohorts == large.evaluated_cohorts);
    CHECK(small.apoptosis_committed_fraction ==
          Catch::Approx(large.apoptosis_committed_fraction).margin(1.0e-15));
    CHECK(small.cell_weighted_mean_effect_fraction ==
          Catch::Approx(large.cell_weighted_mean_effect_fraction).margin(1.0e-15));
}

TEST_CASE("Predeclared response sensitivities reproduce exact aggregate references",
          "[m5][cell][apoptosis-population][sensitivity]") {
    const auto profile = load_profile();
    for (const auto& sensitivity : profile.sensitivity_cases) {
        auto config = profile.model;
        config.half_max_effect_amount = sensitivity.half_max_effect_amount;
        config.hill_coefficient = sensitivity.hill_coefficient;
        config.apoptosis_commitment_threshold = sensitivity.apoptosis_commitment_threshold;
        const auto response =
            mehlissa::models::cell::CohortCompressedApoptosisPopulationModel{config}.evaluate(
                profile.reference_request);
        CHECK(response.apoptosis_committed_fraction ==
              Catch::Approx(sensitivity.expected_apoptosis_committed_fraction)
                  .margin(profile.expected.fraction_tolerance));
        CHECK(response.cell_weighted_mean_effect_fraction ==
              Catch::Approx(sensitivity.expected_mean_effect_fraction)
                  .margin(profile.expected.fraction_tolerance));
    }
}

TEST_CASE("Population requests reject ambiguous cohorts limits and count overflow",
          "[m5][cell][apoptosis-population][validation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::CohortCompressedApoptosisPopulationModel model{profile.model};

    auto duplicate = profile.reference_request;
    duplicate.cohorts[1].cohort_id = duplicate.cohorts[0].cohort_id;
    CHECK_THROWS_AS(model.evaluate(duplicate), mehlissa::core::MehlissaError);

    auto unbounded = profile.reference_request;
    unbounded.maximum_reported_cohorts = 0;
    CHECK_THROWS_AS(model.evaluate(unbounded), mehlissa::core::MehlissaError);

    auto overflowing = profile.reference_request;
    overflowing.cohorts = {
        {"near-limit", std::numeric_limits<std::uint64_t>::max(), mehlissa::core::moles(0.0)},
        {"overflow", 1, mehlissa::core::moles(0.0)}};
    CHECK_THROWS_AS(model.evaluate(overflowing), mehlissa::core::MehlissaError);

    CHECK_THROWS_AS(mehlissa::models::cell::synthetic_hill_effect(mehlissa::core::moles(-1.0),
                                                                  mehlissa::core::moles(1.0), 2.0),
                    mehlissa::core::MehlissaError);

    auto changed_reference = profile;
    changed_reference.expected.apoptosis_committed_fraction = 0.3;
    CHECK_THROWS_AS(
        mehlissa::models::cell::validate_apoptosis_population_profile(changed_reference),
        mehlissa::core::MehlissaError);
}

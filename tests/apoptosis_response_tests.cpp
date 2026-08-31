// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/apoptosis_response.hpp>
#include <mehlissa/models/cell/apoptosis_response_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] mehlissa::models::cell::ApoptosisResponseProfile load_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_apoptosis_response_profile(
        {root / "examples/cell-models/synthetic-apoptosis-response-v1.json",
         root / "data/schemas/apoptosis-response-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::cell::DrugDeliveryResponse reference_delivery() {
    return {"m5-6-conservative-delivery",
            "cell.drug-delivery.synthetic.v1",
            "drug.synthetic.1",
            true,
            2'202'913'832ns,
            10s,
            mehlissa::core::moles(1.0e-7),
            mehlissa::core::moles(1.83156388887342e-9),
            mehlissa::core::moles(2.34039288695757e-8),
            mehlissa::core::moles(7.47645072415509e-8),
            mehlissa::core::moles(9.81684361111266e-8),
            mehlissa::core::moles(0.0)};
}

[[nodiscard]] double seconds(const mehlissa::core::SimulationClock::Duration duration) {
    return std::chrono::duration<double>{duration}.count();
}

} // namespace

TEST_CASE("A strict apoptosis profile binds effect state feedback and evidence",
          "[m5][cell][apoptosis][schema]") {
    const auto profile = load_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.implementation_kind == mehlissa::models::cell::synthetic_hill_apoptosis_kind);
    CHECK(profile.model.drug_id == "drug.synthetic.1");
    CHECK(profile.feedback_target.event_id == "m5-7-apoptosis-committed");
    CHECK(profile.reference_case.expected_state ==
          mehlissa::models::cell::CellState::apoptosis_committed);
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("Intracellular drug maps to the checked apoptosis commitment",
          "[m5][cell][apoptosis][reference]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::SyntheticHillApoptosisModel model{profile.model};
    const auto request =
        mehlissa::models::cell::make_apoptosis_reference_request(profile, reference_delivery());
    const auto response = model.evaluate(request);

    CHECK(model.kind() == mehlissa::models::cell::synthetic_hill_apoptosis_kind);
    CHECK(response.state == profile.reference_case.expected_state);
    CHECK(response.effect_fraction == Catch::Approx(profile.reference_case.expected_effect_fraction)
                                          .margin(profile.reference_case.effect_tolerance));
    CHECK(mehlissa::core::in_moles(response.intracellular_drug_amount) ==
          Catch::Approx(
              mehlissa::core::in_moles(profile.reference_case.expected_intracellular_drug_amount))
              .margin(mehlissa::core::in_moles(profile.reference_case.amount_tolerance)));
    CHECK(seconds(response.observed_at) == Catch::Approx(12.202913832).margin(1.0e-9));
    CHECK(response.delivery_activated);
}

TEST_CASE("No activation and subthreshold uptake retain a viable state",
          "[m5][cell][apoptosis][gating]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::SyntheticHillApoptosisModel model{profile.model};

    auto no_activation = reference_delivery();
    no_activation.activated = false;
    no_activation.activation_offset.reset();
    no_activation.intracellular_drug_amount = mehlissa::core::moles(0.0);
    const auto silent = model.evaluate({"m5-7-no-activation", no_activation});
    CHECK(silent.state == mehlissa::models::cell::CellState::viable);
    CHECK(silent.effect_fraction == 0.0);
    CHECK(seconds(silent.observed_at) == Catch::Approx(10.0));

    auto subthreshold = reference_delivery();
    subthreshold.intracellular_drug_amount = mehlissa::core::moles(1.0e-8);
    const auto viable = model.evaluate({"m5-7-subthreshold", subthreshold});
    CHECK(viable.state == mehlissa::models::cell::CellState::viable);
    CHECK(viable.effect_fraction == Catch::Approx(1.0 / 26.0).margin(1.0e-15));
}

TEST_CASE("The Hill response is bounded stable and exactly half maximal",
          "[m5][cell][apoptosis][analytical]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::SyntheticHillApoptosisModel model{profile.model};
    auto delivery = reference_delivery();

    delivery.intracellular_drug_amount = profile.model.half_max_effect_amount;
    CHECK(model.evaluate({"half-max", delivery}).effect_fraction == Catch::Approx(0.5));

    delivery.intracellular_drug_amount = mehlissa::core::moles(1.0e200);
    const auto high = model.evaluate({"high-finite-amount", delivery});
    CHECK(std::isfinite(high.effect_fraction));
    CHECK(high.effect_fraction == Catch::Approx(1.0));
}

TEST_CASE("Apoptosis response rejects identity timing and provenance inconsistencies",
          "[m5][cell][apoptosis][validation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::SyntheticHillApoptosisModel model{profile.model};

    auto wrong_drug = reference_delivery();
    wrong_drug.drug_id = "drug.other";
    CHECK_THROWS_AS(model.evaluate({"wrong-drug", wrong_drug}), mehlissa::core::MehlissaError);

    auto inconsistent_activation = reference_delivery();
    inconsistent_activation.activation_offset.reset();
    CHECK_THROWS_AS(model.evaluate({"inconsistent-activation", inconsistent_activation}),
                    mehlissa::core::MehlissaError);

    auto wrong_reference = reference_delivery();
    wrong_reference.request_id = "delivery.other";
    CHECK_THROWS_AS(
        mehlissa::models::cell::make_apoptosis_reference_request(profile, wrong_reference),
        mehlissa::core::MehlissaError);

    auto duplicate_source = profile;
    duplicate_source.sources[1].id = duplicate_source.sources[0].id;
    CHECK_THROWS_AS(mehlissa::models::cell::validate_apoptosis_response_profile(duplicate_source),
                    mehlissa::core::MehlissaError);

    auto invalid_config = profile.model;
    invalid_config.apoptosis_commitment_threshold = 1.0;
    CHECK_THROWS_AS(mehlissa::models::cell::SyntheticHillApoptosisModel{invalid_config},
                    mehlissa::core::MehlissaError);
}

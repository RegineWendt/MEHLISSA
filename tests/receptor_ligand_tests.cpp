// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/cell/analytical_receptor_ligand_model.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>

namespace {

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] mehlissa::models::cell::ReceptorLigandProfile load_profile() {
    const auto root = project_root();
    return mehlissa::models::cell::load_receptor_ligand_profile({
        root / "examples/cell-models/synthetic-receptor-ligand-v1.json",
        root / "data/schemas/receptor-ligand-profile/1.0.0.schema.json",
    });
}

} // namespace

TEST_CASE("A strict receptor-ligand profile loads with explicit evidence scope",
          "[m5][cell][binding][schema]") {
    const auto profile = load_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-receptor-ligand-v1");
    CHECK(profile.implementation_kind == mehlissa::models::cell::analytical_receptor_ligand_kind);
    CHECK(profile.model.model_id == "cell.receptor-ligand.synthetic.v1");
    CHECK(profile.model.receptor_id == "synthetic-receptor");
    CHECK(profile.model.ligand_id == "synthetic-ligand");
    CHECK(profile.validity.evidence_class == "software_test_surrogate");
    CHECK(profile.sources.size() == 2);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("The analytical binding step matches equilibrium transient threshold and balance",
          "[m5][cell][binding][analytical]") {
    const auto profile = load_profile();
    const auto model = mehlissa::models::cell::make_receptor_ligand_model(profile);
    const auto request = mehlissa::models::cell::make_receptor_ligand_reference_request(profile);
    const auto response = model->evaluate(request);

    CHECK(model->kind() == mehlissa::models::cell::analytical_receptor_ligand_kind);
    CHECK(response.request_id == request.request_id);
    CHECK(response.cell_model_id == profile.model.model_id);
    CHECK(response.receptor_id == profile.model.receptor_id);
    CHECK(response.ligand_id == profile.model.ligand_id);
    CHECK(response.equilibrium_bound_fraction == Catch::Approx(0.75));
    CHECK(response.final_bound_fraction ==
          Catch::Approx(profile.reference_case.expected_final_bound_fraction)
              .margin(profile.reference_case.absolute_tolerance));
    CHECK(mehlissa::core::in_moles(response.total_receptor_amount) == Catch::Approx(1.0e-21));
    CHECK(mehlissa::core::in_moles(response.bound_receptor_amount) ==
          Catch::Approx(7.362632708334493e-22));
    CHECK(mehlissa::models::cell::receptor_balance_error_moles(response) ==
          Catch::Approx(0.0).margin(1.0e-32));
    REQUIRE(response.detection_threshold_reached);
    REQUIRE(response.first_threshold_crossing_time.has_value());
    const auto crossing = response.first_threshold_crossing_time.value_or(
        mehlissa::core::SimulationClock::Duration::min());
    CHECK(std::chrono::duration<double>{crossing}.count() ==
          Catch::Approx(profile.reference_case.expected_threshold_crossing_seconds).margin(1.0e-9));
}

TEST_CASE("Zero-ligand dissociation follows the analytical first-order limit",
          "[m5][cell][binding][dissociation]") {
    const auto profile = load_profile();
    const mehlissa::models::cell::AnalyticalReceptorLigandModel model{profile.model};
    const auto half_life =
        std::log(2.0) / mehlissa::core::in_per_second(profile.model.dissociation_rate);
    const auto response = model.evaluate({
        "dissociation-half-life",
        profile.model.ligand_id,
        profile.model.compartment_id,
        mehlissa::core::moles_per_cubic_meter(0.0),
        std::chrono::duration_cast<mehlissa::core::SimulationClock::Duration>(
            std::chrono::duration<double>{half_life}),
        1.0,
    });

    CHECK(response.equilibrium_bound_fraction == 0.0);
    CHECK(response.final_bound_fraction == Catch::Approx(0.5).margin(1.0e-9));
    REQUIRE(response.first_threshold_crossing_time.has_value());
    CHECK(response.first_threshold_crossing_time.value_or(
              mehlissa::core::SimulationClock::Duration::min()) ==
          mehlissa::core::SimulationClock::Duration::zero());
}

TEST_CASE("Receptor-ligand contracts reject incompatible and nonphysical inputs",
          "[m5][cell][binding][validation]") {
    const auto profile = load_profile();
    const auto model = mehlissa::models::cell::make_receptor_ligand_model(profile);
    auto wrong_ligand = mehlissa::models::cell::make_receptor_ligand_reference_request(profile);
    wrong_ligand.ligand_id = "another-ligand";
    CHECK_THROWS_AS(model->evaluate(wrong_ligand), mehlissa::core::MehlissaError);

    auto negative_concentration =
        mehlissa::models::cell::make_receptor_ligand_reference_request(profile);
    negative_concentration.ligand_concentration = mehlissa::core::moles_per_cubic_meter(-1.0);
    CHECK_THROWS_AS(model->evaluate(negative_concentration), mehlissa::core::MehlissaError);

    auto invalid_threshold = profile;
    invalid_threshold.model.detection_threshold_fraction = 0.0;
    CHECK_THROWS_AS(mehlissa::models::cell::validate_receptor_ligand_profile(invalid_threshold),
                    mehlissa::core::MehlissaError);

    auto duplicate_source = profile;
    duplicate_source.sources.push_back(duplicate_source.sources.front());
    CHECK_THROWS_AS(mehlissa::models::cell::validate_receptor_ligand_profile(duplicate_source),
                    mehlissa::core::MehlissaError);
}

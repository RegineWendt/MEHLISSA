// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/apoptosis_response_profile.hpp>
#include <mehlissa/models/cell/drug_delivery_profile.hpp>
#include <mehlissa/models/cell/intracellular_response_profile.hpp>
#include <mehlissa/models/cosimulation/cell_state_feedback.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] mehlissa::models::cell::IntracellularResponseProfile load_intracellular() {
    const auto root = project_root();
    return mehlissa::models::cell::load_intracellular_response_profile(
        {root / "examples/cell-models/synthetic-intracellular-response-v1.json",
         root / "data/schemas/intracellular-response-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::cell::DrugDeliveryProfile load_delivery() {
    const auto root = project_root();
    return mehlissa::models::cell::load_drug_delivery_profile(
        {root / "examples/cell-models/synthetic-conservative-drug-delivery-v1.json",
         root / "data/schemas/drug-delivery-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::cell::ApoptosisResponseProfile load_apoptosis() {
    const auto root = project_root();
    return mehlissa::models::cell::load_apoptosis_response_profile(
        {root / "examples/cell-models/synthetic-apoptosis-response-v1.json",
         root / "data/schemas/apoptosis-response-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::cosimulation::CellStateFeedbackConfig
feedback_config(const mehlissa::models::cell::ApoptosisResponseProfile& profile) {
    return {profile.feedback_target.event_id, profile.model.model_id, profile.model.cell_id,
            profile.feedback_target.target_model_id, profile.feedback_target.target_port_id};
}

[[nodiscard]] double seconds(const mehlissa::core::SimulationClock::Duration duration) {
    return std::chrono::duration<double>{duration}.count();
}

} // namespace

TEST_CASE("The M5 chain returns apoptosis through a neutral higher-layer boundary",
          "[m5][cosimulation][apoptosis][feedback][end-to-end]") {
    const auto intracellular = load_intracellular();
    const auto delivery = load_delivery();
    const auto apoptosis = load_apoptosis();

    const mehlissa::models::cell::IntracellularOdeModel intracellular_model{intracellular.ode};
    const auto intracellular_response =
        intracellular_model.evaluate(intracellular.reference.request);
    const auto activation = mehlissa::models::cell::make_nanodevice_activation_signal(
        intracellular_response, delivery.reference_case.activation_target);
    REQUIRE(activation.has_value());

    const mehlissa::models::cell::AnalyticalDrugDeliveryModel delivery_model{delivery.model};
    const auto delivery_response =
        delivery_model.evaluate(mehlissa::models::cell::make_drug_delivery_reference_request(
            delivery, activation.value_or(mehlissa::models::cell::NanodeviceActivationSignal{})));
    const mehlissa::models::cell::SyntheticHillApoptosisModel apoptosis_model{apoptosis.model};
    const auto apoptosis_response = apoptosis_model.evaluate(
        mehlissa::models::cell::make_apoptosis_reference_request(apoptosis, delivery_response));
    const auto event = mehlissa::models::cosimulation::make_cell_state_feedback_event(
        apoptosis_response, feedback_config(apoptosis));

    REQUIRE(event.has_value());
    const auto feedback = event.value_or(mehlissa::models::coupling::CellStateEvent{});
    CHECK(feedback.contract_version ==
          mehlissa::models::coupling::cell_state_event_contract_version);
    CHECK(feedback.event_type == mehlissa::models::coupling::apoptosis_committed_event_type);
    CHECK(feedback.source_model_id == apoptosis.model.model_id);
    CHECK(feedback.source_cell_id == apoptosis.model.cell_id);
    CHECK(feedback.target_model_id == apoptosis.feedback_target.target_model_id);
    CHECK(feedback.target_port_id == apoptosis.feedback_target.target_port_id);
    CHECK(feedback.measure_name == "synthetic_effect_fraction");
    CHECK(feedback.measure_value == Catch::Approx(apoptosis.reference_case.expected_effect_fraction)
                                        .margin(apoptosis.reference_case.effect_tolerance));
    CHECK(seconds(feedback.occurred_at) == Catch::Approx(12.202913832).margin(1.0e-9));
}

TEST_CASE("Viable responses stay silent at the higher-layer event boundary",
          "[m5][cosimulation][apoptosis][feedback][gating]") {
    const auto profile = load_apoptosis();
    mehlissa::models::cell::ApoptosisResponse viable{"viable-request",
                                                     profile.model.model_id,
                                                     profile.model.cell_id,
                                                     profile.model.drug_id,
                                                     "delivery-request",
                                                     "delivery-model",
                                                     false,
                                                     std::chrono::seconds{10},
                                                     mehlissa::core::moles(0.0),
                                                     0.0,
                                                     mehlissa::models::cell::CellState::viable};
    CHECK_FALSE(mehlissa::models::cosimulation::make_cell_state_feedback_event(
                    viable, feedback_config(profile))
                    .has_value());

    auto wrong_source = feedback_config(profile);
    wrong_source.source_cell_id = "cell.other";
    CHECK_THROWS_AS(
        mehlissa::models::cosimulation::make_cell_state_feedback_event(viable, wrong_source),
        mehlissa::core::MehlissaError);
}

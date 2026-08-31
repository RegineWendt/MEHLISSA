// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/drug_delivery.hpp>
#include <mehlissa/models/cell/drug_delivery_profile.hpp>
#include <mehlissa/models/cell/intracellular_response_network.hpp>
#include <mehlissa/models/cell/intracellular_response_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>

namespace {

using namespace std::chrono_literals;
using mehlissa::core::in_moles;

[[nodiscard]] mehlissa::models::cell::IntracellularResponseProfile load_intracellular_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_intracellular_response_profile(
        {root / "examples/cell-models/synthetic-intracellular-response-v1.json",
         root / "data/schemas/intracellular-response-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::cell::DrugDeliveryProfile load_delivery_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::cell::load_drug_delivery_profile(
        {root / "examples/cell-models/synthetic-conservative-drug-delivery-v1.json",
         root / "data/schemas/drug-delivery-profile/1.0.0.schema.json"});
}

[[nodiscard]] std::optional<mehlissa::models::cell::NanodeviceActivationSignal>
run_activation(const mehlissa::models::cell::IntracellularResponseProfile& intracellular,
               const mehlissa::models::cell::DrugDeliveryProfile& delivery) {
    const mehlissa::models::cell::IntracellularOdeModel network{intracellular.ode};
    const auto response = network.evaluate(intracellular.reference.request);
    return mehlissa::models::cell::make_nanodevice_activation_signal(
        response, delivery.reference_case.activation_target);
}

[[nodiscard]] double seconds(const mehlissa::core::SimulationClock::Duration duration) {
    return std::chrono::duration<double>{duration}.count();
}

} // namespace

TEST_CASE("A strict drug-delivery profile binds activation release uptake and evidence",
          "[m5][cell][drug-delivery][schema]") {
    const auto profile = load_delivery_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.implementation_kind ==
          mehlissa::models::cell::analytical_conservative_drug_delivery_kind);
    CHECK(profile.model.nanodevice_id == profile.reference_case.activation_target.nanodevice_id);
    CHECK(profile.model.payload_id == profile.reference_case.activation_target.payload_id);
    CHECK(in_moles(profile.model.loaded_amount) == Catch::Approx(1.0e-7));
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("An intracellular response activates conservative release and cellular uptake",
          "[m5][cell][drug-delivery][activation][conservation][reference]") {
    const auto intracellular = load_intracellular_profile();
    const auto delivery = load_delivery_profile();
    const auto activation = run_activation(intracellular, delivery);
    REQUIRE(activation.has_value());
    const auto activation_signal =
        activation.value_or(mehlissa::models::cell::NanodeviceActivationSignal{});
    CHECK(activation_signal.source_request_id == delivery.reference_case.source_request_id);
    CHECK(activation_signal.source_network_id == delivery.reference_case.source_network_id);
    CHECK(seconds(activation_signal.trigger_offset) ==
          Catch::Approx(seconds(delivery.reference_case.expected_activation_offset))
              .margin(delivery.reference_case.activation_time_tolerance_seconds));

    const mehlissa::models::cell::AnalyticalDrugDeliveryModel model{delivery.model};
    const auto request =
        mehlissa::models::cell::make_drug_delivery_reference_request(delivery, activation_signal);
    const auto response = model.evaluate(request);
    CHECK(model.kind() == mehlissa::models::cell::analytical_conservative_drug_delivery_kind);
    CHECK(response.activated);
    CHECK(in_moles(response.device_payload_amount) ==
          Catch::Approx(in_moles(delivery.reference_case.expected_device_payload_amount))
              .margin(in_moles(delivery.reference_case.amount_tolerance)));
    CHECK(in_moles(response.extracellular_drug_amount) ==
          Catch::Approx(in_moles(delivery.reference_case.expected_extracellular_drug_amount))
              .margin(in_moles(delivery.reference_case.amount_tolerance)));
    CHECK(in_moles(response.intracellular_drug_amount) ==
          Catch::Approx(in_moles(delivery.reference_case.expected_intracellular_drug_amount))
              .margin(in_moles(delivery.reference_case.amount_tolerance)));
    CHECK(in_moles(response.device_payload_amount + response.extracellular_drug_amount +
                   response.intracellular_drug_amount) ==
          Catch::Approx(in_moles(response.initial_payload_amount)).margin(1.0e-20));
    CHECK(in_moles(response.released_drug_amount) ==
          Catch::Approx(
              in_moles(response.extracellular_drug_amount + response.intracellular_drug_amount))
              .margin(1.0e-20));
    CHECK(in_moles(response.balance_error) == Catch::Approx(0.0).margin(1.0e-20));

    const mehlissa::models::cell::IntracellularSsaModel stochastic_network{intracellular.ssa};
    mehlissa::core::RandomStream random{31, "m5.6.ssa-adapter"};
    const auto stochastic_response =
        stochastic_network.evaluate(intracellular.reference.request, random);
    const auto stochastic_activation = mehlissa::models::cell::make_nanodevice_activation_signal(
        stochastic_response, delivery.reference_case.activation_target);
    REQUIRE(stochastic_activation.has_value());
    CHECK(stochastic_activation.value_or(mehlissa::models::cell::NanodeviceActivationSignal{})
              .source_network_id == activation_signal.source_network_id);
}

TEST_CASE("A silent intracellular network leaves the payload sealed",
          "[m5][cell][drug-delivery][no-activation]") {
    auto intracellular = load_intracellular_profile();
    const auto delivery = load_delivery_profile();
    intracellular.reference.request.receptor_trajectory.front().bound_fraction = 0.0;
    const mehlissa::models::cell::IntracellularOdeModel network{intracellular.ode};
    const auto network_response = network.evaluate(intracellular.reference.request);
    const auto activation = mehlissa::models::cell::make_nanodevice_activation_signal(
        network_response, delivery.reference_case.activation_target);
    CHECK_FALSE(activation.has_value());

    const mehlissa::models::cell::AnalyticalDrugDeliveryModel model{delivery.model};
    const auto response =
        model.evaluate({delivery.reference_case.request_id,
                        delivery.reference_case.observation_after_activation, activation});
    CHECK_FALSE(response.activated);
    CHECK(response.device_payload_amount == response.initial_payload_amount);
    CHECK(in_moles(response.extracellular_drug_amount) == 0.0);
    CHECK(in_moles(response.intracellular_drug_amount) == 0.0);
    CHECK(in_moles(response.released_drug_amount) == 0.0);
}

TEST_CASE("Equal release and uptake rates retain the analytical limiting solution",
          "[m5][cell][drug-delivery][analytical-limit]") {
    auto profile = load_delivery_profile();
    profile.model.release_rate = mehlissa::core::per_second(0.3);
    profile.model.uptake_rate = mehlissa::core::per_second(0.3);
    const mehlissa::models::cell::AnalyticalDrugDeliveryModel model{profile.model};
    const auto activation = run_activation(load_intracellular_profile(), profile);
    REQUIRE(activation.has_value());
    const auto response =
        model.evaluate({"equal-rate-limit", 10s,
                        activation.value_or(mehlissa::models::cell::NanodeviceActivationSignal{})});
    const auto expected_external = 1.0e-7 * 0.3 * 10.0 * std::exp(-3.0);
    CHECK(in_moles(response.extracellular_drug_amount) ==
          Catch::Approx(expected_external).margin(1.0e-18));
    CHECK(in_moles(response.device_payload_amount + response.extracellular_drug_amount +
                   response.intracellular_drug_amount) == Catch::Approx(1.0e-7).margin(1.0e-20));
}

TEST_CASE("Drug delivery rejects inconsistent events identities and provenance",
          "[m5][cell][drug-delivery][validation]") {
    const auto intracellular = load_intracellular_profile();
    const auto profile = load_delivery_profile();
    const auto activation = run_activation(intracellular, profile);
    REQUIRE(activation.has_value());
    const auto activation_signal =
        activation.value_or(mehlissa::models::cell::NanodeviceActivationSignal{});

    auto wrong_identity = activation_signal;
    wrong_identity.payload_id = "payload.other";
    const mehlissa::models::cell::AnalyticalDrugDeliveryModel model{profile.model};
    CHECK_THROWS_AS(model.evaluate({"wrong-payload", 10s, wrong_identity}),
                    mehlissa::core::MehlissaError);
    CHECK_THROWS_AS(
        mehlissa::models::cell::make_drug_delivery_reference_request(profile, wrong_identity),
        mehlissa::core::MehlissaError);

    auto wrong_time = activation_signal;
    wrong_time.trigger_offset += 1s;
    CHECK_THROWS_AS(
        mehlissa::models::cell::make_drug_delivery_reference_request(profile, wrong_time),
        mehlissa::core::MehlissaError);

    auto duplicate_source = profile;
    duplicate_source.sources[1].id = duplicate_source.sources[0].id;
    CHECK_THROWS_AS(mehlissa::models::cell::validate_drug_delivery_profile(duplicate_source),
                    mehlissa::core::MehlissaError);

    const mehlissa::models::cell::NanodeviceActivationSignal invalid_signal{
        "0.0.0", "activation", "device", "payload", "request", "network", 0s};
    CHECK_THROWS_AS(mehlissa::models::cell::validate_nanodevice_activation_signal(invalid_signal),
                    mehlissa::core::MehlissaError);

    auto inconsistent_response =
        mehlissa::models::cell::IntracellularOdeModel{intracellular.ode}.evaluate(
            intracellular.reference.request);
    inconsistent_response.first_response_time.reset();
    CHECK_THROWS_AS(mehlissa::models::cell::make_nanodevice_activation_signal(
                        inconsistent_response, profile.reference_case.activation_target),
                    mehlissa::core::MehlissaError);
}

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cosimulation/receptor_detection_adapter.hpp>
#include <mehlissa/models/iot/local_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] mehlissa::models::cell::ReceptorLigandProfile load_receptor_profile() {
    const auto root = project_root();
    return mehlissa::models::cell::load_receptor_ligand_profile(
        {root / "examples/cell-models/synthetic-receptor-ligand-v1.json",
         root / "data/schemas/receptor-ligand-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::LocalCommunicationProfile load_communication_profile() {
    const auto root = project_root();
    return mehlissa::models::iot::load_local_communication_profile(
        {root / "examples/iot-models/synthetic-local-communication-v1.json",
         root / "data/schemas/local-communication-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::Nanodevice load_device(const std::string_view file_name) {
    const auto root = project_root();
    const auto profile = mehlissa::models::iot::load_nanodevice_profile(
        {root / "examples/iot-models" / file_name,
         root / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
    return mehlissa::models::iot::Nanodevice{profile.device};
}

} // namespace

TEST_CASE("An M5 receptor detection reaches the M6 collector over one explicit link",
          "[m5][m6][cosimulation][detection][link][reference]") {
    const auto receptor_profile = load_receptor_profile();
    const auto communication_profile = load_communication_profile();
    const auto receptor_model =
        mehlissa::models::cell::make_receptor_ligand_model(receptor_profile);
    const auto response = receptor_model->evaluate(
        mehlissa::models::cell::make_receptor_ligand_reference_request(receptor_profile));

    const auto event = mehlissa::models::cosimulation::make_receptor_detection_event(
        response, {communication_profile.reference_case.event_id,
                   communication_profile.detection_adapter.source_device_id});
    const auto request = mehlissa::models::iot::make_detection_message_request(
        communication_profile.detection_adapter, event);
    auto locator = load_device("synthetic-locator-v1.json");
    auto collector = load_device("synthetic-collector-v1.json");
    mehlissa::models::iot::ScheduledOneHopLink link{communication_profile.link};
    mehlissa::models::iot::OneHopCommunicationSession session{link};

    const auto exchange = session.exchange(locator, collector, request);

    REQUIRE(response.first_threshold_crossing_time.has_value());
    CHECK(event.detected_at == response.first_threshold_crossing_time.value_or(
                                   mehlissa::core::SimulationClock::Duration::min()));
    CHECK(event.source_model_id == response.cell_model_id);
    CHECK(event.source_request_id == response.request_id);
    CHECK(event.signal_id == response.ligand_id);
    CHECK(exchange.message.source_event_id == event.event_id);
    CHECK(exchange.message.content.find(response.cell_model_id) != std::string::npos);
    CHECK(exchange.message.content.find(response.request_id) != std::string::npos);
    CHECK(exchange.message.content.find(response.compartment_id) != std::string::npos);
    CHECK(exchange.message.correlation_id ==
          communication_profile.detection_adapter.correlation_id);
    CHECK(exchange.transmission.status == mehlissa::models::iot::OneHopDeliveryStatus::delivered);
    CHECK(exchange.transmission.completed_at ==
          event.detected_at + communication_profile.reference_case.expected_latency);
    CHECK(exchange.transmission.link_energy ==
          communication_profile.reference_case.expected_link_energy);
    CHECK(session.metrics().attempted_messages == 1);
    CHECK(session.metrics().delivered_messages == 1);
    CHECK(mehlissa::models::iot::delivery_fraction(session.metrics()) == 1.0);
    CHECK(collector.reception_count() == 1);

    const auto received = collector.take_received_messages();
    REQUIRE(received.size() == 1);
    CHECK(received.front() == exchange.message);
}

TEST_CASE("The receptor adapter rejects absent or acausal detection",
          "[m5][m6][cosimulation][detection][validation]") {
    const auto receptor_profile = load_receptor_profile();
    const auto receptor_model =
        mehlissa::models::cell::make_receptor_ligand_model(receptor_profile);
    auto response = receptor_model->evaluate(
        mehlissa::models::cell::make_receptor_ligand_reference_request(receptor_profile));

    response.detection_threshold_reached = false;
    response.first_threshold_crossing_time.reset();
    CHECK_THROWS_AS(mehlissa::models::cosimulation::make_receptor_detection_event(
                        response, {"detection.absent", "locator.synthetic.1"}),
                    mehlissa::core::MehlissaError);

    response.detection_threshold_reached = true;
    response.first_threshold_crossing_time =
        response.observation_time + std::chrono::nanoseconds{1};
    CHECK_THROWS_AS(mehlissa::models::cosimulation::make_receptor_detection_event(
                        response, {"detection.acausal", "locator.synthetic.1"}),
                    mehlissa::core::MehlissaError);
}

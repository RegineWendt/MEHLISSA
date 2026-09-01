// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/active_gateway_profile.hpp>
#include <mehlissa/models/iot/ban_station.hpp>
#include <mehlissa/models/iot/ban_station_profile.hpp>
#include <mehlissa/models/iot/cluster_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <string>

namespace {

using namespace std::chrono_literals;
using namespace mehlissa::models::iot;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] NanodeviceProfile load_device(const std::string_view file_name) {
    return load_nanodevice_profile({root() / "examples/iot-models" / file_name,
                                    root() / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
}

[[nodiscard]] ActiveGatewayProfile load_gateway_profile() {
    return load_active_gateway_profile(
        {root() / "examples/iot-models/synthetic-active-gateway-v1.json",
         root() / "data/schemas/active-gateway-profile/1.0.0.schema.json"});
}

[[nodiscard]] ClusterCommunicationProfile load_cluster_profile() {
    return load_cluster_communication_profile(
        {root() / "examples/iot-models/synthetic-gateway-cluster-v1.json",
         root() / "data/schemas/cluster-communication-profile/1.0.0.schema.json"});
}

[[nodiscard]] BanStationProfile load_ban_station_profile_data() {
    return load_ban_station_profile(
        {root() / "examples/iot-models/synthetic-ban-station-v1.json",
         root() / "data/schemas/ban-station-profile/1.0.0.schema.json"});
}

[[nodiscard]] GatewayMeasurement measurement() {
    return {std::string{gateway_measurement_contract_version},
            "measurement.m6-5.lung.1",
            "gateway.synthetic.wrist",
            "message.m6-4.measurement.1",
            "collector.synthetic.uplink.1",
            "experiment.m6-5.closed-loop",
            "detection.m6-2.receptor-threshold.1",
            120ms,
            256,
            "application/vnd.mehlissa.measurement+json;version=1.0.0",
            R"({"tissue_id":"lung.tissue.synthetic","detected":true})"};
}

[[nodiscard]] LocalMessageRequest local_measurement_request() {
    return {"lung.1",
            LocalMessageKind::measurement,
            "gateway.synthetic.wrist",
            "experiment.m6-5.closed-loop",
            "detection.m6-2.receptor-threshold.1",
            100ms,
            2s,
            3,
            256,
            "application/vnd.mehlissa.measurement+json;version=1.0.0",
            R"({"tissue_id":"lung.tissue.synthetic","detected":true})"};
}

[[nodiscard]] GatewayCommand command(const std::string& id = "command.m6-5.activate.1") {
    return {std::string{gateway_command_contract_version},
            id,
            "actuator.synthetic.1",
            "experiment.m6-5.closed-loop",
            150ms,
            1s,
            3,
            128,
            "application/vnd.mehlissa.control+json;version=1.0.0",
            R"({"action":"activate","payload_id":"payload.synthetic.1"})"};
}

[[nodiscard]] GatewayBanAdapterConfig gateway_adapter_config() {
    return {"adapter.synthetic.gateway-ban",
            "gateway.synthetic.wrist",
            "station.synthetic.analysis-control",
            "ban-frame.m6-5.measurement",
            4,
            4};
}

[[nodiscard]] ExternalStationConfig station_config() {
    return {"station.synthetic.analysis-control",
            "decision.m6-5",
            "ban-frame.m6-5.command",
            {"gateway.synthetic.wrist"},
            {"actuator.synthetic.1"},
            {"application/vnd.mehlissa.control+json;version=1.0.0"},
            4,
            4};
}

[[nodiscard]] ScheduledBanTransportConfig uplink_config() {
    return {"ban.synthetic.gateway-station", 10ms,
            mehlissa::core::joules(2.0e-6),  mehlissa::core::joules(1.0e-6),
            mehlissa::core::joules(0.5e-6),  {ScheduledLinkOutcome::delivered}};
}

[[nodiscard]] ScheduledBanTransportConfig downlink_config() {
    return {"ban.synthetic.station-gateway", 15ms,
            mehlissa::core::joules(1.5e-6),  mehlissa::core::joules(0.75e-6),
            mehlissa::core::joules(0.4e-6),  {ScheduledLinkOutcome::delivered}};
}

} // namespace

TEST_CASE("Strict BAN and station profile composes both directions", "[m6][iot][ban][schema]") {
    const auto profile = load_ban_station_profile_data();
    CHECK(profile.gateway_adapter.gateway_id == "gateway.synthetic.wrist");
    CHECK(profile.station.station_id == "station.synthetic.analysis-control");
    CHECK(profile.reference_case.target_device_id == "actuator.synthetic.1");
    CHECK(profile.reference_case.expected_uplink_latency == 10ms);
    CHECK(profile.reference_case.expected_downlink_latency == 15ms);
    CHECK(profile.uplink_transport.repeating_outcomes.front() == ScheduledLinkOutcome::delivered);
}

TEST_CASE("BAN adapters close a governed station-to-device path", "[m6][iot][ban][closed-loop]") {
    const auto profile = load_ban_station_profile_data();
    auto active_gateway_profile = load_gateway_profile();
    active_gateway_profile.gateway.measurement_id_prefix = "measurement.m6-5";
    const auto endpoint_profile = load_device("synthetic-gateway-endpoint-v1.json");
    ActiveGateway gateway{active_gateway_profile.gateway, endpoint_profile.device,
                          endpoint_profile.profile_id};
    auto collector = Nanodevice{load_device("synthetic-uplink-collector-v1.json").device};
    auto relay = Nanodevice{load_device("synthetic-relay-v1.json").device};
    auto actuator = Nanodevice{load_device("synthetic-actuator-v1.json").device};
    ClusterDeviceMap devices{{std::string{collector.device_id()}, collector},
                             {std::string{gateway.gateway_id()}, gateway.endpoint()},
                             {std::string{relay.device_id()}, relay},
                             {std::string{actuator.device_id()}, actuator}};
    NanodeviceCluster cluster{load_cluster_profile().cluster};
    BoundedMultiHopSession local_session{cluster};

    const auto local_uplink =
        local_session.exchange(devices, collector.device_id(), gateway.gateway_id(),
                               local_measurement_request(), ClusterRouteStrategy::fewest_hops);
    REQUIRE(local_uplink.status == OneHopDeliveryStatus::delivered);
    const auto published_measurement = gateway.publish_next_measurement(120ms);
    CHECK(published_measurement.measurement_id == profile.reference_case.measurement_id);

    GatewayBanAdapter gateway_adapter{profile.gateway_adapter};
    ExternalAnalysisControlStation station{profile.station};
    ScheduledBanTransportAdapter uplink_transport{profile.uplink_transport};
    ScheduledBanTransportAdapter downlink_transport{profile.downlink_transport};
    BanCommunicationSession uplink{uplink_transport};
    BanCommunicationSession downlink{downlink_transport};

    const auto measurement_frame = gateway_adapter.publish_measurement(published_measurement, 1s);
    const auto uplink_result = uplink.exchange(measurement_frame);
    REQUIRE(uplink_result.status == OneHopDeliveryStatus::delivered);
    station.receive_measurement(measurement_frame, uplink_result.completed_at);

    const auto decision = station.evaluate_command(
        {"request.activate.1", profile.reference_case.measurement_id, command()});
    REQUIRE(decision.status == CommandGovernanceStatus::approved);
    REQUIRE(decision.approved_command.has_value());
    const auto approved_command = decision.approved_command.value_or(GovernedGatewayCommand{});
    CHECK(approved_command.source_measurement_id == "measurement.m6-5.lung.1");
    CHECK(approved_command.station_id == "station.synthetic.analysis-control");

    const auto command_frame = station.prepare_command_frame(decision);
    const auto downlink_result = downlink.exchange(command_frame);
    REQUIRE(downlink_result.status == OneHopDeliveryStatus::delivered);
    const auto accepted_command =
        gateway_adapter.accept_command(command_frame, downlink_result.completed_at);
    CHECK(accepted_command.created_at == 165ms);
    CHECK(accepted_command.valid_for == 985ms);

    const auto local_result = local_session.exchange(
        devices, gateway.gateway_id(), actuator.device_id(),
        gateway.prepare_downlink(accepted_command), ClusterRouteStrategy::lowest_total_latency);

    REQUIRE(local_result.status == OneHopDeliveryStatus::delivered);
    CHECK(local_result.hops.size() == 2);
    CHECK(local_result.metrics.total_delivery_latency == 20ms);
    CHECK(local_result.hops.front().transmission.departed_at == 165ms);
    REQUIRE(actuator.received_messages().size() == 1);
    CHECK(actuator.received_messages().front().source_event_id == "command.m6-5.activate.1");
    CHECK(gateway_adapter.uplink_count() == 1);
    CHECK(gateway_adapter.downlink_count() == 1);
    CHECK(station.measurement_count() == 1);
    CHECK(station.approved_command_count() == 1);
    CHECK(uplink.metrics().total_delivery_latency == 10ms);
    CHECK(downlink.metrics().total_delivery_latency == 15ms);
    CHECK(mehlissa::core::in_joules(uplink.metrics().transmitter_energy) == Catch::Approx(2.0e-6));
    CHECK(mehlissa::core::in_joules(downlink.metrics().receiver_energy) == Catch::Approx(0.75e-6));
}

TEST_CASE("Station policy denies unobserved mismatched and duplicate command requests",
          "[m6][iot][ban][governance]") {
    GatewayBanAdapter gateway_adapter{gateway_adapter_config()};
    ExternalAnalysisControlStation station{station_config()};

    const auto unknown = station.evaluate_command(
        {"request.unknown", "measurement.missing", command("command.unknown")});
    CHECK(unknown.status == CommandGovernanceStatus::denied_unknown_measurement);
    CHECK_FALSE(unknown.approved_command);
    CHECK_THROWS_AS(station.prepare_command_frame(unknown), mehlissa::core::MehlissaError);

    const auto frame = gateway_adapter.publish_measurement(measurement(), 1s);
    station.receive_measurement(frame, 130ms);

    auto acausal = command("command.acausal");
    acausal.created_at = 125ms;
    const auto time_decision =
        station.evaluate_command({"request.acausal", "measurement.m6-5.lung.1", acausal});
    CHECK(time_decision.status == CommandGovernanceStatus::denied_time_order);

    auto wrong_target = command("command.wrong-target");
    wrong_target.target_device_id = "actuator.not-allowed";
    const auto target_decision =
        station.evaluate_command({"request.wrong-target", "measurement.m6-5.lung.1", wrong_target});
    CHECK(target_decision.status == CommandGovernanceStatus::denied_target);

    auto wrong_correlation = command("command.wrong-correlation");
    wrong_correlation.correlation_id = "experiment.unrelated";
    const auto correlation_decision = station.evaluate_command(
        {"request.wrong-correlation", "measurement.m6-5.lung.1", wrong_correlation});
    CHECK(correlation_decision.status == CommandGovernanceStatus::denied_correlation_mismatch);

    const StationCommandRequest approved_request{"request.approved", "measurement.m6-5.lung.1",
                                                 command("command.approved")};
    const auto approved = station.evaluate_command(approved_request);
    REQUIRE(approved.status == CommandGovernanceStatus::approved);
    const auto duplicate = station.evaluate_command(approved_request);
    CHECK(duplicate.status == CommandGovernanceStatus::denied_duplicate_request);

    const auto command_frame = station.prepare_command_frame(approved);
    static_cast<void>(gateway_adapter.accept_command(command_frame, 160ms));
    CHECK_THROWS_AS(gateway_adapter.accept_command(command_frame, 160ms),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Scheduled BAN adapter reports prescribed drops and expiry", "[m6][iot][ban][metrics]") {
    GatewayBanAdapter gateway_adapter{gateway_adapter_config()};
    auto config = uplink_config();
    config.repeating_outcomes = {ScheduledLinkOutcome::lost, ScheduledLinkOutcome::corrupted,
                                 ScheduledLinkOutcome::delivered};
    ScheduledBanTransportAdapter transport{config};
    BanCommunicationSession session{transport};

    auto first = measurement();
    const auto lost = session.exchange(gateway_adapter.publish_measurement(first, 1s));
    CHECK(lost.drop_reason == OneHopDropReason::loss);

    first.measurement_id = "measurement.m6-5.lung.2";
    first.source_message_id = "message.m6-4.measurement.2";
    const auto corrupted = session.exchange(gateway_adapter.publish_measurement(first, 1s));
    CHECK(corrupted.drop_reason == OneHopDropReason::corruption);

    first.measurement_id = "measurement.m6-5.lung.3";
    first.source_message_id = "message.m6-4.measurement.3";
    const auto expired = session.exchange(gateway_adapter.publish_measurement(first, 5ms));
    CHECK(expired.drop_reason == OneHopDropReason::expired);
    CHECK(session.metrics().attempted_messages == 3);
    CHECK(session.metrics().lost_messages == 1);
    CHECK(session.metrics().corrupted_messages == 1);
    CHECK(session.metrics().expired_messages == 1);
    CHECK(session.metrics().delivered_messages == 0);
}

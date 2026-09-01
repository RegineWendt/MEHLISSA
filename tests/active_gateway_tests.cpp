// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/active_gateway_profile.hpp>
#include <mehlissa/models/iot/cluster_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <ranges>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::iot::ActiveGateway;
using mehlissa::models::iot::BoundedMultiHopSession;
using mehlissa::models::iot::ClusterDeviceMap;
using mehlissa::models::iot::ClusterRouteStrategy;
using mehlissa::models::iot::GatewayCommand;
using mehlissa::models::iot::LocalMessageKind;
using mehlissa::models::iot::Nanodevice;
using mehlissa::models::iot::NanodeviceCluster;
using mehlissa::models::iot::NanodeviceProfile;
using mehlissa::models::iot::OneHopDeliveryStatus;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] NanodeviceProfile load_device(const std::string_view file_name) {
    return mehlissa::models::iot::load_nanodevice_profile(
        {root() / "examples/iot-models" / file_name,
         root() / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::ActiveGatewayProfile load_gateway() {
    return mehlissa::models::iot::load_active_gateway_profile(
        {root() / "examples/iot-models/synthetic-active-gateway-v1.json",
         root() / "data/schemas/active-gateway-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::ClusterCommunicationProfile load_cluster() {
    return mehlissa::models::iot::load_cluster_communication_profile(
        {root() / "examples/iot-models/synthetic-gateway-cluster-v1.json",
         root() / "data/schemas/cluster-communication-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::LocalMessageRequest measurement_request() {
    return {"message.m6-4.measurement.1",
            LocalMessageKind::measurement,
            "gateway.synthetic.wrist",
            "experiment.m6-4.gateway-roundtrip",
            "detection.m6-2.receptor-threshold.1",
            100ms,
            2s,
            3,
            256,
            "application/vnd.mehlissa.measurement+json;version=1.0.0",
            R"({"tissue_id":"lung.tissue.synthetic","detected":true})"};
}

[[nodiscard]] GatewayCommand command(const std::string& command_id = "command.m6-4.activate.1") {
    return {std::string{mehlissa::models::iot::gateway_command_contract_version},
            command_id,
            "actuator.synthetic.1",
            "experiment.m6-4.gateway-roundtrip",
            200ms,
            1s,
            3,
            128,
            "application/vnd.mehlissa.control+json;version=1.0.0",
            R"({"action":"activate","payload_id":"payload.synthetic.1"})"};
}

struct GatewayDevices final {
    mehlissa::models::iot::ActiveGatewayProfile gateway_profile{load_gateway()};
    NanodeviceProfile gateway_endpoint_profile{load_device("synthetic-gateway-endpoint-v1.json")};
    ActiveGateway gateway{gateway_profile.gateway, gateway_endpoint_profile.device,
                          gateway_endpoint_profile.profile_id};
    Nanodevice collector{load_device("synthetic-uplink-collector-v1.json").device};
    Nanodevice relay{load_device("synthetic-relay-v1.json").device};
    Nanodevice actuator{load_device("synthetic-actuator-v1.json").device};

    [[nodiscard]] ClusterDeviceMap map() {
        return {{std::string{collector.device_id()}, collector},
                {std::string{gateway.endpoint().device_id()}, gateway.endpoint()},
                {std::string{relay.device_id()}, relay},
                {std::string{actuator.device_id()}, actuator}};
    }
};

} // namespace

TEST_CASE("Strict active-gateway and gateway-cluster profiles compose",
          "[m6][iot][gateway][schema]") {
    const auto gateway = load_gateway();
    const auto cluster_profile = load_cluster();
    NanodeviceCluster cluster{cluster_profile.cluster};
    const auto downlink = cluster.select_route(gateway.gateway.gateway_id,
                                               gateway.reference_case.downlink_target_device_id,
                                               ClusterRouteStrategy::lowest_total_latency);

    CHECK(gateway.gateway.endpoint_profile_id == "synthetic-gateway-endpoint-v1");
    CHECK(gateway.gateway.accepted_uplink_kinds.size() == 2);
    CHECK(downlink.device_ids == std::vector<std::string>{"gateway.synthetic.wrist",
                                                          "relay.synthetic.1",
                                                          "actuator.synthetic.1"});
    CHECK(downlink.total_configured_latency == 20ms);
}

TEST_CASE("An active gateway publishes a measurement and routes a command downlink",
          "[m6][iot][gateway][roundtrip]") {
    GatewayDevices devices;
    NanodeviceCluster cluster{load_cluster().cluster};
    BoundedMultiHopSession session{cluster};

    const auto uplink =
        session.exchange(devices.map(), devices.collector.device_id(), devices.gateway.gateway_id(),
                         measurement_request(), ClusterRouteStrategy::fewest_hops);
    REQUIRE(uplink.status == OneHopDeliveryStatus::delivered);
    REQUIRE(uplink.hops.size() == 1);
    CHECK(uplink.metrics.total_delivery_latency == 20ms);
    CHECK(mehlissa::core::in_joules(uplink.metrics.transmitter_energy) == Catch::Approx(0.35e-6));
    CHECK(mehlissa::core::in_joules(uplink.metrics.receiver_energy) == Catch::Approx(0.4e-6));
    CHECK(mehlissa::core::in_joules(uplink.metrics.link_energy) == Catch::Approx(0.1e-6));

    const auto measurement = devices.gateway.publish_next_measurement(120ms);
    CHECK(measurement.contract_version ==
          mehlissa::models::iot::gateway_measurement_contract_version);
    CHECK(measurement.gateway_id == "gateway.synthetic.wrist");
    CHECK(measurement.source_message_id == "message.m6-4.measurement.1");
    CHECK(measurement.source_device_id == "collector.synthetic.uplink.1");
    CHECK(measurement.correlation_id == "experiment.m6-4.gateway-roundtrip");
    CHECK(measurement.source_event_id == "detection.m6-2.receptor-threshold.1");
    CHECK(measurement.observed_at == 120ms);
    CHECK(devices.gateway.measurement_count() == 1);
    CHECK(devices.gateway.endpoint().used_message_storage_bytes() == 0);

    const auto downlink_request = devices.gateway.prepare_downlink(command());
    CHECK(downlink_request.kind == LocalMessageKind::control);
    CHECK(downlink_request.source_event_id == "command.m6-4.activate.1");
    const auto downlink =
        session.exchange(devices.map(), devices.gateway.gateway_id(), devices.actuator.device_id(),
                         downlink_request, ClusterRouteStrategy::lowest_total_latency);

    REQUIRE(downlink.status == OneHopDeliveryStatus::delivered);
    REQUIRE(downlink.hops.size() == 2);
    CHECK(downlink.metrics.total_delivery_latency == 20ms);
    CHECK(downlink.metrics.attempted_bytes == 256);
    CHECK(mehlissa::core::in_joules(downlink.metrics.transmitter_energy) == Catch::Approx(1.1e-6));
    CHECK(mehlissa::core::in_joules(downlink.metrics.receiver_energy) == Catch::Approx(0.45e-6));
    CHECK(mehlissa::core::in_joules(downlink.metrics.link_energy) == Catch::Approx(0.1e-6));
    CHECK(devices.gateway.command_count() == 1);
    REQUIRE(devices.actuator.received_messages().size() == 1);
    CHECK(devices.actuator.received_messages().front().kind == LocalMessageKind::control);
    CHECK(devices.actuator.received_messages().front().source_event_id ==
          "command.m6-4.activate.1");
    CHECK(mehlissa::core::in_joules(devices.gateway.endpoint().remaining_energy()) ==
          Catch::Approx(8.8e-6));
}

TEST_CASE("Gateway rejects empty publication duplicate commands and incompatible endpoints",
          "[m6][iot][gateway][validation]") {
    GatewayDevices devices;
    CHECK_THROWS_AS(devices.gateway.publish_next_measurement(100ms), mehlissa::core::MehlissaError);
    CHECK(devices.gateway.measurement_count() == 0);

    static_cast<void>(devices.gateway.prepare_downlink(command()));
    CHECK_THROWS_AS(devices.gateway.prepare_downlink(command()), mehlissa::core::MehlissaError);
    CHECK(devices.gateway.command_count() == 1);

    auto self_command = command("command.m6-4.self.1");
    self_command.target_device_id = devices.gateway.gateway_id();
    CHECK_THROWS_AS(devices.gateway.prepare_downlink(self_command), mehlissa::core::MehlissaError);
    CHECK(devices.gateway.command_count() == 1);

    auto endpoint = load_device("synthetic-gateway-endpoint-v1.json");
    const auto collect = std::ranges::find(endpoint.device.capabilities,
                                           mehlissa::models::iot::NanodeviceCapability::collect);
    endpoint.device.capabilities.erase(collect);
    const auto gateway_profile = load_gateway();
    CHECK_THROWS_AS(ActiveGateway(gateway_profile.gateway, endpoint.device, endpoint.profile_id),
                    mehlissa::core::MehlissaError);
    CHECK_THROWS_AS(ActiveGateway(gateway_profile.gateway,
                                  load_device("synthetic-gateway-endpoint-v1.json").device,
                                  "wrong-profile"),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Unsupported uplink kind remains buffered and unpublished",
          "[m6][iot][gateway][uplink-validation]") {
    GatewayDevices devices;
    auto unsupported = measurement_request();
    unsupported.kind = LocalMessageKind::control;
    const auto message = devices.collector.emit_local_message(unsupported);
    devices.gateway.endpoint().receive_local_message(message, 120ms);

    CHECK_THROWS_AS(devices.gateway.publish_next_measurement(120ms), mehlissa::core::MehlissaError);
    CHECK(devices.gateway.measurement_count() == 0);
    CHECK(devices.gateway.endpoint().received_messages().size() == 1);
}

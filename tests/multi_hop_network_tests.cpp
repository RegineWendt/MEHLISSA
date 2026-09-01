// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/cluster_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <ranges>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::iot::BoundedMultiHopSession;
using mehlissa::models::iot::ClusterDeviceMap;
using mehlissa::models::iot::ClusterRouteStrategy;
using mehlissa::models::iot::LocalMessageKind;
using mehlissa::models::iot::Nanodevice;
using mehlissa::models::iot::NanodeviceCluster;
using mehlissa::models::iot::NanodeviceProfile;
using mehlissa::models::iot::OneHopDeliveryStatus;
using mehlissa::models::iot::OneHopDropReason;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] NanodeviceProfile load_device(const std::string_view file_name) {
    return mehlissa::models::iot::load_nanodevice_profile(
        {root() / "examples/iot-models" / file_name,
         root() / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::ClusterCommunicationProfile load_cluster() {
    return mehlissa::models::iot::load_cluster_communication_profile(
        {root() / "examples/iot-models/synthetic-cluster-communication-v1.json",
         root() / "data/schemas/cluster-communication-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::LocalMessageRequest request() {
    return {"message.m6-3.multihop.1",
            LocalMessageKind::detection,
            "collector.synthetic.1",
            "experiment.m6-3.cluster",
            "detection.m6-2.receptor-threshold.1",
            100ms,
            2s,
            3,
            320,
            "application/vnd.mehlissa.detection+json;version=1.0.0",
            R"({"event_id":"detection.m6-2.receptor-threshold.1"})"};
}

struct Devices final {
    Nanodevice locator{load_device("synthetic-locator-v1.json").device};
    Nanodevice relay{load_device("synthetic-relay-v1.json").device};
    Nanodevice collector{load_device("synthetic-collector-v1.json").device};

    [[nodiscard]] ClusterDeviceMap map() {
        return {{std::string{locator.device_id()}, locator},
                {std::string{relay.device_id()}, relay},
                {std::string{collector.device_id()}, collector}};
    }
};

} // namespace

TEST_CASE("Strict cluster profile selects deterministic bounded routes",
          "[m6][iot][cluster][schema]") {
    const auto profile = load_cluster();
    NanodeviceCluster cluster{profile.cluster};

    const auto low_latency = cluster.select_route("locator.synthetic.1", "collector.synthetic.1",
                                                  ClusterRouteStrategy::lowest_total_latency);
    CHECK(low_latency.device_ids == std::vector<std::string>{"locator.synthetic.1",
                                                             "relay.synthetic.1",
                                                             "collector.synthetic.1"});
    CHECK(low_latency.link_indices.size() == 2);
    CHECK(low_latency.total_configured_latency == 25ms);

    const auto fewest = cluster.select_route("locator.synthetic.1", "collector.synthetic.1",
                                             ClusterRouteStrategy::fewest_hops);
    CHECK(fewest.device_ids ==
          std::vector<std::string>{"locator.synthetic.1", "collector.synthetic.1"});
    CHECK(fewest.link_indices.size() == 1);
    CHECK(fewest.total_configured_latency == 40ms);
}

TEST_CASE("A relay preserves trace and aggregates exact multi-hop metrics",
          "[m6][iot][cluster][relay][metrics]") {
    NanodeviceCluster cluster{load_cluster().cluster};
    Devices devices;
    BoundedMultiHopSession session{cluster};

    const auto result =
        session.exchange(devices.map(), devices.locator.device_id(), devices.collector.device_id(),
                         request(), ClusterRouteStrategy::lowest_total_latency);

    REQUIRE(result.status == OneHopDeliveryStatus::delivered);
    CHECK(result.drop_reason == OneHopDropReason::none);
    REQUIRE(result.hops.size() == 2);
    CHECK(result.hops.front().message.target_device_id == "relay.synthetic.1");
    CHECK(result.hops.back().message.source_device_id == "relay.synthetic.1");
    CHECK(result.hops.back().message.target_device_id == "collector.synthetic.1");
    CHECK(result.hops.back().message.correlation_id == "experiment.m6-3.cluster");
    CHECK(result.hops.back().message.source_event_id == "detection.m6-2.receptor-threshold.1");
    CHECK(result.hops.back().message.hop_limit == 2);
    CHECK(result.metrics.attempted_messages == 2);
    CHECK(result.metrics.delivered_messages == 2);
    CHECK(result.metrics.attempted_bytes == 640);
    CHECK(result.metrics.delivered_bytes == 640);
    CHECK(result.metrics.total_delivery_latency == 25ms);
    CHECK(result.metrics.maximum_delivery_latency == 15ms);
    CHECK(mehlissa::core::in_joules(result.metrics.transmitter_energy) == Catch::Approx(0.8e-6));
    CHECK(mehlissa::core::in_joules(result.metrics.receiver_energy) == Catch::Approx(0.45e-6));
    CHECK(mehlissa::core::in_joules(result.metrics.link_energy) == Catch::Approx(0.12e-6));
    CHECK(devices.relay.used_message_storage_bytes() == 0);
    CHECK(devices.collector.reception_count() == 1);
}

TEST_CASE("A second-hop loss terminates forwarding without collector mutation",
          "[m6][iot][cluster][drop]") {
    auto config = load_cluster().cluster;
    config.links.at(1).link.repeating_outcomes = {
        mehlissa::models::iot::ScheduledLinkOutcome::lost};
    NanodeviceCluster cluster{config};
    Devices devices;
    BoundedMultiHopSession session{cluster};

    const auto result =
        session.exchange(devices.map(), devices.locator.device_id(), devices.collector.device_id(),
                         request(), ClusterRouteStrategy::lowest_total_latency);

    CHECK(result.status == OneHopDeliveryStatus::dropped);
    CHECK(result.drop_reason == OneHopDropReason::loss);
    CHECK(result.hops.size() == 2);
    CHECK(result.metrics.attempted_messages == 2);
    CHECK(result.metrics.delivered_messages == 1);
    CHECK(result.metrics.lost_messages == 1);
    CHECK(result.metrics.delivered_bytes == 320);
    CHECK(result.metrics.total_delivery_latency == 10ms);
    CHECK(devices.relay.used_message_storage_bytes() == 0);
    CHECK(devices.collector.reception_count() == 0);
}

TEST_CASE("Hop bounds and relay capabilities fail before route mutation",
          "[m6][iot][cluster][validation]") {
    NanodeviceCluster cluster{load_cluster().cluster};
    Devices devices;
    BoundedMultiHopSession session{cluster};
    auto too_short = request();
    too_short.hop_limit = 1;

    CHECK_THROWS_AS(session.exchange(devices.map(), devices.locator.device_id(),
                                     devices.collector.device_id(), too_short,
                                     ClusterRouteStrategy::lowest_total_latency),
                    mehlissa::core::MehlissaError);
    CHECK(devices.locator.transmission_count() == 0);
    CHECK(devices.relay.reception_count() == 0);

    auto non_relay_config = load_device("synthetic-relay-v1.json").device;
    const auto relay_capability = std::ranges::find(
        non_relay_config.capabilities, mehlissa::models::iot::NanodeviceCapability::relay);
    non_relay_config.capabilities.erase(relay_capability);
    Nanodevice non_relay{non_relay_config};
    auto device_map = devices.map();
    device_map.at("relay.synthetic.1") = non_relay;
    CHECK_THROWS_AS(session.exchange(device_map, devices.locator.device_id(),
                                     devices.collector.device_id(), request(),
                                     ClusterRouteStrategy::lowest_total_latency),
                    mehlissa::core::MehlissaError);
    CHECK(devices.locator.transmission_count() == 0);
}

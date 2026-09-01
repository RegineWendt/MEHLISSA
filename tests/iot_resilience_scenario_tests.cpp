// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/ban_station_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>
#include <mehlissa/models/iot/network_simulator_adapter.hpp>
#include <mehlissa/models/iot/network_simulator_adapter_profile.hpp>
#include <mehlissa/models/iot/resilience_scenario_profile.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>

namespace {

using namespace std::chrono_literals;
using namespace mehlissa::models::iot;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] ResilienceScenarioProfile load_resilience_profile() {
    return load_resilience_scenario_profile(
        {root() / "examples/iot-models/synthetic-iot-resilience-scenarios-v1.json",
         root() / "data/schemas/iot-resilience-scenario-profile/1.0.0.schema.json"});
}

[[nodiscard]] BanStationProfile load_ban_profile() {
    return load_ban_station_profile(
        {root() / "examples/iot-models/synthetic-ban-station-v1.json",
         root() / "data/schemas/ban-station-profile/1.0.0.schema.json"});
}

[[nodiscard]] NetworkSimulatorAdapterProfile load_network_profile() {
    return load_network_simulator_adapter_profile(
        {root() / "examples/iot-models/synthetic-external-network-simulator-v1.json",
         root() / "data/schemas/network-simulator-adapter-profile/1.0.0.schema.json"});
}

[[nodiscard]] NanodeviceProfile load_device(const std::string_view file_name) {
    return load_nanodevice_profile({root() / "examples/iot-models" / file_name,
                                    root() / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
}

[[nodiscard]] GatewayMeasurement measurement(const std::string& id = "measurement.m6-7.1") {
    return {std::string{gateway_measurement_contract_version},
            id,
            "gateway.synthetic.wrist",
            "message.m6-7.1",
            "collector.synthetic.uplink.1",
            "experiment.m6-7.resilience",
            "detection.m6-7.1",
            120ms,
            256,
            "application/vnd.mehlissa.measurement+json;version=1.0.0",
            R"({"detected":true})"};
}

[[nodiscard]] GatewayCommand command(const std::string& id = "command.m6-7.1") {
    return {std::string{gateway_command_contract_version},
            id,
            "actuator.synthetic.1",
            "experiment.m6-7.resilience",
            150ms,
            1s,
            3,
            128,
            "application/vnd.mehlissa.control+json;version=1.0.0",
            R"({"action":"activate"})"};
}

[[nodiscard]] LocalMessageRequest local_request(const std::string& id) {
    return {id,
            LocalMessageKind::detection,
            "collector.synthetic.1",
            "experiment.m6-7.resilience",
            "detection.m6-7.1",
            100ms,
            1s,
            1,
            64,
            "application/vnd.mehlissa.detection+json;version=1.0.0",
            R"({"detected":true})"};
}

[[nodiscard]] const ResilienceScenario& scenario(const ResilienceScenarioProfile& profile,
                                                 const ResilienceInjection injection) {
    const auto found =
        std::ranges::find(profile.scenarios, injection, &ResilienceScenario::injection);
    REQUIRE(found != profile.scenarios.end());
    return *found;
}

class EchoNetworkClient final : public NetworkSimulatorClient {
  public:
    explicit EchoNetworkClient(const bool mismatch_identity = false)
        : mismatch_identity_{mismatch_identity} {}

    [[nodiscard]] NetworkSimulationResponse
    simulate(const NetworkSimulationRequest& request) override {
        ++call_count_;
        return {std::string{network_simulation_response_contract_version},
                mismatch_identity_ ? "network-request.mismatched" : request.request_id,
                request.adapter_id,
                request.simulator_id,
                request.simulator_version,
                request.scenario_id,
                request.frame_id,
                NetworkSimulationOutcome::delivered,
                request.departed_at + 10ms,
                mehlissa::core::joules(2.0e-6),
                mehlissa::core::joules(1.0e-6),
                mehlissa::core::joules(0.5e-6)};
    }

    [[nodiscard]] std::uint64_t call_count() const noexcept { return call_count_; }

  private:
    bool mismatch_identity_{};
    std::uint64_t call_count_{};
};

} // namespace

TEST_CASE("Strict M6 resilience profile covers every predeclared boundary scenario",
          "[m6][iot][resilience][schema]") {
    const auto profile = load_resilience_profile();

    CHECK(profile.baseline_profile_id == "synthetic-ban-station-v1");
    CHECK(profile.scenarios.size() == 12);
    CHECK(profile.security_scope.protected_properties.size() == 6);
    CHECK(profile.security_scope.excluded_claims.size() == 5);
    for (const auto& item : profile.scenarios) {
        CHECK(item.protected_state_unchanged);
        CHECK(to_string(item.injection) != "unknown");
        CHECK(to_string(item.expected_disposition) != "unknown");
    }

    auto incomplete = profile;
    incomplete.scenarios.pop_back();
    CHECK_THROWS_AS(validate_resilience_scenario_profile(incomplete),
                    mehlissa::core::MehlissaError);

    auto inconsistent = profile;
    inconsistent.scenarios.front().expected_disposition = ResilienceDisposition::rejected_invariant;
    CHECK_THROWS_AS(validate_resilience_scenario_profile(inconsistent),
                    mehlissa::core::MehlissaError);

    auto wrong_boundary = profile;
    wrong_boundary.scenarios.front().expected_boundary = "station_governance";
    CHECK_THROWS_AS(validate_resilience_scenario_profile(wrong_boundary),
                    mehlissa::core::MehlissaError);

    auto missing_accounting = profile;
    missing_accounting.scenarios.front().communication_metrics_accounted = false;
    CHECK_THROWS_AS(validate_resilience_scenario_profile(missing_accounting),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Prescribed BAN failure outcomes are accounted without station mutation",
          "[m6][iot][resilience][transport]") {
    const auto resilience = load_resilience_profile();
    auto profile = load_ban_profile();
    profile.uplink_transport.repeating_outcomes = {ScheduledLinkOutcome::lost,
                                                   ScheduledLinkOutcome::corrupted,
                                                   ScheduledLinkOutcome::delivered};
    ScheduledBanTransportAdapter adapter{profile.uplink_transport};
    BanCommunicationSession session{adapter};
    GatewayBanAdapter gateway{profile.gateway_adapter};
    ExternalAnalysisControlStation station{profile.station};

    const auto lost = session.exchange(gateway.publish_measurement(measurement(), 1s));
    CHECK(lost.drop_reason == OneHopDropReason::loss);
    CHECK(scenario(resilience, ResilienceInjection::uplink_loss).communication_metrics_accounted);

    const auto corrupted =
        session.exchange(gateway.publish_measurement(measurement("measurement.m6-7.2"), 1s));
    CHECK(corrupted.drop_reason == OneHopDropReason::corruption);
    CHECK(scenario(resilience, ResilienceInjection::uplink_corruption)
              .communication_metrics_accounted);

    const auto expired =
        session.exchange(gateway.publish_measurement(measurement("measurement.m6-7.3"), 5ms));
    CHECK(expired.drop_reason == OneHopDropReason::expired);
    CHECK(scenario(resilience, ResilienceInjection::frame_expiry).communication_metrics_accounted);

    CHECK(station.measurement_count() == 0);
    CHECK(session.metrics().attempted_messages == 3);
    CHECK(session.metrics().delivered_messages == 0);
    CHECK(session.metrics().lost_messages == 1);
    CHECK(session.metrics().corrupted_messages == 1);
    CHECK(session.metrics().expired_messages == 1);
    CHECK(session.metrics().attempted_bytes == 768);
}

TEST_CASE("Station miscontrol trace and capacity cases deny without command mutation",
          "[m6][iot][resilience][governance]") {
    const auto resilience = load_resilience_profile();
    auto profile = load_ban_profile();
    profile.station.maximum_approved_commands = 1;
    GatewayBanAdapter gateway{profile.gateway_adapter};
    ExternalAnalysisControlStation station{profile.station};
    const auto frame = gateway.publish_measurement(measurement(), 1s);
    station.receive_measurement(frame, 130ms);

    auto wrong_target = command("command.m6-7.wrong-target");
    wrong_target.target_device_id = "actuator.not-allowed";
    CHECK(
        station.evaluate_command({"request.m6-7.wrong-target", "measurement.m6-7.1", wrong_target})
            .status == CommandGovernanceStatus::denied_target);
    CHECK(scenario(resilience, ResilienceInjection::unauthorized_target).expected_disposition ==
          ResilienceDisposition::denied_target);

    auto wrong_type = command("command.m6-7.wrong-type");
    wrong_type.content_type = "application/octet-stream";
    CHECK(station.evaluate_command({"request.m6-7.wrong-type", "measurement.m6-7.1", wrong_type})
              .status == CommandGovernanceStatus::denied_content_type);

    auto wrong_correlation = command("command.m6-7.wrong-correlation");
    wrong_correlation.correlation_id = "experiment.m6-7.unrelated";
    CHECK(station
              .evaluate_command(
                  {"request.m6-7.wrong-correlation", "measurement.m6-7.1", wrong_correlation})
              .status == CommandGovernanceStatus::denied_correlation_mismatch);

    const StationCommandRequest approved{"request.m6-7.approved", "measurement.m6-7.1",
                                         command("command.m6-7.approved")};
    CHECK(station.evaluate_command(approved).status == CommandGovernanceStatus::approved);
    CHECK(station.evaluate_command(approved).status ==
          CommandGovernanceStatus::denied_duplicate_request);
    CHECK(station
              .evaluate_command(
                  {"request.m6-7.excess", "measurement.m6-7.1", command("command.m6-7.excess")})
              .status == CommandGovernanceStatus::denied_capacity);

    CHECK(scenario(resilience, ResilienceInjection::disallowed_content_type).expected_disposition ==
          ResilienceDisposition::denied_content_type);
    CHECK(scenario(resilience, ResilienceInjection::correlation_mismatch).expected_disposition ==
          ResilienceDisposition::denied_correlation_mismatch);
    CHECK(scenario(resilience, ResilienceInjection::duplicate_request).expected_disposition ==
          ResilienceDisposition::denied_duplicate_request);
    CHECK(
        scenario(resilience, ResilienceInjection::station_command_capacity).expected_disposition ==
        ResilienceDisposition::denied_capacity);
    CHECK(station.measurement_count() == 1);
    CHECK(station.approved_command_count() == 1);
}

TEST_CASE("Gateway rejects replay without accepting a second command",
          "[m6][iot][resilience][replay]") {
    const auto resilience = load_resilience_profile();
    const auto profile = load_ban_profile();
    GatewayBanAdapter gateway{profile.gateway_adapter};
    ExternalAnalysisControlStation station{profile.station};
    const auto measurement_frame = gateway.publish_measurement(measurement(), 1s);
    station.receive_measurement(measurement_frame, 130ms);
    const auto approved =
        station.evaluate_command({"request.m6-7.replay", "measurement.m6-7.1", command()});
    REQUIRE(approved.status == CommandGovernanceStatus::approved);
    const auto command_frame = station.prepare_command_frame(approved);

    static_cast<void>(gateway.accept_command(command_frame, 160ms));
    CHECK_THROWS_AS(gateway.accept_command(command_frame, 160ms), mehlissa::core::MehlissaError);
    CHECK(gateway.downlink_count() == 1);
    CHECK(scenario(resilience, ResilienceInjection::gateway_command_replay).expected_disposition ==
          ResilienceDisposition::rejected_invariant);
}

TEST_CASE("External simulator identity and attempt bounds fail closed",
          "[m6][iot][resilience][network-sim]") {
    const auto resilience = load_resilience_profile();
    auto profile = load_network_profile();
    const auto ban_profile = load_ban_profile();
    GatewayBanAdapter gateway{ban_profile.gateway_adapter};
    const auto frame = gateway.publish_measurement(measurement(), 1s);

    EchoNetworkClient mismatch_client{true};
    ExternalNetworkSimulatorAdapter mismatch_adapter{profile.adapter, mismatch_client};
    CHECK_THROWS_AS(mismatch_adapter.transfer(frame), mehlissa::core::MehlissaError);
    CHECK(mismatch_adapter.attempt_count() == 1);
    CHECK(mismatch_client.call_count() == 1);

    profile.adapter.maximum_attempts = 1;
    EchoNetworkClient capacity_client;
    ExternalNetworkSimulatorAdapter capacity_adapter{profile.adapter, capacity_client};
    CHECK(capacity_adapter.transfer(frame).status == OneHopDeliveryStatus::delivered);
    CHECK_THROWS_AS(capacity_adapter.transfer(frame), mehlissa::core::MehlissaError);
    CHECK(capacity_adapter.attempt_count() == 1);
    CHECK(capacity_client.call_count() == 1);

    CHECK(scenario(resilience, ResilienceInjection::external_response_identity_mismatch)
              .expected_disposition == ResilienceDisposition::rejected_invariant);
    CHECK(
        scenario(resilience, ResilienceInjection::external_attempt_capacity).expected_disposition ==
        ResilienceDisposition::rejected_invariant);
}

TEST_CASE("Local resource exhaustion rejects a second emission without extra debit",
          "[m6][iot][resilience][resource]") {
    const auto resilience = load_resilience_profile();
    auto config = load_device("synthetic-locator-v1.json").device;
    config.resources.maximum_transmissions = 1;
    Nanodevice device{config};
    static_cast<void>(device.emit_local_message(local_request("message.m6-7.first")));
    const auto remaining_energy = device.remaining_energy();

    CHECK_THROWS_AS(device.emit_local_message(local_request("message.m6-7.excess")),
                    mehlissa::core::MehlissaError);
    CHECK(device.transmission_count() == 1);
    CHECK(device.remaining_energy() == remaining_energy);
    CHECK(
        scenario(resilience, ResilienceInjection::local_resource_exhaustion).expected_disposition ==
        ResilienceDisposition::rejected_resource);
}

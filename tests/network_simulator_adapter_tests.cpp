// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/ban_station_profile.hpp>
#include <mehlissa/models/iot/network_simulator_adapter.hpp>
#include <mehlissa/models/iot/network_simulator_adapter_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

namespace {

using namespace std::chrono_literals;
using namespace mehlissa::models::iot;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] std::string read_text(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    REQUIRE(stream.good());
    std::ostringstream content;
    content << stream.rdbuf();
    return content.str();
}

void validate_wire_json(const std::string_view document, const std::filesystem::path& schema_path) {
    std::ifstream schema_stream{schema_path, std::ios::binary};
    REQUIRE(schema_stream.good());
    const auto schema =
        jsoncons::jsonschema::make_json_schema(jsoncons::json::parse(schema_stream));
    schema.validate(jsoncons::json::parse(document));
}

[[nodiscard]] BanStationProfile load_ban_profile() {
    return load_ban_station_profile(
        {root() / "examples/iot-models/synthetic-ban-station-v1.json",
         root() / "data/schemas/ban-station-profile/1.0.0.schema.json"});
}

[[nodiscard]] NetworkSimulatorAdapterProfile load_adapter_profile() {
    return load_network_simulator_adapter_profile(
        {root() / "examples/iot-models/synthetic-external-network-simulator-v1.json",
         root() / "data/schemas/network-simulator-adapter-profile/1.0.0.schema.json"});
}

[[nodiscard]] GatewayMeasurement measurement() {
    return {std::string{gateway_measurement_contract_version},
            "measurement.m6-5.lung.1",
            "gateway.synthetic.wrist",
            "lung.1",
            "collector.synthetic.uplink.1",
            "experiment.m6-5.closed-loop",
            "detection.m6-2.receptor-threshold.1",
            120ms,
            256,
            "application/vnd.mehlissa.measurement+json;version=1.0.0",
            R"({"tissue_id":"lung.tissue.synthetic","detected":true})"};
}

[[nodiscard]] BanFrame
measurement_frame(const mehlissa::core::SimulationClock::Duration valid_for = 1s) {
    GatewayBanAdapter gateway{load_ban_profile().gateway_adapter};
    return gateway.publish_measurement(measurement(), valid_for);
}

class FixtureJsonExchange final : public NetworkSimulatorJsonExchange {
  public:
    explicit FixtureJsonExchange(std::string response) : response_{std::move(response)} {}

    [[nodiscard]] std::string exchange(const std::string_view request_json) override {
        last_request_ = request_json;
        ++call_count_;
        return response_;
    }

    [[nodiscard]] const std::string& last_request() const noexcept { return last_request_; }
    [[nodiscard]] std::uint64_t call_count() const noexcept { return call_count_; }

  private:
    std::string response_;
    std::string last_request_;
    std::uint64_t call_count_{};
};

[[nodiscard]] std::string
response_json(const std::string_view request_id, const std::string_view outcome,
              const std::int64_t completed_at_ns,
              const std::string_view adapter_id = "adapter.synthetic.external-network-simulator") {
    return "{\"contract_version\":\"1.0.0\",\"request_id\":\"" + std::string{request_id} +
           "\",\"adapter_id\":\"" + std::string{adapter_id} +
           "\",\"simulator_id\":\"simulator.synthetic.fixture\"," +
           "\"simulator_version\":\"1.0.0\",\"scenario_id\":" +
           "\"scenario.synthetic.ban-link-v1\",\"frame_id\":" +
           "\"ban-frame.m6-5.measurement.measurement.m6-5.lung.1\",\"outcome\":\"" +
           std::string{outcome} + "\",\"completed_at_ns\":" + std::to_string(completed_at_ns) +
           ",\"transmitter_energy_j\":0.0000025,\"receiver_energy_j\":0.00000125," +
           "\"link_energy_j\":0.0000006}";
}

inline constexpr std::string_view reference_request_id =
    "network-request.m6-6.ban-frame.m6-5.measurement.measurement.m6-5.lung.1.1";

} // namespace

TEST_CASE("Strict external network-simulator profile binds one scenario",
          "[m6][iot][network-sim][schema]") {
    const auto profile = load_adapter_profile();
    CHECK(profile.adapter.simulator_id == "simulator.synthetic.fixture");
    CHECK(profile.adapter.scenario_id == "scenario.synthetic.ban-link-v1");
    CHECK(profile.reference_case.expected_outcome == NetworkSimulationOutcome::delivered);
    CHECK(profile.reference_case.expected_latency == 12ms);
    CHECK(mehlissa::core::in_joules(profile.reference_case.expected_link_energy) ==
          Catch::Approx(0.6e-6));
}

TEST_CASE("External simulator JSON result drives the existing BAN session",
          "[m6][iot][network-sim][exchange]") {
    const auto profile = load_adapter_profile();
    FixtureJsonExchange exchange{
        read_text(root() / "examples/iot-models/synthetic-network-simulator-response-v1.json")};
    validate_wire_json(
        read_text(root() / "examples/iot-models/synthetic-network-simulator-response-v1.json"),
        root() / "data/schemas/network-simulation-response/1.0.0.schema.json");
    JsonNetworkSimulatorClient client{exchange};
    ExternalNetworkSimulatorAdapter adapter{profile.adapter, client};
    BanCommunicationSession session{adapter};

    const auto result = session.exchange(measurement_frame());

    REQUIRE(result.status == OneHopDeliveryStatus::delivered);
    CHECK(result.drop_reason == OneHopDropReason::none);
    CHECK(result.latency == profile.reference_case.expected_latency);
    CHECK(result.completed_at == 132ms);
    CHECK(mehlissa::core::in_joules(result.transmitter_energy) == Catch::Approx(2.5e-6));
    CHECK(mehlissa::core::in_joules(result.receiver_energy) == Catch::Approx(1.25e-6));
    CHECK(mehlissa::core::in_joules(result.link_energy) == Catch::Approx(0.6e-6));
    CHECK(session.metrics().delivered_messages == 1);
    CHECK(session.metrics().attempted_bytes == 256);
    CHECK(adapter.attempt_count() == 1);
    CHECK(exchange.call_count() == 1);

    CHECK(exchange.last_request().find("\"frame_kind\":\"measurement_uplink\"") !=
          std::string::npos);
    CHECK(exchange.last_request().find("\"valid_until_ns\":1120000000") != std::string::npos);
    CHECK(exchange.last_request().find("tissue_id") == std::string::npos);
    CHECK(exchange.last_request().find("payload") == std::string::npos);
    validate_wire_json(exchange.last_request(),
                       root() / "data/schemas/network-simulation-request/1.0.0.schema.json");
}

TEST_CASE("External adapter preserves simulator loss corruption and BAN expiry",
          "[m6][iot][network-sim][outcomes]") {
    const auto profile = load_adapter_profile();

    FixtureJsonExchange loss_exchange{response_json(reference_request_id, "lost", 132000000)};
    JsonNetworkSimulatorClient loss_client{loss_exchange};
    ExternalNetworkSimulatorAdapter loss_adapter{profile.adapter, loss_client};
    CHECK(loss_adapter.transfer(measurement_frame()).drop_reason == OneHopDropReason::loss);

    FixtureJsonExchange corruption_exchange{
        response_json(reference_request_id, "corrupted", 132000000)};
    JsonNetworkSimulatorClient corruption_client{corruption_exchange};
    ExternalNetworkSimulatorAdapter corruption_adapter{profile.adapter, corruption_client};
    CHECK(corruption_adapter.transfer(measurement_frame()).drop_reason ==
          OneHopDropReason::corruption);

    FixtureJsonExchange expiry_exchange{
        response_json(reference_request_id, "delivered", 132000000)};
    JsonNetworkSimulatorClient expiry_client{expiry_exchange};
    ExternalNetworkSimulatorAdapter expiry_adapter{profile.adapter, expiry_client};
    const auto expired = expiry_adapter.transfer(measurement_frame(5ms));
    CHECK(expired.status == OneHopDeliveryStatus::dropped);
    CHECK(expired.drop_reason == OneHopDropReason::expired);
}

TEST_CASE("External adapter rejects malformed mismatched and excess responses",
          "[m6][iot][network-sim][validation]") {
    auto profile = load_adapter_profile();

    FixtureJsonExchange mismatch_exchange{
        response_json("network-request.wrong", "delivered", 132000000)};
    JsonNetworkSimulatorClient mismatch_client{mismatch_exchange};
    ExternalNetworkSimulatorAdapter mismatch_adapter{profile.adapter, mismatch_client};
    CHECK_THROWS_AS(mismatch_adapter.transfer(measurement_frame()), mehlissa::core::MehlissaError);
    CHECK(mismatch_adapter.attempt_count() == 1);

    FixtureJsonExchange extra_field_exchange{
        std::string{response_json(reference_request_id, "delivered", 132000000)}.replace(
            response_json(reference_request_id, "delivered", 132000000).size() - 1, 1,
            ",\"unexpected\":true}")};
    JsonNetworkSimulatorClient extra_field_client{extra_field_exchange};
    ExternalNetworkSimulatorAdapter extra_field_adapter{profile.adapter, extra_field_client};
    CHECK_THROWS_AS(extra_field_adapter.transfer(measurement_frame()),
                    mehlissa::core::MehlissaError);

    profile.adapter.maximum_attempts = 1;
    FixtureJsonExchange capacity_exchange{
        response_json(reference_request_id, "delivered", 132000000)};
    JsonNetworkSimulatorClient capacity_client{capacity_exchange};
    ExternalNetworkSimulatorAdapter capacity_adapter{profile.adapter, capacity_client};
    static_cast<void>(capacity_adapter.transfer(measurement_frame()));
    CHECK_THROWS_AS(capacity_adapter.transfer(measurement_frame()), mehlissa::core::MehlissaError);
    CHECK(capacity_exchange.call_count() == 1);
}

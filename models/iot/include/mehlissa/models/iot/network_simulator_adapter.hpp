// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_HPP
#define MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_HPP

#include <mehlissa/models/iot/ban_station.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace mehlissa::models::iot {

inline constexpr std::string_view network_simulation_request_contract_version = "1.0.0";
inline constexpr std::string_view network_simulation_response_contract_version = "1.0.0";
inline constexpr std::string_view external_network_simulator_adapter_kind =
    "external_network_simulator";

enum class NetworkSimulationOutcome : std::uint8_t { delivered, lost, corrupted };

[[nodiscard]] std::string_view to_string(NetworkSimulationOutcome outcome) noexcept;

struct NetworkSimulationRequest final {
    std::string contract_version;
    std::string request_id;
    std::string adapter_id;
    std::string simulator_id;
    std::string simulator_version;
    std::string scenario_id;
    std::string frame_id;
    BanFrameKind frame_kind{BanFrameKind::measurement_uplink};
    std::string source_endpoint_id;
    std::string target_endpoint_id;
    std::string correlation_id;
    std::string source_event_id;
    core::SimulationClock::Duration departed_at{};
    core::SimulationClock::Duration valid_until{};
    std::uint64_t size_bytes{};
};

struct NetworkSimulationResponse final {
    std::string contract_version;
    std::string request_id;
    std::string adapter_id;
    std::string simulator_id;
    std::string simulator_version;
    std::string scenario_id;
    std::string frame_id;
    NetworkSimulationOutcome outcome{NetworkSimulationOutcome::lost};
    core::SimulationClock::Duration completed_at{};
    core::Energy transmitter_energy{};
    core::Energy receiver_energy{};
    core::Energy link_energy{};
};

void validate_network_simulation_request(const NetworkSimulationRequest& request);
void validate_network_simulation_response(const NetworkSimulationResponse& response);

[[nodiscard]] std::string
encode_network_simulation_request_json(const NetworkSimulationRequest& request);
[[nodiscard]] NetworkSimulationResponse
decode_network_simulation_response_json(std::string_view response_json);

class NetworkSimulatorClient {
  public:
    virtual ~NetworkSimulatorClient() = default;

    [[nodiscard]] virtual NetworkSimulationResponse
    simulate(const NetworkSimulationRequest& request) = 0;
};

class NetworkSimulatorJsonExchange {
  public:
    virtual ~NetworkSimulatorJsonExchange() = default;

    [[nodiscard]] virtual std::string exchange(std::string_view request_json) = 0;
};

class JsonNetworkSimulatorClient final : public NetworkSimulatorClient {
  public:
    explicit JsonNetworkSimulatorClient(NetworkSimulatorJsonExchange& exchange) noexcept;

    [[nodiscard]] NetworkSimulationResponse
    simulate(const NetworkSimulationRequest& request) override;

  private:
    NetworkSimulatorJsonExchange& exchange_;
};

struct ExternalNetworkSimulatorAdapterConfig final {
    std::string adapter_id;
    std::string simulator_id;
    std::string simulator_version;
    std::string scenario_id;
    std::string request_id_prefix;
    std::uint64_t maximum_attempts{};
};

void validate_external_network_simulator_adapter_config(
    const ExternalNetworkSimulatorAdapterConfig& config);

class ExternalNetworkSimulatorAdapter final : public BanTransportAdapter {
  public:
    ExternalNetworkSimulatorAdapter(ExternalNetworkSimulatorAdapterConfig config,
                                    NetworkSimulatorClient& client);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view adapter_id() const noexcept override;
    [[nodiscard]] BanTransferResult transfer(const BanFrame& frame) override;
    [[nodiscard]] std::uint64_t attempt_count() const noexcept;

  private:
    ExternalNetworkSimulatorAdapterConfig config_;
    NetworkSimulatorClient& client_;
    std::uint64_t attempt_count_{};
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_HPP

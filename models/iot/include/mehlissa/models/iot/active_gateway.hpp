// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_HPP
#define MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_HPP

#include <mehlissa/models/iot/nanodevice.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view gateway_measurement_contract_version = "1.0.0";
inline constexpr std::string_view gateway_command_contract_version = "1.0.0";

struct GatewayMeasurement final {
    std::string contract_version;
    std::string measurement_id;
    std::string gateway_id;
    std::string source_message_id;
    std::string source_device_id;
    std::string correlation_id;
    std::string source_event_id;
    core::SimulationClock::Duration observed_at{};
    std::uint64_t size_bytes{};
    std::string content_type;
    std::string content;
};

struct GatewayCommand final {
    std::string contract_version;
    std::string command_id;
    std::string target_device_id;
    std::string correlation_id;
    core::SimulationClock::Duration created_at{};
    core::SimulationClock::Duration valid_for{};
    std::uint32_t hop_limit{};
    std::uint64_t size_bytes{};
    std::string content_type;
    std::string content;
};

void validate_gateway_measurement(const GatewayMeasurement& measurement);
void validate_gateway_command(const GatewayCommand& command);

struct ActiveGatewayConfig final {
    std::string gateway_id;
    std::string endpoint_profile_id;
    std::string measurement_id_prefix;
    std::string command_message_id_prefix;
    std::vector<LocalMessageKind> accepted_uplink_kinds;
    std::uint64_t maximum_measurements{};
    std::uint64_t maximum_commands{};
};

void validate_active_gateway_config(const ActiveGatewayConfig& config);

class ActiveGateway final {
  public:
    ActiveGateway(ActiveGatewayConfig config, NanodeviceConfig endpoint_config,
                  std::string_view endpoint_profile_id);

    [[nodiscard]] std::string_view gateway_id() const noexcept;
    [[nodiscard]] Nanodevice& endpoint() noexcept;
    [[nodiscard]] const Nanodevice& endpoint() const noexcept;
    [[nodiscard]] std::uint64_t measurement_count() const noexcept;
    [[nodiscard]] std::uint64_t command_count() const noexcept;

    [[nodiscard]] GatewayMeasurement
    publish_next_measurement(core::SimulationClock::Duration observed_at);
    [[nodiscard]] LocalMessageRequest prepare_downlink(const GatewayCommand& command);

  private:
    ActiveGatewayConfig config_;
    Nanodevice endpoint_;
    std::uint64_t measurement_count_{};
    std::uint64_t command_count_{};
    std::unordered_set<std::string> command_ids_;
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_HPP

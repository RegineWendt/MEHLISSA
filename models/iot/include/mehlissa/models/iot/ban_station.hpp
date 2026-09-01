// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_BAN_STATION_HPP
#define MEHLISSA_MODELS_IOT_BAN_STATION_HPP

#include <mehlissa/models/iot/active_gateway.hpp>
#include <mehlissa/models/iot/one_hop_link.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view ban_frame_contract_version = "1.0.0";
inline constexpr std::string_view station_governance_contract_version = "1.0.0";
inline constexpr std::string_view scheduled_ban_transport_kind = "scheduled_ban_transport";

enum class BanFrameKind : std::uint8_t { measurement_uplink, governed_command_downlink };
enum class CommandGovernanceStatus : std::uint8_t {
    approved,
    denied_unknown_measurement,
    denied_time_order,
    denied_correlation_mismatch,
    denied_target,
    denied_content_type,
    denied_capacity,
    denied_duplicate_request
};

[[nodiscard]] std::string_view to_string(BanFrameKind kind) noexcept;
[[nodiscard]] std::string_view to_string(CommandGovernanceStatus status) noexcept;

struct StationCommandRequest final {
    std::string request_id;
    std::string source_measurement_id;
    GatewayCommand command;
};

struct GovernedGatewayCommand final {
    std::string contract_version;
    std::string decision_id;
    std::string station_id;
    std::string gateway_id;
    std::string source_measurement_id;
    GatewayCommand command;
};

using BanFramePayload = std::variant<GatewayMeasurement, GovernedGatewayCommand>;

struct BanFrame final {
    std::string contract_version;
    std::string frame_id;
    BanFrameKind kind{BanFrameKind::measurement_uplink};
    std::string source_endpoint_id;
    std::string target_endpoint_id;
    std::string correlation_id;
    std::string source_event_id;
    core::SimulationClock::Duration created_at{};
    core::SimulationClock::Duration valid_for{};
    std::uint64_t size_bytes{};
    BanFramePayload payload;
};

void validate_station_command_request(const StationCommandRequest& request);
void validate_governed_gateway_command(const GovernedGatewayCommand& command);
void validate_ban_frame(const BanFrame& frame);
[[nodiscard]] core::SimulationClock::Duration valid_until(const BanFrame& frame);

struct GatewayBanAdapterConfig final {
    std::string adapter_id;
    std::string gateway_id;
    std::string station_id;
    std::string measurement_frame_id_prefix;
    std::uint64_t maximum_uplink_frames{};
    std::uint64_t maximum_downlink_frames{};
};

void validate_gateway_ban_adapter_config(const GatewayBanAdapterConfig& config);

class GatewayBanAdapter final {
  public:
    explicit GatewayBanAdapter(GatewayBanAdapterConfig config);

    [[nodiscard]] BanFrame publish_measurement(const GatewayMeasurement& measurement,
                                               core::SimulationClock::Duration valid_for);
    [[nodiscard]] GatewayCommand accept_command(const BanFrame& frame,
                                                core::SimulationClock::Duration received_at);
    [[nodiscard]] std::uint64_t uplink_count() const noexcept;
    [[nodiscard]] std::uint64_t downlink_count() const noexcept;

  private:
    GatewayBanAdapterConfig config_;
    std::uint64_t uplink_count_{};
    std::uint64_t downlink_count_{};
    std::unordered_map<std::string, std::string> published_measurements_;
    std::unordered_set<std::string> accepted_decision_ids_;
};

struct ExternalStationConfig final {
    std::string station_id;
    std::string decision_id_prefix;
    std::string command_frame_id_prefix;
    std::vector<std::string> accepted_gateway_ids;
    std::vector<std::string> allowed_target_device_ids;
    std::vector<std::string> allowed_command_content_types;
    std::uint64_t maximum_measurements{};
    std::uint64_t maximum_approved_commands{};
};

void validate_external_station_config(const ExternalStationConfig& config);

struct StationCommandDecision final {
    CommandGovernanceStatus status{CommandGovernanceStatus::denied_unknown_measurement};
    std::string reason;
    std::optional<GovernedGatewayCommand> approved_command;
};

class ExternalAnalysisControlStation final {
  public:
    explicit ExternalAnalysisControlStation(ExternalStationConfig config);

    void receive_measurement(const BanFrame& frame, core::SimulationClock::Duration received_at);
    [[nodiscard]] StationCommandDecision evaluate_command(const StationCommandRequest& request);
    [[nodiscard]] BanFrame prepare_command_frame(const StationCommandDecision& decision) const;
    [[nodiscard]] std::uint64_t measurement_count() const noexcept;
    [[nodiscard]] std::uint64_t approved_command_count() const noexcept;

  private:
    ExternalStationConfig config_;
    std::unordered_map<std::string, GatewayMeasurement> measurements_;
    std::unordered_map<std::string, core::SimulationClock::Duration> measurement_received_at_;
    std::unordered_set<std::string> request_ids_;
    std::uint64_t approved_command_count_{};
};

struct ScheduledBanTransportConfig final {
    std::string adapter_id;
    core::SimulationClock::Duration latency{};
    core::Energy transmitter_energy_per_attempt{};
    core::Energy receiver_energy_per_delivery{};
    core::Energy link_energy_per_attempt{};
    std::vector<ScheduledLinkOutcome> repeating_outcomes;
};

void validate_scheduled_ban_transport_config(const ScheduledBanTransportConfig& config);

struct BanTransferResult final {
    std::string adapter_id;
    std::string frame_id;
    OneHopDeliveryStatus status{OneHopDeliveryStatus::dropped};
    OneHopDropReason drop_reason{OneHopDropReason::none};
    core::SimulationClock::Duration departed_at{};
    core::SimulationClock::Duration completed_at{};
    core::SimulationClock::Duration latency{};
    std::uint64_t size_bytes{};
    core::Energy transmitter_energy{};
    core::Energy receiver_energy{};
    core::Energy link_energy{};
};

class BanTransportAdapter {
  public:
    virtual ~BanTransportAdapter() = default;

    [[nodiscard]] virtual std::string_view kind() const noexcept = 0;
    [[nodiscard]] virtual std::string_view adapter_id() const noexcept = 0;
    [[nodiscard]] virtual BanTransferResult transfer(const BanFrame& frame) = 0;
};

class ScheduledBanTransportAdapter final : public BanTransportAdapter {
  public:
    explicit ScheduledBanTransportAdapter(ScheduledBanTransportConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view adapter_id() const noexcept override;
    [[nodiscard]] BanTransferResult transfer(const BanFrame& frame) override;

  private:
    ScheduledBanTransportConfig config_;
    std::uint64_t attempt_count_{};
};

class BanCommunicationSession final {
  public:
    explicit BanCommunicationSession(BanTransportAdapter& adapter) noexcept;

    [[nodiscard]] BanTransferResult exchange(const BanFrame& frame);
    [[nodiscard]] const CommunicationMetrics& metrics() const noexcept;

  private:
    BanTransportAdapter& adapter_;
    CommunicationMetrics metrics_;
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_BAN_STATION_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/ban_station.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <utility>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool nonnegative_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) >= 0.0;
}

template <typename Range>
[[nodiscard]] bool contains(const Range& values, const std::string_view value) {
    return std::ranges::find(values, value) != values.end();
}

template <typename Range> void require_unique_nonempty(const Range& values) {
    std::unordered_set<std::string> unique;
    for (const auto& value : values) {
        if (value.empty() || !unique.insert(value).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Configured identities must be nonempty and unique");
        }
    }
}

void add_count(std::uint64_t& total, const std::uint64_t increment) {
    if (increment > std::numeric_limits<std::uint64_t>::max() - total) {
        invalid(core::ErrorCode::numeric_overflow, "BAN metric counter overflows");
    }
    total += increment;
}

void add_duration(core::SimulationClock::Duration& total,
                  const core::SimulationClock::Duration increment) {
    if (increment > core::SimulationClock::Duration::max() - total) {
        invalid(core::ErrorCode::numeric_overflow, "BAN latency metric overflows");
    }
    total += increment;
}

void add_energy(core::Energy& total, const core::Energy increment) {
    const auto sum = core::in_joules(total) + core::in_joules(increment);
    if (!std::isfinite(sum) || sum < 0.0) {
        invalid(core::ErrorCode::numeric_overflow, "BAN energy metric overflows");
    }
    total = core::joules(sum);
}

[[nodiscard]] StationCommandDecision denied(const CommandGovernanceStatus status,
                                            std::string reason) {
    return {status, std::move(reason), std::nullopt};
}

} // namespace

std::string_view to_string(const BanFrameKind kind) noexcept {
    switch (kind) {
    case BanFrameKind::measurement_uplink:
        return "measurement_uplink";
    case BanFrameKind::governed_command_downlink:
        return "governed_command_downlink";
    }
    return "unknown";
}

std::string_view to_string(const CommandGovernanceStatus status) noexcept {
    switch (status) {
    case CommandGovernanceStatus::approved:
        return "approved";
    case CommandGovernanceStatus::denied_unknown_measurement:
        return "denied_unknown_measurement";
    case CommandGovernanceStatus::denied_time_order:
        return "denied_time_order";
    case CommandGovernanceStatus::denied_correlation_mismatch:
        return "denied_correlation_mismatch";
    case CommandGovernanceStatus::denied_target:
        return "denied_target";
    case CommandGovernanceStatus::denied_content_type:
        return "denied_content_type";
    case CommandGovernanceStatus::denied_capacity:
        return "denied_capacity";
    case CommandGovernanceStatus::denied_duplicate_request:
        return "denied_duplicate_request";
    }
    return "unknown";
}

void validate_station_command_request(const StationCommandRequest& request) {
    validate_gateway_command(request.command);
    if (request.request_id.empty() || request.source_measurement_id.empty()) {
        invalid(core::ErrorCode::data_invalid, "Station command request requires trace identities");
    }
}

void validate_governed_gateway_command(const GovernedGatewayCommand& command) {
    validate_gateway_command(command.command);
    if (command.contract_version != station_governance_contract_version ||
        command.decision_id.empty() || command.station_id.empty() || command.gateway_id.empty() ||
        command.source_measurement_id.empty()) {
        invalid(core::ErrorCode::data_invalid, "Governed command is incomplete");
    }
}

core::SimulationClock::Duration valid_until(const BanFrame& frame) {
    if (frame.created_at > core::SimulationClock::Duration::max() - frame.valid_for) {
        invalid(core::ErrorCode::numeric_overflow, "BAN frame validity overflows");
    }
    return frame.created_at + frame.valid_for;
}

void validate_ban_frame(const BanFrame& frame) {
    const auto common_valid =
        frame.contract_version == ban_frame_contract_version && !frame.frame_id.empty() &&
        !frame.source_endpoint_id.empty() && !frame.target_endpoint_id.empty() &&
        frame.source_endpoint_id != frame.target_endpoint_id && !frame.correlation_id.empty() &&
        !frame.source_event_id.empty() &&
        frame.created_at >= core::SimulationClock::Duration::zero() &&
        frame.valid_for > core::SimulationClock::Duration::zero() && frame.size_bytes > 0;
    if (!common_valid) {
        invalid(core::ErrorCode::data_invalid, "BAN frame is incomplete or inconsistent");
    }
    static_cast<void>(valid_until(frame));

    if (frame.kind == BanFrameKind::measurement_uplink) {
        if (!std::holds_alternative<GatewayMeasurement>(frame.payload)) {
            invalid(core::ErrorCode::data_invalid, "Measurement BAN frame has wrong payload kind");
        }
        const auto& measurement = std::get<GatewayMeasurement>(frame.payload);
        validate_gateway_measurement(measurement);
        if (measurement.gateway_id != frame.source_endpoint_id ||
            measurement.correlation_id != frame.correlation_id ||
            measurement.source_event_id != frame.source_event_id ||
            measurement.observed_at != frame.created_at ||
            measurement.size_bytes != frame.size_bytes) {
            invalid(core::ErrorCode::data_invalid,
                    "Measurement BAN frame does not preserve identity");
        }
        return;
    }

    if (!std::holds_alternative<GovernedGatewayCommand>(frame.payload)) {
        invalid(core::ErrorCode::data_invalid, "Command BAN frame has wrong payload kind");
    }
    const auto& governed = std::get<GovernedGatewayCommand>(frame.payload);
    validate_governed_gateway_command(governed);
    if (governed.station_id != frame.source_endpoint_id ||
        governed.gateway_id != frame.target_endpoint_id ||
        governed.command.correlation_id != frame.correlation_id ||
        governed.decision_id != frame.source_event_id ||
        governed.command.created_at != frame.created_at ||
        governed.command.valid_for != frame.valid_for ||
        governed.command.size_bytes != frame.size_bytes) {
        invalid(core::ErrorCode::data_invalid,
                "Command BAN frame does not preserve governance identity");
    }
}

void validate_gateway_ban_adapter_config(const GatewayBanAdapterConfig& config) {
    if (config.adapter_id.empty() || config.gateway_id.empty() || config.station_id.empty() ||
        config.gateway_id == config.station_id || config.measurement_frame_id_prefix.empty() ||
        config.maximum_uplink_frames == 0 || config.maximum_downlink_frames == 0) {
        invalid(core::ErrorCode::data_invalid,
                "Gateway BAN adapter requires identities and capacities");
    }
}

GatewayBanAdapter::GatewayBanAdapter(GatewayBanAdapterConfig config) : config_{std::move(config)} {
    validate_gateway_ban_adapter_config(config_);
}

BanFrame GatewayBanAdapter::publish_measurement(const GatewayMeasurement& measurement,
                                                const core::SimulationClock::Duration valid_for) {
    validate_gateway_measurement(measurement);
    if (measurement.gateway_id != config_.gateway_id ||
        valid_for <= core::SimulationClock::Duration::zero() ||
        uplink_count_ >= config_.maximum_uplink_frames ||
        published_measurements_.contains(measurement.measurement_id)) {
        invalid(core::ErrorCode::invariant_violated, "Gateway cannot publish this BAN measurement");
    }
    BanFrame frame{std::string{ban_frame_contract_version},
                   config_.measurement_frame_id_prefix + "." + measurement.measurement_id,
                   BanFrameKind::measurement_uplink,
                   config_.gateway_id,
                   config_.station_id,
                   measurement.correlation_id,
                   measurement.source_event_id,
                   measurement.observed_at,
                   valid_for,
                   measurement.size_bytes,
                   measurement};
    validate_ban_frame(frame);
    published_measurements_.emplace(measurement.measurement_id, measurement.correlation_id);
    ++uplink_count_;
    return frame;
}

GatewayCommand
GatewayBanAdapter::accept_command(const BanFrame& frame,
                                  const core::SimulationClock::Duration received_at) {
    validate_ban_frame(frame);
    if (frame.kind != BanFrameKind::governed_command_downlink ||
        frame.source_endpoint_id != config_.station_id ||
        frame.target_endpoint_id != config_.gateway_id || received_at < frame.created_at ||
        received_at >= valid_until(frame) || downlink_count_ >= config_.maximum_downlink_frames) {
        invalid(core::ErrorCode::data_invalid, "Gateway rejected BAN command boundary conditions");
    }
    const auto& governed = std::get<GovernedGatewayCommand>(frame.payload);
    const auto measurement = published_measurements_.find(governed.source_measurement_id);
    if (measurement == published_measurements_.end() ||
        measurement->second != frame.correlation_id ||
        accepted_decision_ids_.contains(governed.decision_id)) {
        invalid(core::ErrorCode::invariant_violated,
                "Gateway rejected untraceable or replayed command");
    }
    accepted_decision_ids_.insert(governed.decision_id);
    ++downlink_count_;
    auto accepted = governed.command;
    accepted.created_at = received_at;
    accepted.valid_for = valid_until(frame) - received_at;
    validate_gateway_command(accepted);
    return accepted;
}

std::uint64_t GatewayBanAdapter::uplink_count() const noexcept { return uplink_count_; }
std::uint64_t GatewayBanAdapter::downlink_count() const noexcept { return downlink_count_; }

void validate_external_station_config(const ExternalStationConfig& config) {
    if (config.station_id.empty() || config.decision_id_prefix.empty() ||
        config.command_frame_id_prefix.empty() || config.accepted_gateway_ids.empty() ||
        config.allowed_target_device_ids.empty() || config.allowed_command_content_types.empty() ||
        config.maximum_measurements == 0 || config.maximum_approved_commands == 0) {
        invalid(core::ErrorCode::data_invalid, "External station requires policy and capacities");
    }
    require_unique_nonempty(config.accepted_gateway_ids);
    require_unique_nonempty(config.allowed_target_device_ids);
    require_unique_nonempty(config.allowed_command_content_types);
    if (contains(config.accepted_gateway_ids, config.station_id)) {
        invalid(core::ErrorCode::data_invalid, "Station cannot accept itself as a gateway");
    }
}

ExternalAnalysisControlStation::ExternalAnalysisControlStation(ExternalStationConfig config)
    : config_{std::move(config)} {
    validate_external_station_config(config_);
}

void ExternalAnalysisControlStation::receive_measurement(
    const BanFrame& frame, const core::SimulationClock::Duration received_at) {
    validate_ban_frame(frame);
    if (frame.kind != BanFrameKind::measurement_uplink ||
        frame.target_endpoint_id != config_.station_id ||
        !contains(config_.accepted_gateway_ids, frame.source_endpoint_id) ||
        received_at < frame.created_at || received_at > valid_until(frame) ||
        measurements_.size() >= config_.maximum_measurements) {
        invalid(core::ErrorCode::data_invalid,
                "Station rejected BAN measurement boundary conditions");
    }
    const auto& measurement = std::get<GatewayMeasurement>(frame.payload);
    if (!measurements_.emplace(measurement.measurement_id, measurement).second) {
        invalid(core::ErrorCode::invariant_violated, "Station rejected duplicate measurement");
    }
    measurement_received_at_.emplace(measurement.measurement_id, received_at);
}

StationCommandDecision
ExternalAnalysisControlStation::evaluate_command(const StationCommandRequest& request) {
    validate_station_command_request(request);
    if (!request_ids_.insert(request.request_id).second) {
        return denied(CommandGovernanceStatus::denied_duplicate_request,
                      "request identity was already evaluated");
    }
    const auto measurement = measurements_.find(request.source_measurement_id);
    if (measurement == measurements_.end()) {
        return denied(CommandGovernanceStatus::denied_unknown_measurement,
                      "source measurement was not received by this station");
    }
    if (request.command.created_at < measurement_received_at_.at(request.source_measurement_id)) {
        return denied(CommandGovernanceStatus::denied_time_order,
                      "command predates receipt of its source measurement");
    }
    if (measurement->second.correlation_id != request.command.correlation_id) {
        return denied(CommandGovernanceStatus::denied_correlation_mismatch,
                      "command correlation does not match its source measurement");
    }
    if (!contains(config_.allowed_target_device_ids, request.command.target_device_id)) {
        return denied(CommandGovernanceStatus::denied_target,
                      "target device is outside the configured allow-list");
    }
    if (!contains(config_.allowed_command_content_types, request.command.content_type)) {
        return denied(CommandGovernanceStatus::denied_content_type,
                      "command content type is outside the configured allow-list");
    }
    if (approved_command_count_ >= config_.maximum_approved_commands) {
        return denied(CommandGovernanceStatus::denied_capacity,
                      "approved-command capacity is exhausted");
    }

    GovernedGatewayCommand governed{std::string{station_governance_contract_version},
                                    config_.decision_id_prefix + "." + request.request_id,
                                    config_.station_id,
                                    measurement->second.gateway_id,
                                    request.source_measurement_id,
                                    request.command};
    validate_governed_gateway_command(governed);
    ++approved_command_count_;
    return {CommandGovernanceStatus::approved, "configured transport policy approved the command",
            std::move(governed)};
}

BanFrame ExternalAnalysisControlStation::prepare_command_frame(
    const StationCommandDecision& decision) const {
    if (decision.status != CommandGovernanceStatus::approved || !decision.approved_command) {
        invalid(core::ErrorCode::invariant_violated, "Only an approved decision can enter the BAN");
    }
    const auto& governed = *decision.approved_command;
    validate_governed_gateway_command(governed);
    BanFrame frame{std::string{ban_frame_contract_version},
                   config_.command_frame_id_prefix + "." + governed.decision_id,
                   BanFrameKind::governed_command_downlink,
                   config_.station_id,
                   governed.gateway_id,
                   governed.command.correlation_id,
                   governed.decision_id,
                   governed.command.created_at,
                   governed.command.valid_for,
                   governed.command.size_bytes,
                   governed};
    validate_ban_frame(frame);
    return frame;
}

std::uint64_t ExternalAnalysisControlStation::measurement_count() const noexcept {
    return measurements_.size();
}

std::uint64_t ExternalAnalysisControlStation::approved_command_count() const noexcept {
    return approved_command_count_;
}

void validate_scheduled_ban_transport_config(const ScheduledBanTransportConfig& config) {
    if (config.adapter_id.empty() || config.latency < core::SimulationClock::Duration::zero() ||
        !nonnegative_finite(config.transmitter_energy_per_attempt) ||
        !nonnegative_finite(config.receiver_energy_per_delivery) ||
        !nonnegative_finite(config.link_energy_per_attempt) || config.repeating_outcomes.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Scheduled BAN adapter requires identity, costs, and outcomes");
    }
    if (std::ranges::any_of(config.repeating_outcomes, [](const auto outcome) {
            return outcome != ScheduledLinkOutcome::delivered &&
                   outcome != ScheduledLinkOutcome::lost &&
                   outcome != ScheduledLinkOutcome::corrupted;
        })) {
        invalid(core::ErrorCode::data_invalid, "Scheduled BAN adapter has an unknown outcome");
    }
}

ScheduledBanTransportAdapter::ScheduledBanTransportAdapter(ScheduledBanTransportConfig config)
    : config_{std::move(config)} {
    validate_scheduled_ban_transport_config(config_);
}

std::string_view ScheduledBanTransportAdapter::kind() const noexcept {
    return scheduled_ban_transport_kind;
}

std::string_view ScheduledBanTransportAdapter::adapter_id() const noexcept {
    return config_.adapter_id;
}

BanTransferResult ScheduledBanTransportAdapter::transfer(const BanFrame& frame) {
    validate_ban_frame(frame);
    if (frame.created_at > core::SimulationClock::Duration::max() - config_.latency) {
        invalid(core::ErrorCode::numeric_overflow, "BAN completion time overflows");
    }
    if (attempt_count_ == std::numeric_limits<std::uint64_t>::max()) {
        invalid(core::ErrorCode::numeric_overflow, "BAN attempt counter overflows");
    }
    const auto completed_at = frame.created_at + config_.latency;
    const auto scheduled =
        config_.repeating_outcomes[attempt_count_ % config_.repeating_outcomes.size()];
    ++attempt_count_;
    auto status = OneHopDeliveryStatus::dropped;
    auto reason = OneHopDropReason::none;
    if (completed_at > valid_until(frame)) {
        reason = OneHopDropReason::expired;
    } else if (scheduled == ScheduledLinkOutcome::delivered) {
        status = OneHopDeliveryStatus::delivered;
    } else if (scheduled == ScheduledLinkOutcome::lost) {
        reason = OneHopDropReason::loss;
    } else {
        reason = OneHopDropReason::corruption;
    }
    return {config_.adapter_id,
            frame.frame_id,
            status,
            reason,
            frame.created_at,
            completed_at,
            config_.latency,
            frame.size_bytes,
            config_.transmitter_energy_per_attempt,
            status == OneHopDeliveryStatus::delivered ? config_.receiver_energy_per_delivery
                                                      : core::joules(0.0),
            config_.link_energy_per_attempt};
}

BanCommunicationSession::BanCommunicationSession(BanTransportAdapter& adapter) noexcept
    : adapter_{adapter} {}

BanTransferResult BanCommunicationSession::exchange(const BanFrame& frame) {
    auto result = adapter_.transfer(frame);
    const auto status_consistent = (result.status == OneHopDeliveryStatus::delivered &&
                                    result.drop_reason == OneHopDropReason::none) ||
                                   (result.status == OneHopDeliveryStatus::dropped &&
                                    result.drop_reason != OneHopDropReason::none);
    if (result.adapter_id != adapter_.adapter_id() || result.frame_id != frame.frame_id ||
        result.departed_at != frame.created_at || result.completed_at < result.departed_at ||
        result.latency != result.completed_at - result.departed_at ||
        result.size_bytes != frame.size_bytes || !nonnegative_finite(result.transmitter_energy) ||
        !nonnegative_finite(result.receiver_energy) || !nonnegative_finite(result.link_energy) ||
        !status_consistent) {
        invalid(core::ErrorCode::invariant_violated, "BAN adapter returned an inconsistent result");
    }

    add_count(metrics_.attempted_messages, 1);
    add_count(metrics_.attempted_bytes, result.size_bytes);
    add_energy(metrics_.transmitter_energy, result.transmitter_energy);
    add_energy(metrics_.receiver_energy, result.receiver_energy);
    add_energy(metrics_.link_energy, result.link_energy);
    if (result.status == OneHopDeliveryStatus::delivered) {
        add_count(metrics_.delivered_messages, 1);
        add_count(metrics_.delivered_bytes, result.size_bytes);
        add_duration(metrics_.total_delivery_latency, result.latency);
        metrics_.maximum_delivery_latency =
            std::max(metrics_.maximum_delivery_latency, result.latency);
    } else if (result.drop_reason == OneHopDropReason::loss) {
        add_count(metrics_.lost_messages, 1);
    } else if (result.drop_reason == OneHopDropReason::corruption) {
        add_count(metrics_.corrupted_messages, 1);
    } else {
        add_count(metrics_.expired_messages, 1);
    }
    return result;
}

const CommunicationMetrics& BanCommunicationSession::metrics() const noexcept { return metrics_; }

} // namespace mehlissa::models::iot

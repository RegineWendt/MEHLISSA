// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/active_gateway.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <limits>
#include <ranges>
#include <utility>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool accepts_kind(const ActiveGatewayConfig& config,
                                const LocalMessageKind kind) noexcept {
    return std::ranges::find(config.accepted_uplink_kinds, kind) !=
           config.accepted_uplink_kinds.end();
}

} // namespace

void validate_gateway_measurement(const GatewayMeasurement& measurement) {
    if (measurement.contract_version != gateway_measurement_contract_version ||
        measurement.measurement_id.empty() || measurement.gateway_id.empty() ||
        measurement.source_message_id.empty() || measurement.source_device_id.empty() ||
        measurement.correlation_id.empty() || measurement.source_event_id.empty() ||
        measurement.observed_at < core::SimulationClock::Duration::zero() ||
        measurement.size_bytes == 0 || measurement.content_type.empty() ||
        measurement.content.empty() || measurement.content.size() > measurement.size_bytes) {
        invalid(core::ErrorCode::data_invalid,
                "Gateway measurement is incomplete or internally inconsistent");
    }
}

void validate_gateway_command(const GatewayCommand& command) {
    if (command.contract_version != gateway_command_contract_version ||
        command.command_id.empty() || command.target_device_id.empty() ||
        command.correlation_id.empty() ||
        command.created_at < core::SimulationClock::Duration::zero() ||
        command.valid_for <= core::SimulationClock::Duration::zero() || command.hop_limit == 0 ||
        command.size_bytes == 0 || command.content_type.empty() || command.content.empty() ||
        command.content.size() > command.size_bytes ||
        command.created_at > core::SimulationClock::Duration::max() - command.valid_for) {
        invalid(core::ErrorCode::data_invalid,
                "Gateway command is incomplete or internally inconsistent");
    }
}

void validate_active_gateway_config(const ActiveGatewayConfig& config) {
    if (config.gateway_id.empty() || config.endpoint_profile_id.empty() ||
        config.measurement_id_prefix.empty() || config.command_message_id_prefix.empty() ||
        config.accepted_uplink_kinds.empty() || config.maximum_measurements == 0 ||
        config.maximum_commands == 0) {
        invalid(core::ErrorCode::data_invalid,
                "Active gateway requires identities, accepted kinds, and bounded capacities");
    }

    std::vector<LocalMessageKind> unique_kinds;
    for (const auto kind : config.accepted_uplink_kinds) {
        if ((kind != LocalMessageKind::detection && kind != LocalMessageKind::measurement) ||
            std::ranges::find(unique_kinds, kind) != unique_kinds.end()) {
            invalid(core::ErrorCode::data_invalid,
                    "Gateway uplink kinds must be unique detection or measurement values");
        }
        unique_kinds.push_back(kind);
    }
}

ActiveGateway::ActiveGateway(ActiveGatewayConfig config, NanodeviceConfig endpoint_config,
                             const std::string_view endpoint_profile_id)
    : config_{std::move(config)}, endpoint_{std::move(endpoint_config)} {
    validate_active_gateway_config(config_);
    if (endpoint_profile_id != config_.endpoint_profile_id ||
        endpoint_.device_id() != config_.gateway_id ||
        !endpoint_.has_capability(NanodeviceCapability::receive) ||
        !endpoint_.has_capability(NanodeviceCapability::transmit) ||
        !endpoint_.has_capability(NanodeviceCapability::collect)) {
        invalid(core::ErrorCode::data_invalid,
                "Gateway endpoint must match the gateway and receive, collect, and transmit");
    }
}

std::string_view ActiveGateway::gateway_id() const noexcept { return config_.gateway_id; }

Nanodevice& ActiveGateway::endpoint() noexcept { return endpoint_; }

const Nanodevice& ActiveGateway::endpoint() const noexcept { return endpoint_; }

std::uint64_t ActiveGateway::measurement_count() const noexcept { return measurement_count_; }

std::uint64_t ActiveGateway::command_count() const noexcept { return command_count_; }

GatewayMeasurement
ActiveGateway::publish_next_measurement(const core::SimulationClock::Duration observed_at) {
    if (measurement_count_ >= config_.maximum_measurements) {
        invalid(core::ErrorCode::invariant_violated,
                "Gateway measurement publication capacity is exhausted");
    }
    if (endpoint_.received_messages().size() != 1) {
        invalid(core::ErrorCode::invariant_violated,
                "Gateway publication requires exactly one buffered local message");
    }
    const auto& message = endpoint_.received_messages().front();
    if (!accepts_kind(config_, message.kind) || observed_at < message.created_at ||
        observed_at > valid_until(message)) {
        invalid(core::ErrorCode::data_invalid,
                "Buffered message kind or gateway observation time is invalid");
    }
    if (measurement_count_ == std::numeric_limits<std::uint64_t>::max()) {
        invalid(core::ErrorCode::numeric_overflow, "Gateway measurement counter overflows");
    }

    GatewayMeasurement measurement{std::string{gateway_measurement_contract_version},
                                   config_.measurement_id_prefix + "." + message.message_id,
                                   config_.gateway_id,
                                   message.message_id,
                                   message.source_device_id,
                                   message.correlation_id,
                                   message.source_event_id,
                                   observed_at,
                                   message.size_bytes,
                                   message.content_type,
                                   message.content};
    validate_gateway_measurement(measurement);
    static_cast<void>(endpoint_.take_received_messages());
    ++measurement_count_;
    return measurement;
}

LocalMessageRequest ActiveGateway::prepare_downlink(const GatewayCommand& command) {
    validate_gateway_command(command);
    if (command.target_device_id == config_.gateway_id ||
        command_count_ >= config_.maximum_commands || command_ids_.contains(command.command_id)) {
        invalid(core::ErrorCode::invariant_violated,
                "Gateway command target, capacity, or identity is invalid");
    }
    if (command_count_ == std::numeric_limits<std::uint64_t>::max()) {
        invalid(core::ErrorCode::numeric_overflow, "Gateway command counter overflows");
    }

    LocalMessageRequest request{config_.command_message_id_prefix + "." + command.command_id,
                                LocalMessageKind::control,
                                command.target_device_id,
                                command.correlation_id,
                                command.command_id,
                                command.created_at,
                                command.valid_for,
                                command.hop_limit,
                                command.size_bytes,
                                command.content_type,
                                command.content};
    validate_local_message_request(request);
    command_ids_.insert(command.command_id);
    ++command_count_;
    return request;
}

} // namespace mehlissa::models::iot

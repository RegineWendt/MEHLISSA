// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/network_simulator_adapter.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>

#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <string>
#include <utility>

namespace mehlissa::models::iot {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool nonnegative_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) >= 0.0;
}

[[nodiscard]] NetworkSimulationOutcome decode_outcome(const std::string_view value) {
    if (value == "delivered") {
        return NetworkSimulationOutcome::delivered;
    }
    if (value == "lost") {
        return NetworkSimulationOutcome::lost;
    }
    if (value == "corrupted") {
        return NetworkSimulationOutcome::corrupted;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown network-simulation outcome");
}

void require_strict_response_object(const Json& document) {
    constexpr std::array<std::string_view, 12> required{
        "contract_version",  "request_id",           "adapter_id",        "simulator_id",
        "simulator_version", "scenario_id",          "frame_id",          "outcome",
        "completed_at_ns",   "transmitter_energy_j", "receiver_energy_j", "link_energy_j"};
    if (!document.is_object() || document.size() != required.size()) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulation response must contain exactly the versioned fields");
    }
    for (const auto field : required) {
        if (!document.contains(field)) {
            invalid(core::ErrorCode::data_invalid,
                    "Network-simulation response is missing a required field");
        }
    }
}

[[nodiscard]] OneHopDropReason drop_reason(const NetworkSimulationOutcome outcome) noexcept {
    if (outcome == NetworkSimulationOutcome::lost) {
        return OneHopDropReason::loss;
    }
    if (outcome == NetworkSimulationOutcome::corrupted) {
        return OneHopDropReason::corruption;
    }
    return OneHopDropReason::none;
}

} // namespace

std::string_view to_string(const NetworkSimulationOutcome outcome) noexcept {
    switch (outcome) {
    case NetworkSimulationOutcome::delivered:
        return "delivered";
    case NetworkSimulationOutcome::lost:
        return "lost";
    case NetworkSimulationOutcome::corrupted:
        return "corrupted";
    }
    return "unknown";
}

void validate_network_simulation_request(const NetworkSimulationRequest& request) {
    if (request.contract_version != network_simulation_request_contract_version ||
        request.request_id.empty() || request.adapter_id.empty() || request.simulator_id.empty() ||
        request.simulator_version.empty() || request.scenario_id.empty() ||
        request.frame_id.empty() || request.source_endpoint_id.empty() ||
        request.target_endpoint_id.empty() ||
        request.source_endpoint_id == request.target_endpoint_id ||
        request.correlation_id.empty() || request.source_event_id.empty() ||
        (request.frame_kind != BanFrameKind::measurement_uplink &&
         request.frame_kind != BanFrameKind::governed_command_downlink) ||
        request.departed_at < core::SimulationClock::Duration::zero() ||
        request.valid_until <= request.departed_at || request.size_bytes == 0) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulation request is incomplete or internally inconsistent");
    }
}

void validate_network_simulation_response(const NetworkSimulationResponse& response) {
    if (response.contract_version != network_simulation_response_contract_version ||
        response.request_id.empty() || response.adapter_id.empty() ||
        response.simulator_id.empty() || response.simulator_version.empty() ||
        response.scenario_id.empty() || response.frame_id.empty() ||
        response.completed_at < core::SimulationClock::Duration::zero() ||
        !nonnegative_finite(response.transmitter_energy) ||
        !nonnegative_finite(response.receiver_energy) ||
        !nonnegative_finite(response.link_energy)) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulation response is incomplete or internally inconsistent");
    }
    if (response.outcome != NetworkSimulationOutcome::delivered &&
        response.outcome != NetworkSimulationOutcome::lost &&
        response.outcome != NetworkSimulationOutcome::corrupted) {
        invalid(core::ErrorCode::data_invalid, "Network-simulation response has unknown outcome");
    }
}

// jsoncons ownership is RAII-managed; the MSVC-bundled analyzer reports a false leak while
// following the object through serialization and the injected exchange call.
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
std::string encode_network_simulation_request_json(const NetworkSimulationRequest& request) {
    validate_network_simulation_request(request);
    const Json document{
        jsoncons::json_object_arg,
        {{"contract_version", request.contract_version},
         {"request_id", request.request_id},
         {"adapter_id", request.adapter_id},
         {"simulator_id", request.simulator_id},
         {"simulator_version", request.simulator_version},
         {"scenario_id", request.scenario_id},
         {"frame_id", request.frame_id},
         {"frame_kind", std::string{to_string(request.frame_kind)}},
         {"source_endpoint_id", request.source_endpoint_id},
         {"target_endpoint_id", request.target_endpoint_id},
         {"correlation_id", request.correlation_id},
         {"source_event_id", request.source_event_id},
         {"departed_at_ns", request.departed_at.count()},
         {"valid_until_ns", request.valid_until.count()},
         {"size_bytes", request.size_bytes}},
    };
    std::string encoded;
    jsoncons::encode_json(document, encoded);
    return encoded;
}

NetworkSimulationResponse
decode_network_simulation_response_json(const std::string_view response_json) {
    try {
        const auto document = Json::parse(response_json);
        require_strict_response_object(document);
        NetworkSimulationResponse response{
            document.at("contract_version").as<std::string>(),
            document.at("request_id").as<std::string>(),
            document.at("adapter_id").as<std::string>(),
            document.at("simulator_id").as<std::string>(),
            document.at("simulator_version").as<std::string>(),
            document.at("scenario_id").as<std::string>(),
            document.at("frame_id").as<std::string>(),
            decode_outcome(document.at("outcome").as<std::string_view>()),
            core::SimulationClock::Duration{document.at("completed_at_ns").as<std::int64_t>()},
            core::joules(document.at("transmitter_energy_j").as<double>()),
            core::joules(document.at("receiver_energy_j").as<double>()),
            core::joules(document.at("link_energy_j").as<double>())};
        validate_network_simulation_response(response);
        return response;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulation response JSON is invalid: " + std::string{error.what()});
    }
}

JsonNetworkSimulatorClient::JsonNetworkSimulatorClient(
    NetworkSimulatorJsonExchange& exchange) noexcept
    : exchange_{exchange} {}

NetworkSimulationResponse
JsonNetworkSimulatorClient::simulate(const NetworkSimulationRequest& request) {
    return decode_network_simulation_response_json(
        exchange_.exchange(encode_network_simulation_request_json(request)));
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

void validate_external_network_simulator_adapter_config(
    const ExternalNetworkSimulatorAdapterConfig& config) {
    if (config.adapter_id.empty() || config.simulator_id.empty() ||
        config.simulator_version.empty() || config.scenario_id.empty() ||
        config.request_id_prefix.empty() || config.maximum_attempts == 0) {
        invalid(core::ErrorCode::data_invalid,
                "External network-simulator adapter requires identities and bounded attempts");
    }
}

ExternalNetworkSimulatorAdapter::ExternalNetworkSimulatorAdapter(
    ExternalNetworkSimulatorAdapterConfig config, NetworkSimulatorClient& client)
    : config_{std::move(config)}, client_{client} {
    validate_external_network_simulator_adapter_config(config_);
}

std::string_view ExternalNetworkSimulatorAdapter::kind() const noexcept {
    return external_network_simulator_adapter_kind;
}

std::string_view ExternalNetworkSimulatorAdapter::adapter_id() const noexcept {
    return config_.adapter_id;
}

BanTransferResult ExternalNetworkSimulatorAdapter::transfer(const BanFrame& frame) {
    validate_ban_frame(frame);
    if (attempt_count_ >= config_.maximum_attempts ||
        attempt_count_ == std::numeric_limits<std::uint64_t>::max()) {
        invalid(core::ErrorCode::invariant_violated,
                "External network-simulator attempt capacity is exhausted");
    }
    const auto next_attempt = attempt_count_ + 1;
    NetworkSimulationRequest request{std::string{network_simulation_request_contract_version},
                                     config_.request_id_prefix + "." + frame.frame_id + "." +
                                         std::to_string(next_attempt),
                                     config_.adapter_id,
                                     config_.simulator_id,
                                     config_.simulator_version,
                                     config_.scenario_id,
                                     frame.frame_id,
                                     frame.kind,
                                     frame.source_endpoint_id,
                                     frame.target_endpoint_id,
                                     frame.correlation_id,
                                     frame.source_event_id,
                                     frame.created_at,
                                     valid_until(frame),
                                     frame.size_bytes};
    validate_network_simulation_request(request);
    ++attempt_count_;
    const auto response = client_.simulate(request);
    validate_network_simulation_response(response);
    if (response.request_id != request.request_id || response.adapter_id != config_.adapter_id ||
        response.simulator_id != config_.simulator_id ||
        response.simulator_version != config_.simulator_version ||
        response.scenario_id != config_.scenario_id || response.frame_id != frame.frame_id ||
        response.completed_at < frame.created_at) {
        invalid(core::ErrorCode::invariant_violated,
                "External network simulator returned mismatched identity or time");
    }

    auto status = OneHopDeliveryStatus::dropped;
    auto reason = drop_reason(response.outcome);
    if (response.completed_at > valid_until(frame)) {
        reason = OneHopDropReason::expired;
    } else if (response.outcome == NetworkSimulationOutcome::delivered) {
        status = OneHopDeliveryStatus::delivered;
    }
    return {config_.adapter_id,
            frame.frame_id,
            status,
            reason,
            frame.created_at,
            response.completed_at,
            response.completed_at - frame.created_at,
            frame.size_bytes,
            response.transmitter_energy,
            response.receiver_energy,
            response.link_energy};
}

std::uint64_t ExternalNetworkSimulatorAdapter::attempt_count() const noexcept {
    return attempt_count_;
}

} // namespace mehlissa::models::iot

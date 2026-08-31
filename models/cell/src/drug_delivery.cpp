// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/drug_delivery.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

namespace mehlissa::models::cell {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double seconds(const core::SimulationClock::Duration duration) noexcept {
    return std::chrono::duration<double>{duration}.count();
}

template <typename Response>
[[nodiscard]] std::optional<NanodeviceActivationSignal>
activation_signal(const Response& response, const NanodeviceActivationTarget& target) {
    if (response.response_threshold_reached != response.first_response_time.has_value()) {
        invalid("Intracellular response has an inconsistent threshold event");
    }
    if (!response.response_threshold_reached) {
        return std::nullopt;
    }
    NanodeviceActivationSignal signal{std::string{nanodevice_activation_contract_version},
                                      target.activation_id,
                                      target.nanodevice_id,
                                      target.payload_id,
                                      response.request_id,
                                      response.network_id,
                                      response.first_response_time.value()};
    validate_nanodevice_activation_signal(signal);
    return signal;
}

[[nodiscard]] bool positive_finite(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

} // namespace

void validate_nanodevice_activation_signal(const NanodeviceActivationSignal& signal) {
    if (signal.contract_version != nanodevice_activation_contract_version ||
        signal.activation_id.empty() || signal.nanodevice_id.empty() || signal.payload_id.empty() ||
        signal.source_request_id.empty() || signal.source_network_id.empty() ||
        signal.trigger_offset < core::SimulationClock::Duration::zero()) {
        invalid("Nanodevice activation signal is incomplete or invalid");
    }
}

std::optional<NanodeviceActivationSignal>
make_nanodevice_activation_signal(const IntracellularOdeResponse& response,
                                  const NanodeviceActivationTarget& target) {
    return activation_signal(response, target);
}

std::optional<NanodeviceActivationSignal>
make_nanodevice_activation_signal(const IntracellularSsaResponse& response,
                                  const NanodeviceActivationTarget& target) {
    return activation_signal(response, target);
}

AnalyticalDrugDeliveryModel::AnalyticalDrugDeliveryModel(DrugDeliveryConfig config)
    : config_{std::move(config)} {
    const auto loaded = core::in_moles(config_.loaded_amount);
    const auto release = core::in_per_second(config_.release_rate);
    const auto uptake = core::in_per_second(config_.uptake_rate);
    if (config_.model_id.empty() || config_.nanodevice_id.empty() || config_.payload_id.empty() ||
        config_.drug_id.empty() || !positive_finite(loaded) || !positive_finite(release) ||
        !positive_finite(uptake)) {
        invalid("Drug-delivery configuration is incomplete or nonphysical");
    }
}

std::string_view AnalyticalDrugDeliveryModel::kind() const noexcept {
    return analytical_conservative_drug_delivery_kind;
}

DrugDeliveryResponse
AnalyticalDrugDeliveryModel::evaluate(const DrugDeliveryRequest& request) const {
    if (request.request_id.empty() ||
        request.observation_after_activation <= core::SimulationClock::Duration::zero()) {
        invalid("Drug-delivery request is incomplete or nonphysical");
    }
    const auto loaded = core::in_moles(config_.loaded_amount);
    if (!request.activation.has_value()) {
        return {request.request_id,    config_.model_id,
                config_.drug_id,       false,
                std::nullopt,          request.observation_after_activation,
                config_.loaded_amount, config_.loaded_amount,
                core::moles(0.0),      core::moles(0.0),
                core::moles(0.0),      core::moles(0.0)};
    }

    const auto& activation = request.activation.value();
    validate_nanodevice_activation_signal(activation);
    if (activation.nanodevice_id != config_.nanodevice_id ||
        activation.payload_id != config_.payload_id) {
        invalid("Nanodevice activation does not address this delivery model");
    }

    const auto duration = seconds(request.observation_after_activation);
    const auto release_rate = core::in_per_second(config_.release_rate);
    const auto uptake_rate = core::in_per_second(config_.uptake_rate);
    const auto device = loaded * std::exp(-release_rate * duration);
    double extracellular = 0.0;
    const auto rate_scale = std::max(release_rate, uptake_rate);
    if (std::abs(release_rate - uptake_rate) <=
        std::numeric_limits<double>::epsilon() * rate_scale * 8.0) {
        extracellular = loaded * release_rate * duration * std::exp(-release_rate * duration);
    } else {
        extracellular = loaded * release_rate / (uptake_rate - release_rate) *
                        (std::exp(-release_rate * duration) - std::exp(-uptake_rate * duration));
    }
    const auto intracellular = loaded - device - extracellular;
    const auto released = loaded - device;
    const auto balance_error = loaded - device - extracellular - intracellular;
    const auto tolerance = loaded * 64.0 * std::numeric_limits<double>::epsilon();
    if (!std::isfinite(device) || !std::isfinite(extracellular) || !std::isfinite(intracellular) ||
        device < -tolerance || extracellular < -tolerance || intracellular < -tolerance ||
        std::abs(balance_error) > tolerance) {
        invalid("Drug-delivery result violates non-negativity or amount conservation");
    }

    return {request.request_id,
            config_.model_id,
            config_.drug_id,
            true,
            activation.trigger_offset,
            request.observation_after_activation,
            config_.loaded_amount,
            core::moles(device),
            core::moles(extracellular),
            core::moles(intracellular),
            core::moles(released),
            core::moles(balance_error)};
}

} // namespace mehlissa::models::cell

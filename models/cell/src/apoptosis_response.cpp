// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/apoptosis_response.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <utility>

namespace mehlissa::models::cell {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] bool fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

std::string_view to_string(const CellState state) noexcept {
    switch (state) {
    case CellState::viable:
        return "viable";
    case CellState::apoptosis_committed:
        return "apoptosis_committed";
    }
    return "unknown";
}

double synthetic_hill_effect(const core::Amount intracellular_amount,
                             const SyntheticHillParameters& parameters) {
    const auto amount = core::in_moles(intracellular_amount);
    const auto half_max = core::in_moles(parameters.half_max_effect_amount);
    if (!std::isfinite(amount) || amount < 0.0 || !std::isfinite(half_max) || half_max <= 0.0 ||
        !std::isfinite(parameters.hill_coefficient) || parameters.hill_coefficient <= 0.0) {
        invalid("Synthetic Hill-effect inputs are nonphysical");
    }
    if (amount == 0.0) {
        return 0.0;
    }
    const auto log_ratio = parameters.hill_coefficient * std::log(amount / half_max);
    if (log_ratio >= 0.0) {
        return 1.0 / (1.0 + std::exp(-log_ratio));
    }
    const auto ratio = std::exp(log_ratio);
    return ratio / (1.0 + ratio);
}

void validate_apoptosis_response(const ApoptosisResponse& response) {
    const auto amount = core::in_moles(response.intracellular_drug_amount);
    const auto valid_state =
        response.state == CellState::viable || response.state == CellState::apoptosis_committed;
    if (response.request_id.empty() || response.model_id.empty() || response.cell_id.empty() ||
        response.drug_id.empty() || response.source_delivery_request_id.empty() ||
        response.source_delivery_model_id.empty() ||
        response.observed_at < core::SimulationClock::Duration::zero() || !std::isfinite(amount) ||
        amount < 0.0 || !fraction(response.effect_fraction) || !valid_state ||
        (!response.delivery_activated && (amount != 0.0 || response.effect_fraction != 0.0 ||
                                          response.state != CellState::viable))) {
        invalid("Apoptosis response is incomplete or inconsistent");
    }
}

SyntheticHillApoptosisModel::SyntheticHillApoptosisModel(ApoptosisResponseConfig config)
    : config_{std::move(config)} {
    if (config_.model_id.empty() || config_.cell_id.empty() || config_.drug_id.empty() ||
        !std::isfinite(core::in_moles(config_.half_max_effect_amount)) ||
        core::in_moles(config_.half_max_effect_amount) <= 0.0 ||
        !std::isfinite(config_.hill_coefficient) || config_.hill_coefficient <= 0.0 ||
        !std::isfinite(config_.apoptosis_commitment_threshold) ||
        config_.apoptosis_commitment_threshold <= 0.0 ||
        config_.apoptosis_commitment_threshold >= 1.0) {
        invalid("Apoptosis-response configuration is incomplete or nonphysical");
    }
}

std::string_view SyntheticHillApoptosisModel::kind() const noexcept {
    return synthetic_hill_apoptosis_kind;
}

ApoptosisResponse
SyntheticHillApoptosisModel::evaluate(const ApoptosisResponseRequest& request) const {
    const auto& delivery = request.delivery;
    const auto intracellular = core::in_moles(delivery.intracellular_drug_amount);
    if (request.request_id.empty() || delivery.request_id.empty() || delivery.model_id.empty() ||
        delivery.drug_id != config_.drug_id ||
        delivery.observation_after_activation < core::SimulationClock::Duration::zero() ||
        !std::isfinite(intracellular) || intracellular < 0.0 ||
        (delivery.activated != delivery.activation_offset.has_value()) ||
        (!delivery.activated && intracellular != 0.0)) {
        invalid("Apoptosis-response request has incompatible delivery input");
    }

    auto observed_at = delivery.observation_after_activation;
    if (delivery.activation_offset.has_value()) {
        const auto activation = delivery.activation_offset.value();
        if (activation < core::SimulationClock::Duration::zero() ||
            activation > core::SimulationClock::Duration::max() - observed_at) {
            invalid("Apoptosis-response observation time is invalid or overflows");
        }
        observed_at += activation;
    }

    const auto effect =
        synthetic_hill_effect(delivery.intracellular_drug_amount,
                              {config_.half_max_effect_amount, config_.hill_coefficient});
    const auto state = effect >= config_.apoptosis_commitment_threshold
                           ? CellState::apoptosis_committed
                           : CellState::viable;
    ApoptosisResponse response{request.request_id,
                               config_.model_id,
                               config_.cell_id,
                               config_.drug_id,
                               delivery.request_id,
                               delivery.model_id,
                               delivery.activated,
                               observed_at,
                               delivery.intracellular_drug_amount,
                               effect,
                               state};
    validate_apoptosis_response(response);
    return response;
}

} // namespace mehlissa::models::cell

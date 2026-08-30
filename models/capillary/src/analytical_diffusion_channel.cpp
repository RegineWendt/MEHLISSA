// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/analytical_diffusion_channel.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

namespace mehlissa::models::capillary {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double equivalent_spherical_radius(const core::Volume volume) noexcept {
    return std::cbrt(3.0 * core::in_cubic_meters(volume) / (4.0 * std::numbers::pi));
}

} // namespace

AnalyticalDiffusionChannel::AnalyticalDiffusionChannel(AnalyticalDiffusionChannelConfig config)
    : config_{std::move(config)} {
    if (config_.model_id.empty() ||
        !std::isfinite(core::in_square_meters_per_second(config_.diffusion_coefficient)) ||
        core::in_square_meters_per_second(config_.diffusion_coefficient) <= 0.0 ||
        !std::isfinite(core::in_per_second(config_.first_order_degradation_rate)) ||
        core::in_per_second(config_.first_order_degradation_rate) < 0.0 ||
        !std::isfinite(config_.maximum_receiver_radius_to_separation_ratio) ||
        config_.maximum_receiver_radius_to_separation_ratio <= 0.0 ||
        config_.maximum_receiver_radius_to_separation_ratio > 0.25) {
        invalid("Analytical diffusion-channel configuration is invalid");
    }
}

std::string_view AnalyticalDiffusionChannel::kind() const noexcept {
    return analytical_free_diffusion_3d_kind;
}

std::string_view AnalyticalDiffusionChannel::model_id() const noexcept { return config_.model_id; }

MolecularChannelResponse
AnalyticalDiffusionChannel::evaluate(const MolecularChannelRequest& request) const {
    const auto emitted_amount = core::in_moles(request.emitted_amount);
    const auto separation = core::in_meters(request.transmitter_receiver_separation);
    const auto receiver_volume = core::in_cubic_meters(request.passive_receiver_volume);
    const auto observation_seconds =
        std::chrono::duration<double>{request.observation_time}.count();
    if (request.request_id.empty() || request.signal_id.empty() ||
        request.context_model_id.empty() || request.region_id.empty() ||
        !std::isfinite(emitted_amount) || emitted_amount <= 0.0 || !std::isfinite(separation) ||
        separation <= 0.0 || !std::isfinite(receiver_volume) || receiver_volume <= 0.0 ||
        !std::isfinite(observation_seconds) || observation_seconds <= 0.0) {
        invalid("Molecular-channel request is incomplete or non-positive");
    }

    const auto receiver_ratio =
        equivalent_spherical_radius(request.passive_receiver_volume) / separation;
    if (receiver_ratio > config_.maximum_receiver_radius_to_separation_ratio) {
        invalid("Passive receiver is too large for the channel's uniform-concentration "
                "approximation");
    }

    const auto diffusion = core::in_square_meters_per_second(config_.diffusion_coefficient);
    const auto degradation = core::in_per_second(config_.first_order_degradation_rate);
    const auto spreading = 4.0 * std::numbers::pi * diffusion * observation_seconds;
    const auto exponent = -(separation * separation) / (4.0 * diffusion * observation_seconds) -
                          degradation * observation_seconds;
    const auto concentration = emitted_amount * std::exp(exponent) / std::pow(spreading, 1.5);
    const auto receiver_amount = concentration * receiver_volume;
    const auto receiver_fraction = receiver_amount / emitted_amount;
    if (!std::isfinite(concentration) || !std::isfinite(receiver_amount) ||
        !std::isfinite(receiver_fraction) || receiver_fraction < 0.0 || receiver_fraction > 1.0) {
        invalid("Analytical diffusion-channel result violates finite passive-receiver bounds");
    }

    return {
        request.request_id,
        request.signal_id,
        config_.model_id,
        request.observation_time,
        core::moles_per_cubic_meter(concentration),
        core::moles(receiver_amount),
        receiver_fraction,
    };
}

core::SimulationClock::Duration
AnalyticalDiffusionChannel::peak_observation_time(const core::Length separation) const {
    const auto distance = core::in_meters(separation);
    if (!std::isfinite(distance) || distance <= 0.0) {
        invalid("Peak observation time requires a positive finite separation");
    }
    const auto diffusion = core::in_square_meters_per_second(config_.diffusion_coefficient);
    const auto degradation = core::in_per_second(config_.first_order_degradation_rate);
    const auto a = distance * distance / (4.0 * diffusion);
    const auto peak_seconds = degradation == 0.0
                                  ? 2.0 * a / 3.0
                                  : 2.0 * a / (1.5 + std::sqrt(2.25 + 4.0 * degradation * a));
    const auto duration = std::chrono::duration<double>{peak_seconds};
    const auto ticks = std::chrono::duration_cast<core::SimulationClock::Duration>(duration);
    if (ticks <= core::SimulationClock::Duration::zero()) {
        invalid("Peak observation time is below simulation-clock resolution");
    }
    return ticks;
}

} // namespace mehlissa::models::capillary

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/brownian_particle_channel.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/random_stream.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <string>
#include <utility>

namespace mehlissa::models::capillary {
namespace {

constexpr std::uint64_t minimum_sample_count = 10'000;
constexpr std::uint64_t maximum_sample_count = 100'000'000;
constexpr double two_to_minus_53 = 1.0 / 9'007'199'254'740'992.0;

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double open_unit_interval(core::RandomStream& random) {
    const auto grid_value = random.next_u64() >> 11U;
    return (static_cast<double>(grid_value) + 0.5) * two_to_minus_53;
}

[[nodiscard]] std::array<double, 2> standard_normal_pair(core::RandomStream& random) {
    const auto radius = std::sqrt(-2.0 * std::log(open_unit_interval(random)));
    const auto angle = 2.0 * std::numbers::pi * open_unit_interval(random);
    return {radius * std::cos(angle), radius * std::sin(angle)};
}

[[nodiscard]] double equivalent_spherical_radius(const core::Volume volume) noexcept {
    return std::cbrt(3.0 * core::in_cubic_meters(volume) / (4.0 * std::numbers::pi));
}

} // namespace

BrownianParticleChannel::BrownianParticleChannel(BrownianParticleChannelConfig config)
    : config_{std::move(config)} {
    if (config_.model_id.empty() || config_.random_stream_name.empty() ||
        !std::isfinite(core::in_square_meters_per_second(config_.diffusion_coefficient)) ||
        core::in_square_meters_per_second(config_.diffusion_coefficient) <= 0.0 ||
        !std::isfinite(core::in_per_second(config_.first_order_degradation_rate)) ||
        core::in_per_second(config_.first_order_degradation_rate) < 0.0 ||
        config_.sample_count < minimum_sample_count ||
        config_.sample_count > maximum_sample_count) {
        invalid("Brownian particle-channel configuration is invalid");
    }
}

std::string_view BrownianParticleChannel::kind() const noexcept {
    return brownian_particle_endpoint_3d_kind;
}

std::string_view BrownianParticleChannel::model_id() const noexcept { return config_.model_id; }

MolecularChannelResponse
BrownianParticleChannel::evaluate(const MolecularChannelRequest& request) const {
    return evaluate_with_diagnostics(request).response;
}

BrownianParticleChannelEvaluation
BrownianParticleChannel::evaluate_with_diagnostics(const MolecularChannelRequest& request) const {
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
        invalid("Brownian particle-channel request is incomplete or non-positive");
    }

    const auto diffusion = core::in_square_meters_per_second(config_.diffusion_coefficient);
    const auto degradation = core::in_per_second(config_.first_order_degradation_rate);
    const auto displacement_standard_deviation = std::sqrt(2.0 * diffusion * observation_seconds);
    const auto survival_probability = std::exp(-degradation * observation_seconds);
    const auto receiver_radius = equivalent_spherical_radius(request.passive_receiver_volume);
    const auto receiver_radius_squared = receiver_radius * receiver_radius;
    core::RandomStream random{config_.experiment_seed,
                              config_.random_stream_name + "." + request.request_id};

    std::uint64_t observation_count{};
    for (std::uint64_t sample = 0; sample < config_.sample_count; ++sample) {
        const auto first_pair = standard_normal_pair(random);
        const auto second_pair = standard_normal_pair(random);
        const auto x_offset = displacement_standard_deviation * first_pair[0] - separation;
        const auto y_offset = displacement_standard_deviation * first_pair[1];
        const auto z_offset = displacement_standard_deviation * second_pair[0];
        const auto inside_receiver =
            x_offset * x_offset + y_offset * y_offset + z_offset * z_offset <=
            receiver_radius_squared;
        const auto survives =
            degradation == 0.0 || open_unit_interval(random) <= survival_probability;
        if (inside_receiver && survives) {
            ++observation_count;
        }
    }

    const auto sample_count = static_cast<double>(config_.sample_count);
    const auto receiver_fraction = static_cast<double>(observation_count) / sample_count;
    const auto standard_error =
        std::sqrt(receiver_fraction * (1.0 - receiver_fraction) / sample_count);
    const auto receiver_amount = emitted_amount * receiver_fraction;
    const auto concentration = receiver_amount / receiver_volume;
    if (!std::isfinite(receiver_fraction) || !std::isfinite(standard_error) ||
        !std::isfinite(receiver_amount) || !std::isfinite(concentration)) {
        invalid("Brownian particle-channel result is non-finite");
    }

    return {
        {request.request_id, request.signal_id, config_.model_id, request.observation_time,
         core::moles_per_cubic_meter(concentration), core::moles(receiver_amount),
         receiver_fraction},
        config_.sample_count,
        observation_count,
        standard_error,
    };
}

} // namespace mehlissa::models::capillary

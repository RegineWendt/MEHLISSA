// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/radial_finite_volume_channel.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace mehlissa::models::capillary {
namespace {

constexpr std::uint64_t minimum_cell_count = 16;
constexpr std::uint64_t maximum_cell_count = 4'096;
constexpr std::uint64_t maximum_time_step_count = 10'000'000;
constexpr std::uint64_t maximum_cell_steps = 100'000'000;
constexpr double negative_roundoff_tolerance = 1.0e-14;

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double sphere_volume(const double radius) noexcept {
    return 4.0 * std::numbers::pi * radius * radius * radius / 3.0;
}

[[nodiscard]] double sphere_area(const double radius) noexcept {
    return 4.0 * std::numbers::pi * radius * radius;
}

[[nodiscard]] double equivalent_spherical_radius(const core::Volume volume) noexcept {
    return std::cbrt(3.0 * core::in_cubic_meters(volume) / (4.0 * std::numbers::pi));
}

void validate_config(const RadialFiniteVolumeChannelConfig& config) {
    const auto diffusion = core::in_square_meters_per_second(config.diffusion_coefficient);
    const auto degradation = core::in_per_second(config.first_order_degradation_rate);
    const auto radius = core::in_meters(config.radial_domain_radius);
    if (config.model_id.empty() || !std::isfinite(diffusion) || diffusion <= 0.0 ||
        !std::isfinite(degradation) || degradation < 0.0 ||
        config.radial_cell_count < minimum_cell_count ||
        config.radial_cell_count > maximum_cell_count || !std::isfinite(radius) || radius <= 0.0 ||
        !std::isfinite(config.cfl_safety_factor) || config.cfl_safety_factor <= 0.0 ||
        config.cfl_safety_factor > 1.0) {
        invalid("Radial finite-volume channel configuration is invalid");
    }
}

} // namespace

RadialFiniteVolumeChannel::RadialFiniteVolumeChannel(RadialFiniteVolumeChannelConfig config)
    : config_{std::move(config)} {
    validate_config(config_);
}

std::string_view RadialFiniteVolumeChannel::kind() const noexcept {
    return radial_finite_volume_diffusion_3d_kind;
}

std::string_view RadialFiniteVolumeChannel::model_id() const noexcept { return config_.model_id; }

MolecularChannelResponse
RadialFiniteVolumeChannel::evaluate(const MolecularChannelRequest& request) const {
    return evaluate_with_diagnostics(request).response;
}

RadialFiniteVolumeChannelEvaluation
RadialFiniteVolumeChannel::evaluate_with_diagnostics(const MolecularChannelRequest& request) const {
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
        invalid("Radial finite-volume channel request is incomplete or non-positive");
    }

    const auto domain_radius = core::in_meters(config_.radial_domain_radius);
    const auto receiver_radius = equivalent_spherical_radius(request.passive_receiver_volume);
    if (separation + receiver_radius >= domain_radius) {
        invalid("Passive receiver must lie completely inside the radial finite-volume domain");
    }

    const auto cell_count = config_.radial_cell_count;
    const auto cell_width = domain_radius / static_cast<double>(cell_count);
    const auto diffusion = core::in_square_meters_per_second(config_.diffusion_coefficient);
    const auto degradation = core::in_per_second(config_.first_order_degradation_rate);
    const auto maximum_step_seconds =
        config_.cfl_safety_factor * cell_width * cell_width / (6.0 * diffusion);
    const auto raw_step_count = std::ceil(observation_seconds / maximum_step_seconds);
    if (!std::isfinite(raw_step_count) || raw_step_count < 1.0 ||
        raw_step_count > static_cast<double>(maximum_time_step_count)) {
        invalid("Radial finite-volume time-step count is invalid or unbounded");
    }
    const auto time_step_count = static_cast<std::uint64_t>(raw_step_count);
    if (cell_count > maximum_cell_steps / time_step_count) {
        invalid("Radial finite-volume cell-step product exceeds the work bound");
    }
    const auto time_step_seconds = observation_seconds / static_cast<double>(time_step_count);
    const auto degradation_factor = std::exp(-degradation * time_step_seconds);

    std::vector<double> cell_volumes(cell_count);
    for (std::uint64_t cell = 0; cell < cell_count; ++cell) {
        const auto inner_radius = static_cast<double>(cell) * cell_width;
        const auto outer_radius = static_cast<double>(cell + 1) * cell_width;
        cell_volumes[cell] = sphere_volume(outer_radius) - sphere_volume(inner_radius);
    }

    std::vector<double> active(cell_count);
    std::vector<double> next(cell_count);
    active.front() = 1.0;
    double degraded_fraction{};
    double escaped_fraction{};
    for (std::uint64_t step = 0; step < time_step_count; ++step) {
        next = active;
        for (std::uint64_t interface = 1; interface < cell_count; ++interface) {
            const auto left = interface - 1;
            const auto right = interface;
            const auto interface_radius = static_cast<double>(interface) * cell_width;
            const auto left_concentration = active[left] / cell_volumes[left];
            const auto right_concentration = active[right] / cell_volumes[right];
            const auto outward_flux = diffusion * sphere_area(interface_radius) *
                                      (left_concentration - right_concentration) / cell_width *
                                      time_step_seconds;
            next[left] -= outward_flux;
            next[right] += outward_flux;
        }

        const auto outer_concentration = active.back() / cell_volumes.back();
        const auto outward_escape = diffusion * sphere_area(domain_radius) * outer_concentration /
                                    (0.5 * cell_width) * time_step_seconds;
        next.back() -= outward_escape;
        escaped_fraction += outward_escape;

        const auto active_before_degradation = std::accumulate(next.begin(), next.end(), 0.0);
        degraded_fraction += active_before_degradation * (1.0 - degradation_factor);
        for (auto& amount_fraction : next) {
            amount_fraction *= degradation_factor;
            if (!std::isfinite(amount_fraction) || amount_fraction < -negative_roundoff_tolerance) {
                invalid("Radial finite-volume update violated positivity or finiteness");
            }
            amount_fraction = std::max(0.0, amount_fraction);
        }
        active.swap(next);
    }

    std::vector<RadialConcentrationCell> final_field;
    final_field.reserve(cell_count);
    std::vector<double> concentrations(cell_count);
    for (std::uint64_t cell = 0; cell < cell_count; ++cell) {
        const auto inner_radius = static_cast<double>(cell) * cell_width;
        const auto outer_radius = static_cast<double>(cell + 1) * cell_width;
        concentrations[cell] = active[cell] / cell_volumes[cell];
        final_field.push_back({cell, core::meters(inner_radius), core::meters(outer_radius),
                               active[cell], concentrations[cell]});
    }

    const auto receiver_coordinate = separation / cell_width - 0.5;
    std::uint64_t receiver_lower_cell{};
    std::uint64_t receiver_upper_cell{};
    double interpolation_weight{};
    if (receiver_coordinate <= 0.0) {
        receiver_lower_cell = 0;
        receiver_upper_cell = 0;
    } else {
        receiver_lower_cell = static_cast<std::uint64_t>(std::floor(receiver_coordinate));
        receiver_lower_cell = std::min(receiver_lower_cell, cell_count - 1);
        receiver_upper_cell = std::min(receiver_lower_cell + 1, cell_count - 1);
        interpolation_weight = receiver_upper_cell == receiver_lower_cell
                                   ? 0.0
                                   : receiver_coordinate - static_cast<double>(receiver_lower_cell);
    }
    const auto normalized_receiver_concentration =
        concentrations[receiver_lower_cell] * (1.0 - interpolation_weight) +
        concentrations[receiver_upper_cell] * interpolation_weight;
    const auto receiver_fraction = normalized_receiver_concentration * receiver_volume;
    const auto receiver_amount = emitted_amount * receiver_fraction;
    const auto receiver_concentration = emitted_amount * normalized_receiver_concentration;
    const auto active_fraction = std::accumulate(active.begin(), active.end(), 0.0);
    const auto conservation_residual = active_fraction + degraded_fraction + escaped_fraction - 1.0;
    if (!std::isfinite(receiver_fraction) || receiver_fraction < 0.0 ||
        receiver_fraction > active_fraction || !std::isfinite(receiver_amount) ||
        !std::isfinite(receiver_concentration) || !std::isfinite(active_fraction) ||
        !std::isfinite(degraded_fraction) || !std::isfinite(escaped_fraction) ||
        escaped_fraction < 0.0 || !std::isfinite(conservation_residual)) {
        invalid("Radial finite-volume result is invalid or non-finite");
    }

    return {
        {request.request_id, request.signal_id, config_.model_id, request.observation_time,
         core::moles_per_cubic_meter(receiver_concentration), core::moles(receiver_amount),
         receiver_fraction},
        cell_count,
        time_step_count,
        core::meters(cell_width),
        time_step_seconds,
        receiver_lower_cell,
        receiver_upper_cell,
        interpolation_weight,
        active_fraction,
        degraded_fraction,
        escaped_fraction,
        conservation_residual,
        std::move(final_field),
    };
}

} // namespace mehlissa::models::capillary

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/trajectory_brownian_channel.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/random_stream.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <string>
#include <utility>

namespace mehlissa::models::capillary {
namespace {

constexpr std::uint64_t minimum_sample_count = 10'000;
constexpr std::uint64_t maximum_sample_count = 10'000'000;
constexpr std::uint64_t maximum_step_count = 4'096;
constexpr std::uint64_t maximum_particle_steps = 100'000'000;
constexpr std::size_t maximum_trajectory_points = 1'000'000;
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

void reflect_coordinate(double& coordinate, const double half_extent,
                        std::uint64_t& reflection_count) {
    while (coordinate > half_extent || coordinate < -half_extent) {
        if (coordinate > half_extent) {
            coordinate = 2.0 * half_extent - coordinate;
        } else {
            coordinate = -2.0 * half_extent - coordinate;
        }
        if (reflection_count == std::numeric_limits<std::uint64_t>::max()) {
            invalid("Brownian trajectory reflection counter overflow");
        }
        ++reflection_count;
    }
}

void validate_config(const TrajectoryBrownianChannelConfig& config) {
    const auto diffusion = core::in_square_meters_per_second(config.diffusion_coefficient);
    const auto degradation = core::in_per_second(config.first_order_degradation_rate);
    if (config.model_id.empty() || config.random_stream_name.empty() || !std::isfinite(diffusion) ||
        diffusion <= 0.0 || !std::isfinite(degradation) || degradation < 0.0 ||
        config.sample_count < minimum_sample_count || config.sample_count > maximum_sample_count ||
        config.step_count == 0 || config.step_count > maximum_step_count ||
        config.sample_count > maximum_particle_steps / config.step_count ||
        config.retained_trajectory_count > config.sample_count ||
        config.maximum_retained_points > maximum_trajectory_points) {
        invalid("Brownian trajectory-channel configuration is invalid or unbounded");
    }
    if (config.retained_trajectory_count > 0 && config.maximum_retained_points == 0) {
        invalid("Retained Brownian trajectories require a positive point bound");
    }
    if (config.boundary.kind == BrownianBoundaryKind::reflecting_box) {
        for (const auto half_extent : config.boundary.reflecting_box_half_extents) {
            if (!std::isfinite(core::in_meters(half_extent)) ||
                core::in_meters(half_extent) <= 0.0) {
                invalid("Reflecting Brownian box half-extents must be positive and finite");
            }
        }
    }
}

} // namespace

TrajectoryBrownianChannel::TrajectoryBrownianChannel(TrajectoryBrownianChannelConfig config)
    : config_{std::move(config)} {
    validate_config(config_);
}

std::string_view TrajectoryBrownianChannel::kind() const noexcept {
    return brownian_trajectory_3d_kind;
}

std::string_view TrajectoryBrownianChannel::model_id() const noexcept { return config_.model_id; }

BrownianBoundaryKind TrajectoryBrownianChannel::boundary_kind() const noexcept {
    return config_.boundary.kind;
}

MolecularChannelResponse
TrajectoryBrownianChannel::evaluate(const MolecularChannelRequest& request) const {
    return evaluate_with_diagnostics(request).response;
}

TrajectoryBrownianChannelEvaluation
TrajectoryBrownianChannel::evaluate_with_diagnostics(const MolecularChannelRequest& request) const {
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
        invalid("Brownian trajectory-channel request is incomplete or non-positive");
    }

    const auto receiver_radius = equivalent_spherical_radius(request.passive_receiver_volume);
    if (config_.boundary.kind == BrownianBoundaryKind::reflecting_box) {
        const auto x_half_extent = core::in_meters(config_.boundary.reflecting_box_half_extents[0]);
        const auto y_half_extent = core::in_meters(config_.boundary.reflecting_box_half_extents[1]);
        const auto z_half_extent = core::in_meters(config_.boundary.reflecting_box_half_extents[2]);
        if (separation + receiver_radius > x_half_extent || receiver_radius > y_half_extent ||
            receiver_radius > z_half_extent) {
            invalid("Passive receiver must lie completely inside the reflecting Brownian box");
        }
    }

    const auto diffusion = core::in_square_meters_per_second(config_.diffusion_coefficient);
    const auto degradation = core::in_per_second(config_.first_order_degradation_rate);
    const auto step_seconds = observation_seconds / static_cast<double>(config_.step_count);
    const auto step_standard_deviation = std::sqrt(2.0 * diffusion * step_seconds);
    const auto step_survival_probability = std::exp(-degradation * step_seconds);
    const auto receiver_radius_squared = receiver_radius * receiver_radius;
    core::RandomStream random{config_.experiment_seed, config_.random_stream_name + ".steps-" +
                                                           std::to_string(config_.step_count) +
                                                           "." + request.request_id};

    std::vector<BrownianTrajectoryPoint> retained_points;
    retained_points.reserve(config_.maximum_retained_points);
    std::uint64_t dropped_points{};
    auto retain = [&](const std::uint64_t particle, const std::uint64_t step,
                      const std::array<double, 3>& position, const bool survives) {
        if (particle >= config_.retained_trajectory_count) {
            return;
        }
        if (retained_points.size() < config_.maximum_retained_points) {
            retained_points.push_back(
                {particle,
                 step,
                 step_seconds * static_cast<double>(step),
                 {core::meters(position[0]), core::meters(position[1]), core::meters(position[2])},
                 survives});
        } else {
            ++dropped_points;
        }
    };

    std::uint64_t receiver_observation_count{};
    std::uint64_t surviving_particle_count{};
    std::uint64_t boundary_reflection_count{};
    double squared_displacement_sum{};
    for (std::uint64_t particle = 0; particle < config_.sample_count; ++particle) {
        std::array<double, 3> position{};
        bool survives = true;
        retain(particle, 0, position, survives);
        for (std::uint64_t step = 1; step <= config_.step_count; ++step) {
            const auto first_pair = standard_normal_pair(random);
            const auto second_pair = standard_normal_pair(random);
            position[0] += step_standard_deviation * first_pair[0];
            position[1] += step_standard_deviation * first_pair[1];
            position[2] += step_standard_deviation * second_pair[0];
            if (config_.boundary.kind == BrownianBoundaryKind::reflecting_box) {
                for (std::size_t dimension = 0; dimension < position.size(); ++dimension) {
                    reflect_coordinate(
                        position[dimension],
                        core::in_meters(config_.boundary.reflecting_box_half_extents[dimension]),
                        boundary_reflection_count);
                }
            }
            if (degradation > 0.0) {
                const auto survives_step = open_unit_interval(random) <= step_survival_probability;
                survives = survives && survives_step;
            }
            retain(particle, step, position, survives);
        }

        squared_displacement_sum +=
            position[0] * position[0] + position[1] * position[1] + position[2] * position[2];
        if (survives) {
            ++surviving_particle_count;
            const auto x_offset = position[0] - separation;
            if (x_offset * x_offset + position[1] * position[1] + position[2] * position[2] <=
                receiver_radius_squared) {
                ++receiver_observation_count;
            }
        }
    }

    const auto sample_count = static_cast<double>(config_.sample_count);
    const auto receiver_fraction = static_cast<double>(receiver_observation_count) / sample_count;
    const auto standard_error =
        std::sqrt(receiver_fraction * (1.0 - receiver_fraction) / sample_count);
    const auto receiver_amount = emitted_amount * receiver_fraction;
    const auto concentration = receiver_amount / receiver_volume;
    const auto mean_squared_displacement = squared_displacement_sum / sample_count;
    const auto expected_mean_squared_displacement = 6.0 * diffusion * observation_seconds;
    if (!std::isfinite(receiver_fraction) || !std::isfinite(standard_error) ||
        !std::isfinite(receiver_amount) || !std::isfinite(concentration) ||
        !std::isfinite(mean_squared_displacement) || mean_squared_displacement < 0.0) {
        invalid("Brownian trajectory-channel result is invalid or non-finite");
    }

    return {
        {request.request_id, request.signal_id, config_.model_id, request.observation_time,
         core::moles_per_cubic_meter(concentration), core::moles(receiver_amount),
         receiver_fraction},
        config_.sample_count,
        config_.step_count,
        receiver_observation_count,
        surviving_particle_count,
        boundary_reflection_count,
        standard_error,
        mean_squared_displacement,
        expected_mean_squared_displacement,
        std::move(retained_points),
        dropped_points,
    };
}

} // namespace mehlissa::models::capillary

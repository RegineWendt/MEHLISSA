// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/axial_advection_reaction_case.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/random_stream.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace mehlissa::models::capillary {
namespace {

constexpr std::uint64_t minimum_particle_count = 10'000;
constexpr std::uint64_t maximum_particle_count = 10'000'000;
constexpr std::uint64_t maximum_field_cell_count = 4'096;
constexpr std::uint64_t maximum_field_time_steps = 2'000'000;
constexpr std::uint64_t maximum_field_cell_steps = 200'000'000;
constexpr double two_to_minus_53 = 1.0 / 9'007'199'254'740'992.0;

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double open_unit_interval(core::RandomStream& random) {
    const auto grid_value = random.next_u64() >> 11U;
    return (static_cast<double>(grid_value) + 0.5) * two_to_minus_53;
}

[[nodiscard]] double standard_normal(core::RandomStream& random) {
    const auto radius = std::sqrt(-2.0 * std::log(open_unit_interval(random)));
    const auto angle = 2.0 * std::numbers::pi * open_unit_interval(random);
    return radius * std::cos(angle);
}

struct ScalarConfig final {
    double length{};
    double source{};
    double receiver_center{};
    double receiver_width{};
    double radius{};
    double velocity{};
    double diffusion{};
    double bulk_rate{};
    double wall_velocity{};
    double wall_rate{};
    double total_rate{};
    double observation_seconds{};
};

[[nodiscard]] ScalarConfig validate_config(const AxialAdvectionReactionConfig& config) {
    ScalarConfig values{
        core::in_meters(config.domain_length),
        core::in_meters(config.source_position),
        core::in_meters(config.receiver_center),
        core::in_meters(config.receiver_width),
        core::in_meters(config.lumen_radius),
        core::in_meters_per_second(config.mean_advection_speed),
        core::in_square_meters_per_second(config.diffusion_coefficient),
        core::in_per_second(config.bulk_reaction_rate),
        core::in_meters_per_second(config.wall_interaction_velocity),
        0.0,
        0.0,
        core::in_seconds(config.observation_time),
    };
    values.wall_rate = 2.0 * values.wall_velocity / values.radius;
    values.total_rate = values.bulk_rate + values.wall_rate;
    const auto receiver_left = values.receiver_center - values.receiver_width / 2.0;
    const auto receiver_right = values.receiver_center + values.receiver_width / 2.0;
    if (config.model_id.empty() || !std::isfinite(values.length) || values.length <= 0.0 ||
        !std::isfinite(values.source) || values.source <= 0.0 || values.source >= values.length ||
        !std::isfinite(values.receiver_center) || !std::isfinite(values.receiver_width) ||
        values.receiver_width <= 0.0 || receiver_left <= 0.0 || receiver_right >= values.length ||
        !std::isfinite(values.radius) || values.radius <= 0.0 || !std::isfinite(values.velocity) ||
        values.velocity <= 0.0 || !std::isfinite(values.diffusion) || values.diffusion <= 0.0 ||
        !std::isfinite(values.bulk_rate) || values.bulk_rate <= 0.0 ||
        !std::isfinite(values.wall_velocity) || values.wall_velocity <= 0.0 ||
        !std::isfinite(values.wall_rate) || values.wall_rate <= 0.0 ||
        !std::isfinite(values.observation_seconds) || values.observation_seconds <= 0.0 ||
        values.source + values.velocity * values.observation_seconds >= values.length) {
        invalid("Axial advection-reaction configuration is incomplete or outside its domain");
    }
    return values;
}

struct NormalDistribution final {
    double mean{};
    double standard_deviation{};
};

[[nodiscard]] double normal_interval_probability(const double left, const double right,
                                                 const NormalDistribution distribution) noexcept {
    const auto scale = std::numbers::sqrt2 * distribution.standard_deviation;
    return 0.5 * (std::erf((right - distribution.mean) / scale) -
                  std::erf((left - distribution.mean) / scale));
}

[[nodiscard]] double relative_error(const double value, const double reference) noexcept {
    if (reference == 0.0) {
        return std::abs(value);
    }
    return std::abs(value - reference) / std::abs(reference);
}

void validate_particle_config(const AxialParticleConfig& config) {
    if (config.sample_count < minimum_particle_count ||
        config.sample_count > maximum_particle_count || config.random_stream_name.empty()) {
        invalid("Axial particle configuration is invalid or unbounded");
    }
}

void validate_field_config(const AxialFieldConfig& config) {
    if (config.cell_count < 16 || config.cell_count > maximum_field_cell_count ||
        !std::isfinite(config.cfl_safety_factor) || config.cfl_safety_factor <= 0.0 ||
        config.cfl_safety_factor > 0.5) {
        invalid("Axial finite-volume configuration is invalid or unbounded");
    }
}

void validate_gate(const AxialAdvectionReactionVerificationGate& gate) {
    const std::array bounded_positive{
        gate.maximum_particle_standardized_error,      gate.maximum_coarse_relative_reference_error,
        gate.maximum_refined_relative_reference_error, gate.maximum_relative_refinement_difference,
        gate.maximum_relative_active_fraction_error,   gate.maximum_absolute_conservation_residual,
    };
    if (gate.minimum_particle_receiver_count == 0 ||
        !std::ranges::all_of(
            bounded_positive,
            [](const double value) { return std::isfinite(value) && value > 0.0; }) ||
        gate.maximum_refined_relative_reference_error >
            gate.maximum_coarse_relative_reference_error ||
        gate.maximum_coarse_relative_reference_error > 1.0 ||
        gate.maximum_relative_refinement_difference > 1.0 ||
        gate.maximum_relative_active_fraction_error > 1.0 ||
        !std::isfinite(gate.maximum_escaped_amount_fraction) ||
        gate.maximum_escaped_amount_fraction < 0.0 || gate.maximum_escaped_amount_fraction > 1.0) {
        invalid("Axial advection-reaction verification gate is invalid");
    }
}

} // namespace

AxialAnalyticalEvaluation
evaluate_axial_advection_reaction_reference(const AxialAdvectionReactionConfig& config) {
    const auto values = validate_config(config);
    const auto active = std::exp(-values.total_rate * values.observation_seconds);
    const auto reacted = 1.0 - active;
    const auto bulk_reacted = reacted * values.bulk_rate / values.total_rate;
    const auto wall_reacted = reacted * values.wall_rate / values.total_rate;
    const auto mean = values.source + values.velocity * values.observation_seconds;
    const auto standard_deviation = std::sqrt(2.0 * values.diffusion * values.observation_seconds);
    const auto receiver_left = values.receiver_center - values.receiver_width / 2.0;
    const auto receiver_right = values.receiver_center + values.receiver_width / 2.0;
    const auto receiver = active * normal_interval_probability(receiver_left, receiver_right,
                                                               {mean, standard_deviation});
    return {receiver,
            active,
            bulk_reacted,
            wall_reacted,
            values.wall_rate,
            core::meters(mean),
            core::meters(standard_deviation),
            active + bulk_reacted + wall_reacted - 1.0};
}

AxialParticleEvaluation
evaluate_axial_advection_reaction_particles(const AxialAdvectionReactionConfig& config,
                                            const AxialParticleConfig& particle_config) {
    const auto values = validate_config(config);
    validate_particle_config(particle_config);
    const auto survival = std::exp(-values.total_rate * values.observation_seconds);
    const auto bulk_probability = values.bulk_rate / values.total_rate;
    const auto mean = values.source + values.velocity * values.observation_seconds;
    const auto standard_deviation = std::sqrt(2.0 * values.diffusion * values.observation_seconds);
    const auto receiver_left = values.receiver_center - values.receiver_width / 2.0;
    const auto receiver_right = values.receiver_center + values.receiver_width / 2.0;
    core::RandomStream random{particle_config.experiment_seed,
                              particle_config.random_stream_name + "." + config.model_id};

    std::uint64_t receiver_count{};
    std::uint64_t active_in_domain_count{};
    std::uint64_t escaped_count{};
    std::uint64_t bulk_reacted_count{};
    std::uint64_t wall_reacted_count{};
    double active_position_sum{};
    std::uint64_t active_total_count{};
    for (std::uint64_t particle = 0; particle < particle_config.sample_count; ++particle) {
        const auto position = mean + standard_deviation * standard_normal(random);
        const auto survives = open_unit_interval(random) <= survival;
        const auto reaction_kind = open_unit_interval(random);
        if (!survives) {
            if (reaction_kind <= bulk_probability) {
                ++bulk_reacted_count;
            } else {
                ++wall_reacted_count;
            }
            continue;
        }
        ++active_total_count;
        active_position_sum += position;
        if (position < 0.0 || position >= values.length) {
            ++escaped_count;
            continue;
        }
        ++active_in_domain_count;
        if (position >= receiver_left && position <= receiver_right) {
            ++receiver_count;
        }
    }
    if (active_total_count == 0) {
        invalid("Axial particle evaluation produced no active samples");
    }

    const auto count = static_cast<double>(particle_config.sample_count);
    const auto receiver_fraction = static_cast<double>(receiver_count) / count;
    const auto active_fraction = static_cast<double>(active_in_domain_count) / count;
    const auto escaped_fraction = static_cast<double>(escaped_count) / count;
    const auto bulk_fraction = static_cast<double>(bulk_reacted_count) / count;
    const auto wall_fraction = static_cast<double>(wall_reacted_count) / count;
    const auto standard_error = std::sqrt(receiver_fraction * (1.0 - receiver_fraction) / count);
    const auto residual = active_fraction + escaped_fraction + bulk_fraction + wall_fraction - 1.0;
    return {particle_config.sample_count,
            receiver_count,
            active_in_domain_count,
            escaped_count,
            bulk_reacted_count,
            wall_reacted_count,
            receiver_fraction,
            active_fraction,
            escaped_fraction,
            bulk_fraction,
            wall_fraction,
            standard_error,
            core::meters(active_position_sum / static_cast<double>(active_total_count)),
            residual};
}

AxialFieldEvaluation
evaluate_axial_advection_reaction_field(const AxialAdvectionReactionConfig& config,
                                        const AxialFieldConfig& field_config) {
    const auto values = validate_config(config);
    validate_field_config(field_config);
    const auto cell_count = static_cast<std::size_t>(field_config.cell_count);
    const auto dx = values.length / static_cast<double>(field_config.cell_count);
    const auto transport_rate = 4.0 * values.diffusion / (dx * dx) + 2.0 * values.velocity / dx;
    const auto maximum_step = field_config.cfl_safety_factor / transport_rate;
    const auto step_count =
        static_cast<std::uint64_t>(std::ceil(values.observation_seconds / maximum_step));
    if (step_count == 0 || step_count > maximum_field_time_steps ||
        field_config.cell_count > maximum_field_cell_steps / step_count) {
        invalid("Axial finite-volume evaluation exceeds its bounded work budget");
    }
    const auto dt = values.observation_seconds / static_cast<double>(step_count);
    const auto reaction_survival = std::exp(-values.total_rate * dt);
    const auto source_cell =
        std::min(static_cast<std::size_t>(values.source / dx), cell_count - std::size_t{1});
    std::vector<double> amounts(cell_count);
    std::vector<double> updated(cell_count);
    std::vector<double> fluxes(cell_count + 1);
    amounts[source_cell] = 1.0;
    double escaped_left{};
    double escaped_right{};
    double bulk_reacted{};
    double wall_reacted{};

    for (std::uint64_t step = 0; step < step_count; ++step) {
        const auto first_concentration = amounts.front() / dx;
        fluxes.front() = -2.0 * values.diffusion * first_concentration / dx;
        for (std::size_t face = 1; face < cell_count; ++face) {
            const auto left_concentration = amounts[face - 1] / dx;
            const auto right_concentration = amounts[face] / dx;
            fluxes[face] = values.velocity * 0.5 * (left_concentration + right_concentration) -
                           values.diffusion * (right_concentration - left_concentration) / dx;
        }
        const auto last_concentration = amounts.back() / dx;
        fluxes.back() =
            values.velocity * last_concentration + 2.0 * values.diffusion * last_concentration / dx;
        escaped_left += std::max(0.0, -fluxes.front()) * dt;
        escaped_right += std::max(0.0, fluxes.back()) * dt;

        for (std::size_t cell = 0; cell < cell_count; ++cell) {
            auto transported = amounts[cell] - dt * (fluxes[cell + 1] - fluxes[cell]);
            if (transported < -1.0e-14 || !std::isfinite(transported)) {
                invalid("Axial finite-volume transport produced a negative or non-finite amount");
            }
            transported = std::max(0.0, transported);
            const auto reacted = transported * (1.0 - reaction_survival);
            bulk_reacted += reacted * values.bulk_rate / values.total_rate;
            wall_reacted += reacted * values.wall_rate / values.total_rate;
            updated[cell] = transported * reaction_survival;
        }
        amounts.swap(updated);
    }

    const auto active = std::accumulate(amounts.begin(), amounts.end(), 0.0);
    const auto receiver_left = values.receiver_center - values.receiver_width / 2.0;
    const auto receiver_right = values.receiver_center + values.receiver_width / 2.0;
    double receiver{};
    double first_moment{};
    std::vector<AxialFieldCell> final_field;
    final_field.reserve(cell_count);
    for (std::size_t cell = 0; cell < cell_count; ++cell) {
        const auto left = static_cast<double>(cell) * dx;
        const auto right = left + dx;
        const auto overlap =
            std::max(0.0, std::min(right, receiver_right) - std::max(left, receiver_left));
        receiver += amounts[cell] * overlap / dx;
        first_moment += amounts[cell] * (left + right) / 2.0;
        final_field.push_back({static_cast<std::uint64_t>(cell), core::meters(left),
                               core::meters(right), amounts[cell], amounts[cell] / dx});
    }
    const auto residual = active + escaped_left + escaped_right + bulk_reacted + wall_reacted - 1.0;
    if (!std::isfinite(receiver) || !std::isfinite(residual) || active <= 0.0) {
        invalid("Axial finite-volume result is invalid or non-finite");
    }
    return {field_config.cell_count,
            step_count,
            receiver,
            active,
            escaped_left,
            escaped_right,
            bulk_reacted,
            wall_reacted,
            core::meters(first_moment / active),
            residual,
            std::move(final_field)};
}

AxialAdvectionReactionVerificationResult verify_axial_advection_reaction_case(
    const AxialAdvectionReactionConfig& config, const AxialParticleConfig& particle_config,
    const AxialFieldConfig& coarse_field_config, const AxialFieldConfig& refined_field_config,
    const AxialAdvectionReactionVerificationGate& gate) {
    validate_gate(gate);
    if (coarse_field_config.cell_count >= refined_field_config.cell_count ||
        refined_field_config.cell_count % coarse_field_config.cell_count != 0) {
        invalid("Axial finite-volume verification requires a nested coarse-to-refined grid");
    }
    auto result = AxialAdvectionReactionVerificationResult{
        evaluate_axial_advection_reaction_reference(config),
        evaluate_axial_advection_reaction_particles(config, particle_config),
        evaluate_axial_advection_reaction_field(config, coarse_field_config),
        evaluate_axial_advection_reaction_field(config, refined_field_config),
    };
    const auto reference_fraction = result.reference.receiver_amount_fraction;
    const auto particle_standard_error =
        std::sqrt(reference_fraction * (1.0 - reference_fraction) /
                  static_cast<double>(result.particle.sample_count));
    result.particle_reference_standardized_error =
        (result.particle.receiver_amount_fraction - reference_fraction) / particle_standard_error;
    result.particle_relative_active_fraction_error = relative_error(
        result.particle.active_amount_fraction + result.particle.escaped_amount_fraction,
        result.reference.active_amount_fraction);
    result.coarse_relative_reference_error =
        relative_error(result.coarse_field.receiver_amount_fraction, reference_fraction);
    result.refined_relative_reference_error =
        relative_error(result.refined_field.receiver_amount_fraction, reference_fraction);
    result.relative_refinement_difference =
        relative_error(result.refined_field.receiver_amount_fraction,
                       result.coarse_field.receiver_amount_fraction);
    const auto coarse_escape = result.coarse_field.escaped_left_amount_fraction +
                               result.coarse_field.escaped_right_amount_fraction;
    const auto refined_escape = result.refined_field.escaped_left_amount_fraction +
                                result.refined_field.escaped_right_amount_fraction;
    result.passes =
        result.particle.receiver_observation_count >= gate.minimum_particle_receiver_count &&
        std::abs(result.particle_reference_standardized_error) <=
            gate.maximum_particle_standardized_error &&
        result.particle_relative_active_fraction_error <=
            gate.maximum_relative_active_fraction_error &&
        result.coarse_relative_reference_error <= gate.maximum_coarse_relative_reference_error &&
        result.refined_relative_reference_error <= gate.maximum_refined_relative_reference_error &&
        result.relative_refinement_difference <= gate.maximum_relative_refinement_difference &&
        std::abs(result.reference.conservation_residual) <=
            gate.maximum_absolute_conservation_residual &&
        std::abs(result.particle.conservation_residual) <=
            gate.maximum_absolute_conservation_residual &&
        std::abs(result.coarse_field.conservation_residual) <=
            gate.maximum_absolute_conservation_residual &&
        std::abs(result.refined_field.conservation_residual) <=
            gate.maximum_absolute_conservation_residual &&
        coarse_escape <= gate.maximum_escaped_amount_fraction &&
        refined_escape <= gate.maximum_escaped_amount_fraction;
    return result;
}

} // namespace mehlissa::models::capillary

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/time_varying_receptor_ligand_model.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/analytical_receptor_ligand_model.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

namespace mehlissa::models::cell {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

[[nodiscard]] double seconds(const core::SimulationClock::Duration duration) noexcept {
    return std::chrono::duration<double>{duration}.count();
}

[[nodiscard]] core::SimulationClock::Duration duration_from_seconds(const double value) {
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

void validate_request(const TimeVaryingReceptorLigandModelConfig& config,
                      const TimeVaryingReceptorLigandRequest& request) {
    if (request.request_id.empty() || request.ligand_id != config.binding.ligand_id ||
        request.compartment_id != config.binding.compartment_id ||
        request.observation_time <= core::SimulationClock::Duration::zero() ||
        !valid_fraction(request.initial_bound_fraction) || request.ligand_trajectory.empty() ||
        request.ligand_trajectory.front().offset != core::SimulationClock::Duration::zero()) {
        invalid("Time-varying receptor-ligand request is incompatible or incomplete");
    }

    auto previous = core::SimulationClock::Duration::min();
    double maximum_concentration = 0.0;
    for (const auto& knot : request.ligand_trajectory) {
        const auto concentration = core::in_moles_per_cubic_meter(knot.concentration);
        if (knot.offset <= previous || knot.offset < core::SimulationClock::Duration::zero() ||
            knot.offset >= request.observation_time || !std::isfinite(concentration) ||
            concentration < 0.0) {
            invalid("Ligand trajectory requires strictly increasing in-range knots and "
                    "non-negative concentrations");
        }
        previous = knot.offset;
        maximum_concentration = std::max(maximum_concentration, concentration);
    }

    const auto maximum_rate =
        core::in_cubic_meters_per_mole_second(config.binding.association_rate) *
            maximum_concentration +
        core::in_per_second(config.binding.dissociation_rate);
    if (!std::isfinite(maximum_rate) || seconds(config.integration_step) * maximum_rate > 1.0) {
        invalid("Integration step is too large for the configured binding-rate scale");
    }
}

[[nodiscard]] double derivative(const ReceptorLigandModelConfig& config,
                                const core::Concentration concentration,
                                const double bound_fraction) noexcept {
    const auto association = core::in_cubic_meters_per_mole_second(config.association_rate) *
                             core::in_moles_per_cubic_meter(concentration);
    return association * (1.0 - bound_fraction) -
           core::in_per_second(config.dissociation_rate) * bound_fraction;
}

[[nodiscard]] double rk4_step(const ReceptorLigandModelConfig& config,
                              const core::Concentration concentration, const double bound_fraction,
                              const core::SimulationClock::Duration step) {
    const auto step_seconds = seconds(step);
    const auto k1 = derivative(config, concentration, bound_fraction);
    const auto k2 = derivative(config, concentration, bound_fraction + 0.5 * step_seconds * k1);
    const auto k3 = derivative(config, concentration, bound_fraction + 0.5 * step_seconds * k2);
    const auto k4 = derivative(config, concentration, bound_fraction + step_seconds * k3);
    const auto next = bound_fraction + step_seconds * (k1 + 2.0 * k2 + 2.0 * k3 + k4) / 6.0;
    if (!std::isfinite(next) || next < -1.0e-12 || next > 1.0 + 1.0e-12) {
        invalid("Numerical receptor occupancy left its physical range");
    }
    return std::clamp(next, 0.0, 1.0);
}

} // namespace

void validate_time_varying_receptor_ligand_config(
    const TimeVaryingReceptorLigandModelConfig& config) {
    if (config.integration_step <= core::SimulationClock::Duration::zero() ||
        config.maximum_integration_steps == 0 || config.maximum_integration_steps > 1'000'000) {
        invalid("Time-varying receptor-ligand solver configuration is invalid");
    }
    static_cast<void>(AnalyticalReceptorLigandModel{config.binding});
}

TimeVaryingReceptorLigandModel::TimeVaryingReceptorLigandModel(
    TimeVaryingReceptorLigandModelConfig config)
    : config_{std::move(config)} {
    validate_time_varying_receptor_ligand_config(config_);
}

std::string_view TimeVaryingReceptorLigandModel::kind() const noexcept {
    return time_varying_receptor_ligand_kind;
}

std::string_view TimeVaryingReceptorLigandModel::model_id() const noexcept {
    return config_.binding.model_id;
}

TimeVaryingReceptorLigandResponse
TimeVaryingReceptorLigandModel::evaluate(const TimeVaryingReceptorLigandRequest& request) const {
    validate_request(config_, request);

    const auto threshold = config_.binding.detection_threshold_fraction;
    auto bound_fraction = request.initial_bound_fraction;
    auto peak_fraction = bound_fraction;
    auto current_time = core::SimulationClock::Duration::zero();
    std::optional<core::SimulationClock::Duration> crossing;
    if (bound_fraction >= threshold) {
        crossing = current_time;
    }

    std::vector<ReceptorBindingSample> samples;
    samples.reserve(std::min(config_.maximum_integration_steps + 1, std::size_t{100'001}));
    samples.push_back(
        {current_time, request.ligand_trajectory.front().concentration, bound_fraction});

    std::size_t integration_steps = 0;
    for (std::size_t segment = 0; segment < request.ligand_trajectory.size(); ++segment) {
        const auto concentration_quantity = request.ligand_trajectory[segment].concentration;
        const auto segment_end = segment + 1 < request.ligand_trajectory.size()
                                     ? request.ligand_trajectory[segment + 1].offset
                                     : request.observation_time;
        while (current_time < segment_end) {
            if (integration_steps >= config_.maximum_integration_steps) {
                invalid("Time-varying receptor-ligand request exceeds the integration-step bound");
            }
            const auto step = std::min(config_.integration_step, segment_end - current_time);
            const auto previous_fraction = bound_fraction;
            bound_fraction =
                rk4_step(config_.binding, concentration_quantity, bound_fraction, step);
            if (!crossing.has_value() && previous_fraction < threshold &&
                bound_fraction >= threshold) {
                const auto fraction =
                    (threshold - previous_fraction) / (bound_fraction - previous_fraction);
                crossing = current_time + duration_from_seconds(seconds(step) * fraction);
            }
            current_time += step;
            ++integration_steps;
            peak_fraction = std::max(peak_fraction, bound_fraction);
            samples.push_back({current_time, concentration_quantity, bound_fraction});
        }
    }

    const auto total_amount =
        config_.binding.total_receptor_concentration * config_.binding.cell_volume;
    const auto bound_amount = total_amount * bound_fraction;
    const auto free_amount = total_amount - bound_amount;
    return {
        request.request_id,
        config_.binding.receptor_id,
        request.ligand_id,
        config_.binding.model_id,
        request.compartment_id,
        request.observation_time,
        total_amount,
        free_amount,
        bound_amount,
        bound_fraction,
        peak_fraction,
        crossing.has_value(),
        crossing,
        integration_steps,
        std::move(samples),
    };
}

core::Amount accounted_receptor_amount(const TimeVaryingReceptorLigandResponse& response) noexcept {
    return response.free_receptor_amount + response.bound_receptor_amount;
}

double receptor_balance_error_moles(const TimeVaryingReceptorLigandResponse& response) noexcept {
    return std::abs(core::in_moles(response.total_receptor_amount) -
                    core::in_moles(accounted_receptor_amount(response)));
}

} // namespace mehlissa::models::cell

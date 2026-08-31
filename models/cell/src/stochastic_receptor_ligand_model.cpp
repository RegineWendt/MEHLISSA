// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/stochastic_receptor_ligand_model.hpp>

#include <mehlissa/core/error.hpp>

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

[[nodiscard]] double seconds(const core::SimulationClock::Duration duration) noexcept {
    return std::chrono::duration<double>{duration}.count();
}

[[nodiscard]] core::SimulationClock::Duration duration_from_seconds(const double value) {
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] double uniform_open(core::RandomStream& random) {
    constexpr auto inverse_two_to_53 = 1.0 / 9007199254740992.0;
    return (static_cast<double>(random.next_u64() >> 11U) + 0.5) * inverse_two_to_53;
}

void validate_request(const StochasticReceptorLigandModelConfig& config,
                      const StochasticReceptorLigandRequest& request) {
    if (request.request_id.empty() || request.ligand_id != config.ligand_id ||
        request.compartment_id != config.compartment_id ||
        request.observation_time <= core::SimulationClock::Duration::zero() ||
        request.initial_bound_receptors > config.receptor_count ||
        request.ligand_trajectory.empty() ||
        request.ligand_trajectory.front().offset != core::SimulationClock::Duration::zero()) {
        invalid("Stochastic receptor-ligand request is incompatible or incomplete");
    }
    auto previous = core::SimulationClock::Duration::min();
    for (const auto& knot : request.ligand_trajectory) {
        const auto concentration = core::in_moles_per_cubic_meter(knot.concentration);
        if (knot.offset <= previous || knot.offset < core::SimulationClock::Duration::zero() ||
            knot.offset >= request.observation_time || !std::isfinite(concentration) ||
            concentration < 0.0) {
            invalid("Stochastic ligand trajectory requires strictly increasing in-range knots and "
                    "non-negative concentrations");
        }
        previous = knot.offset;
    }
}

void retain_sample(std::vector<StochasticBindingSample>& samples, std::size_t& dropped,
                   const std::size_t maximum, const double time_seconds,
                   const std::uint32_t bound) {
    const StochasticBindingSample sample{duration_from_seconds(time_seconds), bound};
    if (samples.size() < maximum) {
        samples.push_back(sample);
    } else {
        samples.back() = sample;
        ++dropped;
    }
}

} // namespace

void validate_stochastic_receptor_ligand_config(const StochasticReceptorLigandModelConfig& config) {
    const auto association = core::in_cubic_meters_per_mole_second(config.association_rate);
    const auto dissociation = core::in_per_second(config.dissociation_rate);
    if (config.model_id.empty() || config.receptor_id.empty() || config.ligand_id.empty() ||
        config.compartment_id.empty() || config.receptor_count == 0 ||
        !std::isfinite(association) || association <= 0.0 || !std::isfinite(dissociation) ||
        dissociation < 0.0 || !std::isfinite(config.detection_threshold_fraction) ||
        config.detection_threshold_fraction <= 0.0 || config.detection_threshold_fraction > 1.0 ||
        config.maximum_reaction_events == 0 || config.maximum_reaction_events > 10'000'000 ||
        config.maximum_recorded_samples < 2 || config.maximum_recorded_samples > 1'000'000) {
        invalid("Stochastic receptor-ligand configuration is invalid");
    }
}

StochasticReceptorLigandModel::StochasticReceptorLigandModel(
    StochasticReceptorLigandModelConfig config)
    : config_{std::move(config)} {
    validate_stochastic_receptor_ligand_config(config_);
}

std::string_view StochasticReceptorLigandModel::kind() const noexcept {
    return stochastic_receptor_ligand_kind;
}

std::string_view StochasticReceptorLigandModel::model_id() const noexcept {
    return config_.model_id;
}

StochasticReceptorLigandResponse
StochasticReceptorLigandModel::evaluate(const StochasticReceptorLigandRequest& request,
                                        core::RandomStream& random) const {
    validate_request(config_, request);
    const auto draws_before = random.draw_count();
    const auto observation_seconds = seconds(request.observation_time);
    const auto threshold_count = static_cast<std::uint32_t>(
        std::ceil(config_.detection_threshold_fraction * config_.receptor_count));
    auto bound = request.initial_bound_receptors;
    auto peak = bound;
    auto time = 0.0;
    std::size_t events = 0;
    std::size_t dropped = 0;
    std::optional<core::SimulationClock::Duration> crossing;
    if (bound >= threshold_count) {
        crossing = core::SimulationClock::Duration::zero();
    }
    std::vector<StochasticBindingSample> samples;
    samples.reserve(config_.maximum_recorded_samples);
    samples.push_back({core::SimulationClock::Duration::zero(), bound});

    for (std::size_t segment = 0; segment < request.ligand_trajectory.size(); ++segment) {
        const auto concentration =
            core::in_moles_per_cubic_meter(request.ligand_trajectory[segment].concentration);
        const auto segment_end = segment + 1 < request.ligand_trajectory.size()
                                     ? seconds(request.ligand_trajectory[segment + 1].offset)
                                     : observation_seconds;
        while (time < segment_end) {
            const auto binding = core::in_cubic_meters_per_mole_second(config_.association_rate) *
                                 concentration *
                                 static_cast<double>(config_.receptor_count - bound);
            const auto dissociation =
                core::in_per_second(config_.dissociation_rate) * static_cast<double>(bound);
            const auto total = binding + dissociation;
            if (!(total > 0.0) || !std::isfinite(total)) {
                time = segment_end;
                break;
            }
            const auto event_time = time - std::log(uniform_open(random)) / total;
            if (event_time >= segment_end) {
                time = segment_end;
                break;
            }
            if (events >= config_.maximum_reaction_events) {
                invalid("Stochastic receptor-ligand request exceeds the reaction-event bound");
            }
            time = event_time;
            const auto previous = bound;
            if (uniform_open(random) * total < binding) {
                ++bound;
            } else {
                --bound;
            }
            ++events;
            peak = std::max(peak, bound);
            if (!crossing.has_value() && previous < threshold_count && bound >= threshold_count) {
                crossing = duration_from_seconds(time);
            }
            retain_sample(samples, dropped, config_.maximum_recorded_samples, time, bound);
        }
    }
    if (samples.back().offset != request.observation_time) {
        retain_sample(samples, dropped, config_.maximum_recorded_samples, observation_seconds,
                      bound);
    }

    const auto fraction = static_cast<double>(bound) / config_.receptor_count;
    return {request.request_id,
            config_.receptor_id,
            request.ligand_id,
            config_.model_id,
            request.compartment_id,
            request.observation_time,
            config_.receptor_count,
            config_.receptor_count - bound,
            bound,
            fraction,
            static_cast<double>(peak) / config_.receptor_count,
            crossing.has_value(),
            crossing,
            events,
            random.draw_count() - draws_before,
            dropped,
            std::move(samples)};
}

} // namespace mehlissa::models::cell

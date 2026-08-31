// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/intracellular_response_network.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <utility>

namespace mehlissa::models::cell {
namespace {

struct State final {
    double messenger{};
    double effector{};
};

struct Propensities final {
    double messenger_activation{};
    double messenger_deactivation{};
    double effector_activation{};
    double effector_deactivation{};

    [[nodiscard]] double total() const noexcept {
        return messenger_activation + messenger_deactivation + effector_activation +
               effector_deactivation;
    }
};

struct MoleculeCounts final {
    std::uint32_t messenger{};
    std::uint32_t effector{};
};

struct StateAdvance final {
    State state;
    State slope;
    double scale{};
};

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

void validate_request(const IntracellularNetworkRequest& request) {
    if (request.request_id.empty() ||
        request.observation_time <= core::SimulationClock::Duration::zero() ||
        !valid_fraction(request.initial_active_messenger_fraction) ||
        !valid_fraction(request.initial_active_effector_fraction) ||
        request.receptor_trajectory.empty() ||
        request.receptor_trajectory.front().offset != core::SimulationClock::Duration::zero()) {
        invalid("Intracellular-network request is incomplete or nonphysical");
    }
    auto previous = core::SimulationClock::Duration::min();
    for (const auto& knot : request.receptor_trajectory) {
        if (knot.offset <= previous || knot.offset < core::SimulationClock::Duration::zero() ||
            knot.offset >= request.observation_time || !valid_fraction(knot.bound_fraction)) {
            invalid("Receptor trajectory requires increasing in-range knots and fractions");
        }
        previous = knot.offset;
    }
}

[[nodiscard]] State derivative(const IntracellularNetworkKinetics& kinetics, const double receptor,
                               const State state) noexcept {
    const auto messenger_on = core::in_per_second(kinetics.messenger_activation_rate);
    const auto messenger_off = core::in_per_second(kinetics.messenger_deactivation_rate);
    const auto effector_on = core::in_per_second(kinetics.effector_activation_rate);
    const auto effector_off = core::in_per_second(kinetics.effector_deactivation_rate);
    return {messenger_on * receptor * (1.0 - state.messenger) - messenger_off * state.messenger,
            effector_on * state.messenger * (1.0 - state.effector) - effector_off * state.effector};
}

[[nodiscard]] State add_scaled(const StateAdvance& advance) noexcept {
    return {advance.state.messenger + advance.scale * advance.slope.messenger,
            advance.state.effector + advance.scale * advance.slope.effector};
}

[[nodiscard]] State rk4_step(const IntracellularNetworkKinetics& kinetics, const double receptor,
                             const State state, const double step) {
    const auto k1 = derivative(kinetics, receptor, state);
    const auto k2 = derivative(kinetics, receptor, add_scaled({state, k1, 0.5 * step}));
    const auto k3 = derivative(kinetics, receptor, add_scaled({state, k2, 0.5 * step}));
    const auto k4 = derivative(kinetics, receptor, add_scaled({state, k3, step}));
    const State next{
        state.messenger +
            step * (k1.messenger + 2.0 * k2.messenger + 2.0 * k3.messenger + k4.messenger) / 6.0,
        state.effector +
            step * (k1.effector + 2.0 * k2.effector + 2.0 * k3.effector + k4.effector) / 6.0};
    if (!valid_fraction(next.messenger) || !valid_fraction(next.effector)) {
        invalid("Intracellular ODE state left its physical fraction range");
    }
    return next;
}

void retain_sample(std::vector<IntracellularStateSample>& samples, std::size_t& dropped,
                   const std::size_t maximum, const IntracellularStateSample sample) {
    if (samples.size() < maximum) {
        samples.push_back(sample);
    } else {
        samples.back() = sample;
        ++dropped;
    }
}

[[nodiscard]] double uniform_open(core::RandomStream& random) {
    constexpr auto inverse_two_to_53 = 1.0 / 9007199254740992.0;
    return (static_cast<double>(random.next_u64() >> 11U) + 0.5) * inverse_two_to_53;
}

[[nodiscard]] Propensities propensities(const IntracellularSsaConfig& config, const double receptor,
                                        const MoleculeCounts counts) noexcept {
    const auto messenger_fraction =
        static_cast<double>(counts.messenger) / config.messenger_molecule_count;
    return {core::in_per_second(config.kinetics.messenger_activation_rate) * receptor *
                static_cast<double>(config.messenger_molecule_count - counts.messenger),
            core::in_per_second(config.kinetics.messenger_deactivation_rate) * counts.messenger,
            core::in_per_second(config.kinetics.effector_activation_rate) * messenger_fraction *
                static_cast<double>(config.effector_molecule_count - counts.effector),
            core::in_per_second(config.kinetics.effector_deactivation_rate) * counts.effector};
}

} // namespace

void validate_intracellular_kinetics(const IntracellularNetworkKinetics& kinetics) {
    const auto messenger_on = core::in_per_second(kinetics.messenger_activation_rate);
    const auto messenger_off = core::in_per_second(kinetics.messenger_deactivation_rate);
    const auto effector_on = core::in_per_second(kinetics.effector_activation_rate);
    const auto effector_off = core::in_per_second(kinetics.effector_deactivation_rate);
    if (kinetics.network_id.empty() || !std::isfinite(messenger_on) || messenger_on <= 0.0 ||
        !std::isfinite(messenger_off) || messenger_off < 0.0 || !std::isfinite(effector_on) ||
        effector_on <= 0.0 || !std::isfinite(effector_off) || effector_off < 0.0 ||
        !std::isfinite(kinetics.response_threshold_fraction) ||
        kinetics.response_threshold_fraction <= 0.0 || kinetics.response_threshold_fraction > 1.0) {
        invalid("Intracellular-network kinetics are invalid");
    }
}

IntracellularOdeModel::IntracellularOdeModel(IntracellularOdeConfig config)
    : config_{std::move(config)} {
    validate_intracellular_kinetics(config_.kinetics);
    if (config_.integration_step <= core::SimulationClock::Duration::zero() ||
        config_.maximum_integration_steps == 0 || config_.maximum_integration_steps > 10'000'000 ||
        config_.maximum_recorded_samples < 2 || config_.maximum_recorded_samples > 1'000'000) {
        invalid("Intracellular ODE configuration is invalid");
    }
    const auto fastest =
        std::max({core::in_per_second(config_.kinetics.messenger_activation_rate) +
                      core::in_per_second(config_.kinetics.messenger_deactivation_rate),
                  core::in_per_second(config_.kinetics.effector_activation_rate) +
                      core::in_per_second(config_.kinetics.effector_deactivation_rate)});
    if (seconds(config_.integration_step) * fastest > 1.0) {
        invalid("Intracellular ODE integration step exceeds its rate-scale bound");
    }
}

std::string_view IntracellularOdeModel::kind() const noexcept { return intracellular_ode_kind; }

IntracellularOdeResponse
IntracellularOdeModel::evaluate(const IntracellularNetworkRequest& request) const {
    validate_request(request);
    State state{request.initial_active_messenger_fraction,
                request.initial_active_effector_fraction};
    auto peak = state.effector;
    auto current = core::SimulationClock::Duration::zero();
    std::optional<core::SimulationClock::Duration> crossing;
    if (state.effector >= config_.kinetics.response_threshold_fraction) {
        crossing = current;
    }
    std::vector<IntracellularStateSample> samples;
    samples.reserve(config_.maximum_recorded_samples);
    samples.push_back({current, state.messenger, state.effector});
    std::size_t steps = 0;
    std::size_t dropped = 0;
    for (std::size_t segment = 0; segment < request.receptor_trajectory.size(); ++segment) {
        const auto receptor = request.receptor_trajectory[segment].bound_fraction;
        const auto end = segment + 1 < request.receptor_trajectory.size()
                             ? request.receptor_trajectory[segment + 1].offset
                             : request.observation_time;
        while (current < end) {
            if (steps >= config_.maximum_integration_steps) {
                invalid("Intracellular ODE request exceeds its integration-step bound");
            }
            const auto step = std::min(config_.integration_step, end - current);
            const auto previous_effector = state.effector;
            state = rk4_step(config_.kinetics, receptor, state, seconds(step));
            if (!crossing.has_value() &&
                previous_effector < config_.kinetics.response_threshold_fraction &&
                state.effector >= config_.kinetics.response_threshold_fraction) {
                const auto alpha =
                    (config_.kinetics.response_threshold_fraction - previous_effector) /
                    (state.effector - previous_effector);
                crossing = current + duration_from_seconds(seconds(step) * alpha);
            }
            current += step;
            ++steps;
            peak = std::max(peak, state.effector);
            retain_sample(samples, dropped, config_.maximum_recorded_samples,
                          {current, state.messenger, state.effector});
        }
    }
    return {request.request_id,
            config_.kinetics.network_id,
            state.messenger,
            state.effector,
            peak,
            crossing.has_value(),
            crossing,
            steps,
            dropped,
            std::move(samples)};
}

IntracellularSsaModel::IntracellularSsaModel(IntracellularSsaConfig config)
    : config_{std::move(config)} {
    validate_intracellular_kinetics(config_.kinetics);
    if (config_.messenger_molecule_count == 0 || config_.effector_molecule_count == 0 ||
        config_.maximum_reaction_events == 0 || config_.maximum_reaction_events > 100'000'000 ||
        config_.maximum_recorded_samples < 2 || config_.maximum_recorded_samples > 1'000'000) {
        invalid("Intracellular SSA configuration is invalid");
    }
}

std::string_view IntracellularSsaModel::kind() const noexcept { return intracellular_ssa_kind; }

IntracellularSsaResponse IntracellularSsaModel::evaluate(const IntracellularNetworkRequest& request,
                                                         core::RandomStream& random) const {
    validate_request(request);
    auto messenger = static_cast<std::uint32_t>(
        std::lround(request.initial_active_messenger_fraction * config_.messenger_molecule_count));
    auto effector = static_cast<std::uint32_t>(
        std::lround(request.initial_active_effector_fraction * config_.effector_molecule_count));
    auto peak = effector;
    auto time = 0.0;
    std::size_t events = 0;
    std::size_t dropped = 0;
    const auto draws_before = random.draw_count();
    const auto threshold_count = static_cast<std::uint32_t>(
        std::ceil(config_.kinetics.response_threshold_fraction * config_.effector_molecule_count));
    std::optional<core::SimulationClock::Duration> crossing;
    if (effector >= threshold_count) {
        crossing = core::SimulationClock::Duration::zero();
    }
    std::vector<IntracellularStateSample> samples;
    samples.reserve(config_.maximum_recorded_samples);
    samples.push_back({core::SimulationClock::Duration::zero(),
                       static_cast<double>(messenger) / config_.messenger_molecule_count,
                       static_cast<double>(effector) / config_.effector_molecule_count});
    for (std::size_t segment = 0; segment < request.receptor_trajectory.size(); ++segment) {
        const auto receptor = request.receptor_trajectory[segment].bound_fraction;
        const auto end = segment + 1 < request.receptor_trajectory.size()
                             ? seconds(request.receptor_trajectory[segment + 1].offset)
                             : seconds(request.observation_time);
        while (time < end) {
            const auto rates = propensities(config_, receptor, {messenger, effector});
            const auto total = rates.total();
            if (!(total > 0.0) || !std::isfinite(total)) {
                time = end;
                break;
            }
            const auto event_time = time - std::log(uniform_open(random)) / total;
            if (event_time >= end) {
                time = end;
                break;
            }
            if (events >= config_.maximum_reaction_events) {
                invalid("Intracellular SSA request exceeds its reaction-event bound");
            }
            time = event_time;
            const auto selector = uniform_open(random) * total;
            const auto messenger_deactivation_boundary =
                rates.messenger_activation + rates.messenger_deactivation;
            const auto effector_activation_boundary =
                messenger_deactivation_boundary + rates.effector_activation;
            if (selector < rates.messenger_activation) {
                ++messenger;
            } else if (selector < messenger_deactivation_boundary) {
                --messenger;
            } else if (selector < effector_activation_boundary) {
                ++effector;
            } else {
                --effector;
            }
            ++events;
            peak = std::max(peak, effector);
            if (!crossing.has_value() && effector >= threshold_count) {
                crossing = duration_from_seconds(time);
            }
            retain_sample(samples, dropped, config_.maximum_recorded_samples,
                          {duration_from_seconds(time),
                           static_cast<double>(messenger) / config_.messenger_molecule_count,
                           static_cast<double>(effector) / config_.effector_molecule_count});
        }
    }
    retain_sample(samples, dropped, config_.maximum_recorded_samples,
                  {request.observation_time,
                   static_cast<double>(messenger) / config_.messenger_molecule_count,
                   static_cast<double>(effector) / config_.effector_molecule_count});
    return {request.request_id,
            config_.kinetics.network_id,
            config_.messenger_molecule_count,
            messenger,
            config_.effector_molecule_count,
            effector,
            static_cast<double>(messenger) / config_.messenger_molecule_count,
            static_cast<double>(effector) / config_.effector_molecule_count,
            static_cast<double>(peak) / config_.effector_molecule_count,
            crossing.has_value(),
            crossing,
            events,
            random.draw_count() - draws_before,
            dropped,
            std::move(samples)};
}

} // namespace mehlissa::models::cell

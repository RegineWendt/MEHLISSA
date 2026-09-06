// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "mehlissa/scenarios/fdg_pet/fdg_pet_model.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace mehlissa::scenarios::fdg_pet {
namespace {
using State = std::array<double, 7>;

double blood_rate(const CandidateParameters& p, double seconds_value) {
    double area{};
    for (std::size_t i{}; i < 4; ++i) {
        area += p.blood_amplitudes_per_minute_squared[i] / p.blood_decay_per_minute[i] / 60.0;
    }
    const double bolus = core::in_seconds(p.administration.injection_duration);
    if (bolus <= 0.0 || area <= 0.0) {
        throw std::invalid_argument("positive bolus and blood kernel are required");
    }
    // Analytic rectangular-bolus convolution of the source-disjoint exponential kernel.
    double convolved{};
    for (std::size_t i{}; i < 4; ++i) {
        const double lambda = p.blood_decay_per_minute[i] / 60.0;
        const double lower = std::max(0.0, seconds_value - bolus);
        if (seconds_value > 0.0) {
            convolved += p.blood_amplitudes_per_minute_squared[i] / 3600.0 *
                         (std::exp(-lambda * lower) - std::exp(-lambda * seconds_value)) /
                         (lambda * bolus);
        }
    }
    return convolved / area;
}

double plasma(const CandidateParameters& p, double time) {
    return in_becquerels(p.administration.injected_activity) /
           core::in_cubic_meters_per_second(p.cardiac_output) * blood_rate(p, time);
}

State derivative(const CandidateParameters& p, double time, const State& y) {
    const double cp = plasma(p, time);
    State d{};
    const auto tissue = [&](std::size_t free_index, const TissueKinetics& k) {
        d[free_index] = core::in_per_second(k.k1) * cp -
                        (core::in_per_second(k.k2) + core::in_per_second(k.k3)) * y[free_index] +
                        core::in_per_second(k.k4) * y[free_index + 1];
        d[free_index + 1] = core::in_per_second(k.k3) * y[free_index] -
                            core::in_per_second(k.k4) * y[free_index + 1];
    };
    tissue(0, p.lung);
    tissue(2, p.liver);
    tissue(4, p.kidney);
    d[6] = core::in_per_second(p.renal_excretion) * y[4] * core::in_cubic_meters(p.kidney.volume);
    return d;
}

State add(const State& y, const State& d, double factor) {
    State result{};
    for (std::size_t i{}; i < result.size(); ++i) {
        result[i] = y[i] + factor * d[i];
    }
    return result;
}

State step(const CandidateParameters& p, double time, const State& y, double dt) {
    const auto k1 = derivative(p, time, y);
    const auto k2 = derivative(p, time + dt / 2.0, add(y, k1, dt / 2.0));
    const auto k3 = derivative(p, time + dt / 2.0, add(y, k2, dt / 2.0));
    const auto k4 = derivative(p, time + dt, add(y, k3, dt));
    State result{};
    for (std::size_t i{}; i < result.size(); ++i) {
        result[i] = std::max(0.0, y[i] + dt * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) / 6.0);
    }
    return result;
}

std::array<double, 5> observe(const CandidateParameters& p, double time, const State& y) {
    const double cp = plasma(p, time);
    const double decay =
        p.decay_reference == DecayReference::injection_time_corrected
            ? 1.0
            : std::exp(-std::log(2.0) * time / core::in_seconds(p.fluorine18_half_life));
    const auto tissue = [&](std::size_t index, const TissueKinetics& k) {
        return decay *
               (k.vascular_fraction * cp + (1.0 - k.vascular_fraction) * (y[index] + y[index + 1]));
    };
    return {decay * cp, tissue(0, p.lung), tissue(2, p.liver), tissue(4, p.kidney), decay * y[6]};
}
} // namespace

CandidateParameters source_disjoint_reference_candidate(const Administration& administration) {
    CandidateParameters p{};
    p.administration = administration;
    p.cardiac_output =
        core::liters_per_minute(5.0 * std::pow(administration.body_mass.si_value() / 70.0, 0.75));
    p.blood_amplitudes_per_minute_squared = {11.1, 0.022, 0.016, 0.016};
    p.blood_decay_per_minute = {14.7, 0.53, 0.089, 0.0085};
    const auto per_minute = [](double value) { return core::per_second(value / 60.0); };
    p.lung = {per_minute(0.108), per_minute(0.735), per_minute(0.016), per_minute(0.013), 0.017,
              core::liters(4.7)};
    p.liver = {per_minute(0.331), per_minute(0.440), per_minute(0.017), per_minute(0.018), 0.20,
               core::liters(1.5)};
    p.kidney = {
        per_minute(0.594),       per_minute(0.428), per_minute(0.013), per_minute(0.0), 0.066,
        core::milliliters(305.0)};
    p.renal_excretion = per_minute(0.10);
    p.fluorine18_half_life = core::minutes(109.77);
    return p;
}

std::vector<FramePrediction> simulate(const CandidateParameters& p,
                                      const std::vector<Frame>& frames, core::Time maximum_step) {
    if (frames.empty() || core::in_seconds(maximum_step) <= 0.0 ||
        p.administration.body_mass.si_value() <= 0.0 ||
        in_becquerels(p.administration.injected_activity) <= 0.0 ||
        core::in_cubic_meters_per_second(p.cardiac_output) <= 0.0) {
        throw std::invalid_argument(
            "positive administration, flow, step and at least one frame are required");
    }
    double previous_end{};
    for (const auto& frame : frames) {
        const double start = core::in_seconds(frame.start);
        const double duration = core::in_seconds(frame.duration);
        if (start < previous_end || duration <= 0.0) {
            throw std::invalid_argument("frames must be positive and non-overlapping");
        }
        previous_end = start + duration;
    }
    std::vector<std::array<double, 5>> integrals(frames.size());
    State state{};
    double time{};
    while (time < previous_end - 1.0e-12) {
        std::size_t frame_index{};
        while (frame_index < frames.size() &&
               time >= core::in_seconds(frames[frame_index].start + frames[frame_index].duration) -
                           1.0e-12) {
            ++frame_index;
        }
        double boundary = previous_end;
        bool inside = false;
        if (frame_index < frames.size()) {
            const double start = core::in_seconds(frames[frame_index].start);
            const double end = start + core::in_seconds(frames[frame_index].duration);
            inside = time >= start - 1.0e-12;
            boundary = inside ? end : start;
        }
        const double dt = std::min(core::in_seconds(maximum_step), boundary - time);
        if (dt <= 1.0e-12) {
            time = boundary;
            continue;
        }
        const auto before = observe(p, time, state);
        const auto next = step(p, time, state, dt);
        const auto after = observe(p, time + dt, next);
        if (inside) {
            for (std::size_t j{}; j < 5; ++j) {
                integrals[frame_index][j] += dt * (before[j] + after[j]) / 2.0;
            }
        }
        state = next;
        time += dt;
    }
    std::vector<FramePrediction> result;
    result.reserve(frames.size());
    for (std::size_t i{}; i < frames.size(); ++i) {
        const double duration = core::in_seconds(frames[i].duration);
        result.push_back({frames[i], ActivityConcentration::from_si(integrals[i][0] / duration),
                          ActivityConcentration::from_si(integrals[i][1] / duration),
                          ActivityConcentration::from_si(integrals[i][2] / duration),
                          ActivityConcentration::from_si(integrals[i][3] / duration),
                          Activity::from_si(integrals[i][4] / duration)});
    }
    return result;
}

} // namespace mehlissa::scenarios::fdg_pet

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/compartment_transport.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace mehlissa::models::body {
namespace {

constexpr auto transition_stream_name = "body.compartment-transport.transitions";
constexpr double random_resolution = 9'007'199'254'740'992.0;

[[nodiscard]] core::SimulationClock::Duration transit_time(const VascularSegment& segment) {
    const auto length = core::in_meters(segment.geometry.length);
    const auto speed = core::in_meters_per_second(segment.hemodynamics.mean_velocity);
    const auto nanoseconds = (length / speed) * 1'000'000'000.0;
    const auto maximum =
        static_cast<double>(std::numeric_limits<core::SimulationClock::Duration::rep>::max());
    if (!std::isfinite(nanoseconds) || nanoseconds <= 0.0 || nanoseconds > maximum) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "Segment '" + segment.id +
                                      "' has an unrepresentable transit time"};
    }
    return core::SimulationClock::Duration{
        static_cast<core::SimulationClock::Duration::rep>(std::ceil(nanoseconds))};
}

[[nodiscard]] core::SimulationClock::Duration
checked_add(const core::SimulationClock::Duration left,
            const core::SimulationClock::Duration right) {
    const auto maximum = core::SimulationClock::Duration::max().count();
    if (right.count() > maximum - left.count()) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow, "Transport duration overflow"};
    }
    return left + right;
}

} // namespace

CompartmentTransport::CompartmentTransport(VascularGraph graph,
                                           std::vector<InjectionEvent> injections)
    : graph_(std::move(graph)), injections_(std::move(injections)) {
    validate_vascular_graph(graph_);

    segment_indices_.reserve(graph_.segments.size());
    transit_times_.reserve(graph_.segments.size());
    for (std::size_t index = 0; index < graph_.segments.size(); ++index) {
        segment_indices_.emplace(graph_.segments[index].id, index);
        transit_times_.push_back(transit_time(graph_.segments[index]));
    }
    maximum_advance_ = *std::ranges::min_element(transit_times_);

    for (const auto& injection : injections_) {
        if (injection.time < core::SimulationClock::Duration::zero()) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Injection time must not be negative"};
        }
        if (injection.particle_count == 0) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Injection particle count must be positive"};
        }
        if (!segment_indices_.contains(injection.segment_id)) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Injection references unknown segment '" +
                                          injection.segment_id + "'"};
        }
    }
    std::stable_sort(injections_.begin(), injections_.end(),
                     [](const InjectionEvent& left, const InjectionEvent& right) {
                         return left.time < right.time;
                     });
}

std::string_view CompartmentTransport::name() const noexcept {
    return "body.compartment-transport";
}

void CompartmentTransport::initialize(core::SimulationContext& context) {
    if (initialized_ || finalized_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Compartment transport can only be initialized once"};
    }
    if (context.clock().now() != core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Compartment transport must initialize at time zero"};
    }
    initialized_ = true;
    inject_until(core::SimulationClock::Duration::zero());
    verify_population_invariant();
}

void CompartmentTransport::advance(core::SimulationContext& context,
                                   const core::SimulationClock::Duration delta) {
    if (!initialized_ || finalized_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only initialized compartment transport can advance"};
    }
    if (delta <= core::SimulationClock::Duration::zero() || delta > maximum_advance_) {
        throw core::MehlissaError{
            core::ErrorCode::invariant_violated,
            "Transport advance must be positive and no greater than the shortest segment "
            "transit time"};
    }

    const auto interval_end = checked_add(context.clock().now(), delta);
    for (auto& particle : particles_) {
        particle.residence_time = checked_add(particle.residence_time, delta);
    }
    inject_until(interval_end);

    struct StagedTransition final {
        std::size_t particle_index{};
        std::size_t successor_index{};
    };
    std::vector<StagedTransition> transitions;
    transitions.reserve(particles_.size());
    for (std::size_t index = 0; index < particles_.size(); ++index) {
        const auto& particle = particles_[index];
        if (particle.residence_time >= transit_times_[particle.segment_index]) {
            transitions.push_back({index, choose_successor(particle.segment_index, context)});
        }
    }

    if (transitions.size() > std::numeric_limits<std::uint64_t>::max() - transition_count_) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "Transport transition counter overflow"};
    }
    for (const auto& transition : transitions) {
        auto& particle = particles_[transition.particle_index];
        particle.residence_time -= transit_times_[particle.segment_index];
        particle.segment_index = transition.successor_index;
    }
    transition_count_ += static_cast<std::uint64_t>(transitions.size());
    verify_population_invariant();
}

void CompartmentTransport::finalize(core::SimulationContext& context) noexcept {
    static_cast<void>(context);
    finalized_ = true;
}

core::SimulationClock::Duration CompartmentTransport::maximum_advance() const noexcept {
    return maximum_advance_;
}

std::uint64_t CompartmentTransport::injected_particle_count() const noexcept {
    return injected_particle_count_;
}

std::uint64_t CompartmentTransport::transition_count() const noexcept { return transition_count_; }

std::size_t CompartmentTransport::particle_count() const noexcept { return particles_.size(); }

std::vector<ParticleLocation> CompartmentTransport::particle_locations() const {
    std::vector<ParticleLocation> result;
    result.reserve(particles_.size());
    for (const auto& particle : particles_) {
        result.push_back(
            {particle.id, graph_.segments[particle.segment_index].id, particle.residence_time});
    }
    return result;
}

std::vector<SegmentPopulation> CompartmentTransport::segment_populations() const {
    std::vector<std::uint64_t> counts(graph_.segments.size());
    for (const auto& particle : particles_) {
        ++counts[particle.segment_index];
    }

    std::vector<SegmentPopulation> result;
    result.reserve(graph_.segments.size());
    for (std::size_t index = 0; index < graph_.segments.size(); ++index) {
        result.push_back({graph_.segments[index].id, counts[index]});
    }
    return result;
}

void CompartmentTransport::inject_until(const core::SimulationClock::Duration cutoff) {
    while (next_injection_ < injections_.size() && injections_[next_injection_].time <= cutoff) {
        const auto& injection = injections_[next_injection_];
        const auto available = std::numeric_limits<std::size_t>::max() - particles_.size();
        if (injection.particle_count > static_cast<std::uint64_t>(available) ||
            injection.particle_count >
                std::numeric_limits<std::uint64_t>::max() - injected_particle_count_ ||
            injection.particle_count >
                std::numeric_limits<std::uint64_t>::max() - next_particle_id_ + 1) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Injection exceeds transport particle capacity"};
        }

        const auto segment_index = segment_indices_.at(injection.segment_id);
        const auto residence_time = cutoff - injection.time;
        for (std::uint64_t count = 0; count < injection.particle_count; ++count) {
            particles_.push_back({next_particle_id_, segment_index, residence_time});
            ++next_particle_id_;
        }
        injected_particle_count_ += injection.particle_count;
        ++next_injection_;
    }
}

std::size_t CompartmentTransport::choose_successor(const std::size_t segment_index,
                                                   core::SimulationContext& context) const {
    const auto& transitions = graph_.segments[segment_index].transitions;
    if (transitions.size() == 1) {
        return segment_indices_.at(transitions.front().successor_id);
    }

    const auto sample = context.random_stream(transition_stream_name).next_u64() >> 11U;
    const auto unit = static_cast<double>(sample) / random_resolution;
    double cumulative{};
    for (std::size_t index = 0; index + 1 < transitions.size(); ++index) {
        cumulative += transitions[index].probability;
        if (unit < cumulative) {
            return segment_indices_.at(transitions[index].successor_id);
        }
    }
    return segment_indices_.at(transitions.back().successor_id);
}

void CompartmentTransport::verify_population_invariant() const {
    if (particles_.size() != injected_particle_count_) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Transport population is not conserved"};
    }
}

} // namespace mehlissa::models::body

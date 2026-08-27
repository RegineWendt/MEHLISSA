// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/compartment_transport.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
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
    : CompartmentTransport(std::move(graph), std::move(injections), {}, {}) {}

CompartmentTransport::CompartmentTransport(VascularGraph graph,
                                           std::vector<InjectionEvent> injections,
                                           std::vector<ExtractionEvent> extractions,
                                           TransportObservationConfig observation_config)
    : graph_(std::move(graph)), injections_(std::move(injections)),
      extractions_(std::move(extractions)), observation_config_(std::move(observation_config)) {
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

    for (const auto& extraction : extractions_) {
        if (extraction.time < core::SimulationClock::Duration::zero()) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Extraction time must not be negative"};
        }
        if (extraction.particle_count.has_value() && extraction.particle_count.value() == 0) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Extraction particle count must be positive when present"};
        }
        if (!segment_indices_.contains(extraction.segment_id)) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Extraction references unknown segment '" +
                                          extraction.segment_id + "'"};
        }
    }
    std::stable_sort(extractions_.begin(), extractions_.end(),
                     [](const ExtractionEvent& left, const ExtractionEvent& right) {
                         return left.time < right.time;
                     });

    if (observation_config_.aggregate_interval < core::SimulationClock::Duration::zero()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Aggregate interval must not be negative"};
    }
    if (observation_config_.aggregate_interval > core::SimulationClock::Duration::zero() &&
        observation_config_.maximum_aggregate_records == 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Enabled aggregates require a positive record limit"};
    }
    if (observation_config_.trajectory_selection != TrajectorySelection::none &&
        observation_config_.maximum_trajectory_records == 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Enabled trajectories require a positive record limit"};
    }
    if (observation_config_.trajectory_selection == TrajectorySelection::first_n &&
        observation_config_.trajectory_particle_limit == 0) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "First-N trajectories require a positive particle limit"};
    }

    measurement_sites_by_segment_.resize(graph_.segments.size());
    std::unordered_set<std::string> measurement_site_ids;
    measurement_site_ids.reserve(observation_config_.measurement_sites.size());
    measurement_counts_.reserve(observation_config_.measurement_sites.size());
    for (std::size_t index = 0; index < observation_config_.measurement_sites.size(); ++index) {
        const auto& site = observation_config_.measurement_sites[index];
        if (site.id.empty() || !measurement_site_ids.emplace(site.id).second) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Measurement site IDs must be non-empty and unique"};
        }
        const auto segment = segment_indices_.find(site.segment_id);
        if (segment == segment_indices_.end()) {
            throw core::MehlissaError{core::ErrorCode::data_invalid,
                                      "Measurement site references unknown segment '" +
                                          site.segment_id + "'"};
        }
        measurement_sites_by_segment_[segment->second].push_back(index);
        measurement_counts_.push_back({site.id, site.segment_id, site.kind, 0});
    }
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
    extract_until(core::SimulationClock::Duration::zero());
    if (observation_config_.aggregate_interval > core::SimulationClock::Duration::zero()) {
        capture_population_snapshot(core::SimulationClock::Duration::zero());
        next_aggregate_time_ = observation_config_.aggregate_interval;
    }
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
        const auto transit = transit_times_[particle.segment_index];
        const auto overshoot = particle.residence_time - transit;
        const auto transition_time = interval_end - overshoot;
        particle.residence_time = overshoot;
        particle.segment_index = transition.successor_index;
        record_entry(transition_time, particle, TrajectoryAction::entered_segment);
    }
    transition_count_ += static_cast<std::uint64_t>(transitions.size());
    extract_until(interval_end);
    if (observation_config_.aggregate_interval > core::SimulationClock::Duration::zero() &&
        interval_end >= next_aggregate_time_) {
        capture_population_snapshot(interval_end);
        next_aggregate_time_ = checked_add(interval_end, observation_config_.aggregate_interval);
    }
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

std::uint64_t CompartmentTransport::extracted_particle_count() const noexcept {
    return extracted_particle_count_;
}

std::uint64_t CompartmentTransport::transition_count() const noexcept { return transition_count_; }

std::size_t CompartmentTransport::particle_count() const noexcept { return particles_.size(); }

void CompartmentTransport::handoff_particle(const std::uint64_t particle_id,
                                            const std::string_view expected_segment_id) {
    if (!initialized_ || finalized_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only initialized body transport can hand off particles"};
    }
    const auto expected_segment = segment_indices_.find(std::string{expected_segment_id});
    if (expected_segment == segment_indices_.end()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Body hand-off references an unknown segment"};
    }
    const auto particle = std::ranges::find(particles_, particle_id, &Particle::id);
    if (particle == particles_.end() || particle->segment_index != expected_segment->second) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Particle is not present at the requested body hand-off segment"};
    }
    if (!outside_body_particle_ids_.insert(particle_id).second) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Particle is already outside the body model"};
    }
    particles_.erase(particle);
    verify_population_invariant();
}

void CompartmentTransport::receive_returned_particle(const std::uint64_t particle_id,
                                                     const std::string_view segment_id,
                                                     const core::SimulationClock::Duration time) {
    if (!initialized_ || finalized_) {
        throw core::MehlissaError{core::ErrorCode::lifecycle_invalid,
                                  "Only initialized body transport can receive particles"};
    }
    const auto segment = segment_indices_.find(std::string{segment_id});
    if (segment == segment_indices_.end()) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Body return references an unknown segment"};
    }
    if (!outside_body_particle_ids_.contains(particle_id)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Returned particle was not handed off by this body model"};
    }
    particles_.push_back({particle_id, segment->second, {}});
    outside_body_particle_ids_.erase(particle_id);
    record_entry(time, particles_.back(), TrajectoryAction::entered_segment);
    verify_population_invariant();
}

std::size_t CompartmentTransport::outside_body_particle_count() const noexcept {
    return outside_body_particle_ids_.size();
}

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

const VascularGraph& CompartmentTransport::graph() const noexcept { return graph_; }

const TransportObservationConfig& CompartmentTransport::observation_config() const noexcept {
    return observation_config_;
}

const std::vector<TrajectoryRecord>& CompartmentTransport::trajectory_records() const noexcept {
    return trajectory_records_;
}

const std::vector<MeasurementRecord>& CompartmentTransport::measurement_records() const noexcept {
    return measurement_records_;
}

const std::vector<MeasurementCount>& CompartmentTransport::measurement_counts() const noexcept {
    return measurement_counts_;
}

const std::vector<PopulationSnapshot>& CompartmentTransport::population_snapshots() const noexcept {
    return population_snapshots_;
}

const std::vector<ExtractionResult>& CompartmentTransport::extraction_results() const noexcept {
    return extraction_results_;
}

bool CompartmentTransport::trajectories_truncated() const noexcept {
    return trajectories_truncated_;
}

bool CompartmentTransport::measurements_truncated() const noexcept {
    return measurements_truncated_;
}

bool CompartmentTransport::aggregates_truncated() const noexcept { return aggregates_truncated_; }

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
            record_entry(injection.time, particles_.back(), TrajectoryAction::injected);
            ++next_particle_id_;
        }
        injected_particle_count_ += injection.particle_count;
        ++next_injection_;
    }
}

void CompartmentTransport::extract_until(const core::SimulationClock::Duration cutoff) {
    while (next_extraction_ < extractions_.size() &&
           extractions_[next_extraction_].time <= cutoff) {
        const auto& extraction = extractions_[next_extraction_];
        const auto segment_index = segment_indices_.at(extraction.segment_id);
        const auto limit =
            extraction.particle_count.value_or(std::numeric_limits<std::uint64_t>::max());
        std::uint64_t extracted{};
        std::vector<Particle> remaining;
        remaining.reserve(particles_.size());
        for (auto& particle : particles_) {
            if (particle.segment_index == segment_index && extracted < limit) {
                record_trajectory(cutoff, particle, TrajectoryAction::extracted);
                ++extracted;
            } else {
                remaining.push_back(particle);
            }
        }
        particles_ = std::move(remaining);
        if (extracted > std::numeric_limits<std::uint64_t>::max() - extracted_particle_count_) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Extraction counter overflow"};
        }
        extracted_particle_count_ += extracted;
        extraction_results_.push_back(
            {extraction.time, cutoff, extraction.segment_id, extraction.particle_count, extracted});
        ++next_extraction_;
    }
}

void CompartmentTransport::record_entry(const core::SimulationClock::Duration time,
                                        const Particle& particle, const TrajectoryAction action) {
    record_trajectory(time, particle, action);
    for (const auto site_index : measurement_sites_by_segment_[particle.segment_index]) {
        auto& count = measurement_counts_[site_index].particle_count;
        if (count == std::numeric_limits<std::uint64_t>::max()) {
            throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                      "Measurement counter overflow"};
        }
        ++count;
        const auto& site = observation_config_.measurement_sites[site_index];
        if (measurement_records_.size() < observation_config_.maximum_measurement_records) {
            measurement_records_.push_back(
                {time, site.id, site.segment_id, site.kind, particle.id});
        } else {
            measurements_truncated_ = true;
        }
    }
}

void CompartmentTransport::record_trajectory(const core::SimulationClock::Duration time,
                                             const Particle& particle,
                                             const TrajectoryAction action) {
    const auto selected =
        observation_config_.trajectory_selection == TrajectorySelection::all ||
        (observation_config_.trajectory_selection == TrajectorySelection::first_n &&
         particle.id <= observation_config_.trajectory_particle_limit);
    if (!selected) {
        return;
    }
    if (trajectory_records_.size() < observation_config_.maximum_trajectory_records) {
        trajectory_records_.push_back(
            {time, particle.id, action, graph_.segments[particle.segment_index].id});
    } else {
        trajectories_truncated_ = true;
    }
}

void CompartmentTransport::capture_population_snapshot(const core::SimulationClock::Duration time) {
    if (population_snapshots_.size() < observation_config_.maximum_aggregate_records) {
        population_snapshots_.push_back({time, segment_populations()});
    } else {
        aggregates_truncated_ = true;
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
    const auto accounted = particles_.size() + outside_body_particle_ids_.size();
    if (extracted_particle_count_ > injected_particle_count_ ||
        accounted != injected_particle_count_ - extracted_particle_count_) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Transport population is not conserved"};
    }
}

} // namespace mehlissa::models::body

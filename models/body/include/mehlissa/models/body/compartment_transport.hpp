// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP
#define MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mehlissa::models::body {

struct InjectionEvent final {
    core::SimulationClock::Duration time;
    std::string segment_id;
    std::uint64_t particle_count{};
};

struct ExtractionEvent final {
    core::SimulationClock::Duration time;
    std::string segment_id;
    std::optional<std::uint64_t> particle_count;
};

enum class MeasurementSiteKind : std::uint8_t { sample, gateway };

struct MeasurementSite final {
    std::string id;
    std::string segment_id;
    MeasurementSiteKind kind{MeasurementSiteKind::sample};
};

enum class TrajectorySelection : std::uint8_t { none, all, first_n };
enum class TrajectoryAction : std::uint8_t { injected, entered_segment, extracted };

struct TransportObservationConfig final {
    std::vector<MeasurementSite> measurement_sites;
    TrajectorySelection trajectory_selection{TrajectorySelection::none};
    std::uint64_t trajectory_particle_limit{};
    std::size_t maximum_trajectory_records{};
    std::size_t maximum_measurement_records{};
    core::SimulationClock::Duration aggregate_interval{};
    std::size_t maximum_aggregate_records{};
};

struct ParticleLocation final {
    std::uint64_t particle_id{};
    std::string segment_id;
    core::SimulationClock::Duration residence_time;

    [[nodiscard]] bool operator==(const ParticleLocation&) const noexcept = default;
};

struct SegmentPopulation final {
    std::string segment_id;
    std::uint64_t particle_count{};

    [[nodiscard]] bool operator==(const SegmentPopulation&) const noexcept = default;
};

struct TrajectoryRecord final {
    core::SimulationClock::Duration time;
    std::uint64_t particle_id{};
    TrajectoryAction action{TrajectoryAction::injected};
    std::string segment_id;

    [[nodiscard]] bool operator==(const TrajectoryRecord&) const noexcept = default;
};

struct MeasurementRecord final {
    core::SimulationClock::Duration time;
    std::string site_id;
    std::string segment_id;
    MeasurementSiteKind kind{MeasurementSiteKind::sample};
    std::uint64_t particle_id{};

    [[nodiscard]] bool operator==(const MeasurementRecord&) const noexcept = default;
};

struct MeasurementCount final {
    std::string site_id;
    std::string segment_id;
    MeasurementSiteKind kind{MeasurementSiteKind::sample};
    std::uint64_t particle_count{};

    [[nodiscard]] bool operator==(const MeasurementCount&) const noexcept = default;
};

struct PopulationSnapshot final {
    core::SimulationClock::Duration time;
    std::vector<SegmentPopulation> populations;

    [[nodiscard]] bool operator==(const PopulationSnapshot&) const noexcept = default;
};

struct ExtractionResult final {
    core::SimulationClock::Duration scheduled_time;
    core::SimulationClock::Duration processed_time;
    std::string segment_id;
    std::optional<std::uint64_t> requested_particle_count;
    std::uint64_t extracted_particle_count{};

    [[nodiscard]] bool operator==(const ExtractionResult&) const noexcept = default;
};

class CompartmentTransport final : public core::SimulationComponent {
  public:
    CompartmentTransport(VascularGraph graph, std::vector<InjectionEvent> injections);
    CompartmentTransport(VascularGraph graph, std::vector<InjectionEvent> injections,
                         std::vector<ExtractionEvent> extractions,
                         TransportObservationConfig observation_config);

    [[nodiscard]] std::string_view name() const noexcept override;
    void initialize(core::SimulationContext& context) override;
    void advance(core::SimulationContext& context, core::SimulationClock::Duration delta) override;
    void finalize(core::SimulationContext& context) noexcept override;

    [[nodiscard]] core::SimulationClock::Duration maximum_advance() const noexcept;
    [[nodiscard]] std::uint64_t injected_particle_count() const noexcept;
    [[nodiscard]] std::uint64_t extracted_particle_count() const noexcept;
    [[nodiscard]] std::uint64_t transition_count() const noexcept;
    [[nodiscard]] std::size_t particle_count() const noexcept;
    [[nodiscard]] std::vector<ParticleLocation> particle_locations() const;
    [[nodiscard]] std::vector<SegmentPopulation> segment_populations() const;
    [[nodiscard]] const VascularGraph& graph() const noexcept;
    [[nodiscard]] const TransportObservationConfig& observation_config() const noexcept;
    [[nodiscard]] const std::vector<TrajectoryRecord>& trajectory_records() const noexcept;
    [[nodiscard]] const std::vector<MeasurementRecord>& measurement_records() const noexcept;
    [[nodiscard]] const std::vector<MeasurementCount>& measurement_counts() const noexcept;
    [[nodiscard]] const std::vector<PopulationSnapshot>& population_snapshots() const noexcept;
    [[nodiscard]] const std::vector<ExtractionResult>& extraction_results() const noexcept;
    [[nodiscard]] bool trajectories_truncated() const noexcept;
    [[nodiscard]] bool measurements_truncated() const noexcept;
    [[nodiscard]] bool aggregates_truncated() const noexcept;

  private:
    struct Particle final {
        std::uint64_t id{};
        std::size_t segment_index{};
        core::SimulationClock::Duration residence_time{};
    };

    void inject_until(core::SimulationClock::Duration cutoff);
    void extract_until(core::SimulationClock::Duration cutoff);
    void record_entry(core::SimulationClock::Duration time, const Particle& particle,
                      TrajectoryAction action);
    void record_trajectory(core::SimulationClock::Duration time, const Particle& particle,
                           TrajectoryAction action);
    void capture_population_snapshot(core::SimulationClock::Duration time);
    [[nodiscard]] std::size_t choose_successor(std::size_t segment_index,
                                               core::SimulationContext& context) const;
    void verify_population_invariant() const;

    VascularGraph graph_;
    std::vector<InjectionEvent> injections_;
    std::vector<ExtractionEvent> extractions_;
    TransportObservationConfig observation_config_;
    std::unordered_map<std::string, std::size_t> segment_indices_;
    std::vector<core::SimulationClock::Duration> transit_times_;
    std::vector<Particle> particles_;
    std::vector<std::vector<std::size_t>> measurement_sites_by_segment_;
    std::vector<TrajectoryRecord> trajectory_records_;
    std::vector<MeasurementRecord> measurement_records_;
    std::vector<MeasurementCount> measurement_counts_;
    std::vector<PopulationSnapshot> population_snapshots_;
    std::vector<ExtractionResult> extraction_results_;
    core::SimulationClock::Duration maximum_advance_{};
    std::size_t next_injection_{};
    std::size_t next_extraction_{};
    std::uint64_t next_particle_id_{1};
    std::uint64_t injected_particle_count_{};
    std::uint64_t extracted_particle_count_{};
    std::uint64_t transition_count_{};
    core::SimulationClock::Duration next_aggregate_time_{};
    bool trajectories_truncated_{};
    bool measurements_truncated_{};
    bool aggregates_truncated_{};
    bool initialized_{};
    bool finalized_{};
};

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP

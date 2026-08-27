// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP
#define MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <cstddef>
#include <cstdint>
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

class CompartmentTransport final : public core::SimulationComponent {
  public:
    CompartmentTransport(VascularGraph graph, std::vector<InjectionEvent> injections);

    [[nodiscard]] std::string_view name() const noexcept override;
    void initialize(core::SimulationContext& context) override;
    void advance(core::SimulationContext& context, core::SimulationClock::Duration delta) override;
    void finalize(core::SimulationContext& context) noexcept override;

    [[nodiscard]] core::SimulationClock::Duration maximum_advance() const noexcept;
    [[nodiscard]] std::uint64_t injected_particle_count() const noexcept;
    [[nodiscard]] std::uint64_t transition_count() const noexcept;
    [[nodiscard]] std::size_t particle_count() const noexcept;
    [[nodiscard]] std::vector<ParticleLocation> particle_locations() const;
    [[nodiscard]] std::vector<SegmentPopulation> segment_populations() const;

  private:
    struct Particle final {
        std::uint64_t id{};
        std::size_t segment_index{};
        core::SimulationClock::Duration residence_time{};
    };

    void inject_until(core::SimulationClock::Duration cutoff);
    [[nodiscard]] std::size_t choose_successor(std::size_t segment_index,
                                               core::SimulationContext& context) const;
    void verify_population_invariant() const;

    VascularGraph graph_;
    std::vector<InjectionEvent> injections_;
    std::unordered_map<std::string, std::size_t> segment_indices_;
    std::vector<core::SimulationClock::Duration> transit_times_;
    std::vector<Particle> particles_;
    core::SimulationClock::Duration maximum_advance_{};
    std::size_t next_injection_{};
    std::uint64_t next_particle_id_{1};
    std::uint64_t injected_particle_count_{};
    std::uint64_t transition_count_{};
    bool initialized_{};
    bool finalized_{};
};

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_COMPARTMENT_TRANSPORT_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::models::body::VascularGraph load_reference_graph() {
    return mehlissa::models::body::load_vascular_graph({
        root_path() / "examples" / "body-models" / "synthetic-branching-circuit.json",
        root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });
}

struct TransportRun final {
    std::vector<mehlissa::models::body::ParticleLocation> locations;
    std::vector<mehlissa::models::body::SegmentPopulation> populations;
    std::uint64_t transitions{};
    std::vector<mehlissa::core::RandomStreamState> random_streams;

    [[nodiscard]] bool operator==(const TransportRun&) const noexcept = default;
};

[[nodiscard]] TransportRun run_branching_transport(const std::uint64_t seed,
                                                   const int step_count = 7) {
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 64}});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{seed};
    host.add(std::move(transport));
    host.initialize();
    for (int step = 0; step < step_count; ++step) {
        host.advance(1s);
    }
    return {observer->particle_locations(), observer->segment_populations(),
            observer->transition_count(), host.context().random_stream_states()};
}

[[nodiscard]] std::uint64_t
population_sum(const std::vector<mehlissa::models::body::SegmentPopulation>& populations) {
    return std::accumulate(populations.begin(), populations.end(), std::uint64_t{},
                           [](const std::uint64_t sum, const auto& population) {
                               return sum + population.particle_count;
                           });
}

} // namespace

TEST_CASE("Scheduled particles are injected at the requested simulation time",
          "[body][transport][injection]") {
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(), std::vector<mehlissa::models::body::InjectionEvent>{
                                    {0ns, "artery-10", 3}, {2s, "organ-a", 2}});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{17};
    host.add(std::move(transport));
    host.initialize();

    REQUIRE(observer->particle_count() == 3);
    host.advance(1s);
    REQUIRE(observer->particle_count() == 3);
    host.advance(1s);
    REQUIRE(observer->particle_count() == 5);
    REQUIRE(observer->injected_particle_count() == 5);
    REQUIRE(population_sum(observer->segment_populations()) == 5);
}

TEST_CASE("A particle moves at most one graph edge per simulation advance",
          "[body][transport][single-move]") {
    auto graph = load_reference_graph();
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        std::move(graph), std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "organ-a", 1}});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{23};
    host.add(std::move(transport));
    host.initialize();

    host.advance(observer->maximum_advance());
    const auto after_one_step = observer->particle_locations();
    REQUIRE(after_one_step.front().segment_id == "vein-90");
    REQUIRE(observer->transition_count() == 1);
}

TEST_CASE("Transport conserves all injected particles across branches and merges",
          "[body][transport][conservation]") {
    const auto result = run_branching_transport(42);
    const auto branch_result = run_branching_transport(42, 4);

    REQUIRE(result.locations.size() == 64);
    REQUIRE(population_sum(result.populations) == 64);
    REQUIRE(result.transitions == 128);
    REQUIRE(result.random_streams == std::vector<mehlissa::core::RandomStreamState>{
                                         {"body.compartment-transport.transitions", 64}});
    REQUIRE(branch_result.populations[1].particle_count == 29);
    REQUIRE(branch_result.populations[2].particle_count == 35);
}

TEST_CASE("Named-stream branching is exactly reproducible for a fixed seed",
          "[body][transport][determinism]") {
    REQUIRE(run_branching_transport(1'234'567) == run_branching_transport(1'234'567));
    REQUIRE_FALSE(run_branching_transport(1'234'567).locations ==
                  run_branching_transport(7'654'321).locations);
}

TEST_CASE("Transport rejects invalid injections and unsafe advance intervals",
          "[body][transport][validation]") {
    REQUIRE_THROWS_WITH(
        mehlissa::models::body::CompartmentTransport(
            load_reference_graph(),
            std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "missing", 1}}),
        Catch::Matchers::ContainsSubstring("unknown segment"));

    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 1}});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{99};
    host.add(std::move(transport));
    host.initialize();

    REQUIRE_THROWS_WITH(host.advance(observer->maximum_advance() + 1ns),
                        Catch::Matchers::ContainsSubstring("shortest segment transit time"));
}

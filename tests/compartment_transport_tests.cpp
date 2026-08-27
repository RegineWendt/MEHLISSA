// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/transport_observation_report.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
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

struct TransportRunOptions final {
    std::uint64_t seed{};
    int step_count{7};
};

[[nodiscard]] TransportRun run_branching_transport(const TransportRunOptions options) {
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 64}});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{options.seed};
    host.add(std::move(transport));
    host.initialize();
    for (int step = 0; step < options.step_count; ++step) {
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
    const auto result = run_branching_transport({.seed = 42});
    const auto branch_result = run_branching_transport({.seed = 42, .step_count = 4});

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
    REQUIRE(run_branching_transport({.seed = 1'234'567}) ==
            run_branching_transport({.seed = 1'234'567}));
    REQUIRE_FALSE(run_branching_transport({.seed = 1'234'567}).locations ==
                  run_branching_transport({.seed = 7'654'321}).locations);
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

TEST_CASE("Scheduled extraction deterministically removes the lowest active particle IDs",
          "[body][transport][extraction]") {
    mehlissa::models::body::TransportObservationConfig observations;
    observations.trajectory_selection = mehlissa::models::body::TrajectorySelection::all;
    observations.maximum_trajectory_records = 10;
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 5}},
        std::vector<mehlissa::models::body::ExtractionEvent>{{1s, "artery-10", 2}}, observations);
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{31};
    host.add(std::move(transport));
    host.initialize();
    host.advance(1s);

    REQUIRE(observer->injected_particle_count() == 5);
    REQUIRE(observer->extracted_particle_count() == 2);
    REQUIRE(observer->particle_count() == 3);
    REQUIRE(observer->particle_locations().at(0).particle_id == 3);
    REQUIRE(observer->extraction_results() ==
            std::vector<mehlissa::models::body::ExtractionResult>{{1s, 1s, "artery-10", 2, 2}});
    REQUIRE(observer->trajectory_records().at(5).action ==
            mehlissa::models::body::TrajectoryAction::extracted);
    REQUIRE(observer->trajectory_records().at(5).particle_id == 1);
    REQUIRE(observer->trajectory_records().at(6).particle_id == 2);
}

TEST_CASE("An extraction without a requested count removes all particles at its site",
          "[body][transport][extraction]") {
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 5}},
        std::vector<mehlissa::models::body::ExtractionEvent>{{0ns, "artery-10", std::nullopt}},
        mehlissa::models::body::TransportObservationConfig{});
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{31};
    host.add(std::move(transport));
    host.initialize();

    REQUIRE(observer->particle_count() == 0);
    REQUIRE(observer->extracted_particle_count() == 5);
    REQUIRE(observer->extraction_results().front().extracted_particle_count == 5);
}

TEST_CASE("Measurement and trajectory detail is bounded while aggregate counts remain exact",
          "[body][transport][observation]") {
    mehlissa::models::body::TransportObservationConfig observations;
    observations.measurement_sites = {
        {"organ-a-sample", "organ-a", mehlissa::models::body::MeasurementSiteKind::sample},
        {"wrist-gateway", "vein-90", mehlissa::models::body::MeasurementSiteKind::gateway},
    };
    observations.trajectory_selection = mehlissa::models::body::TrajectorySelection::first_n;
    observations.trajectory_particle_limit = 2;
    observations.maximum_trajectory_records = 5;
    observations.maximum_measurement_records = 3;
    observations.aggregate_interval = 1s;
    observations.maximum_aggregate_records = 2;
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 64}},
        std::vector<mehlissa::models::body::ExtractionEvent>{}, observations);
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{42};
    host.add(std::move(transport));
    host.initialize();
    for (int step = 0; step < 7; ++step) {
        host.advance(1s);
    }

    REQUIRE(observer->measurement_counts().at(0).particle_count == 29);
    REQUIRE(observer->measurement_counts().at(1).particle_count == 64);
    REQUIRE(observer->measurement_records().size() == 3);
    REQUIRE(observer->measurements_truncated());
    REQUIRE(observer->trajectory_records().size() == 5);
    REQUIRE(observer->trajectories_truncated());
    REQUIRE(observer->population_snapshots().size() == 2);
    REQUIRE(observer->aggregates_truncated());
    REQUIRE(population_sum(observer->population_snapshots().back().populations) == 64);
}

TEST_CASE("Transport observations are written as schema-valid structured output",
          "[body][transport][observation][schema]") {
    mehlissa::models::body::TransportObservationConfig observations;
    observations.measurement_sites = {
        {"arterial-sample", "artery-10", mehlissa::models::body::MeasurementSiteKind::sample}};
    observations.maximum_measurement_records = 1;
    observations.aggregate_interval = 1s;
    observations.maximum_aggregate_records = 2;
    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_reference_graph(),
        std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 2}},
        std::vector<mehlissa::models::body::ExtractionEvent>{}, observations);
    auto* observer = transport.get();
    mehlissa::core::ComponentHost host{71};
    host.add(std::move(transport));
    host.initialize();
    host.advance(1s);

    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto output_path = std::filesystem::temp_directory_path() /
                             ("mehlissa-transport-observation-" + std::to_string(unique) + ".json");
    mehlissa::models::body::write_transport_observation_report(
        *observer, {output_path, root_path() / "data" / "schemas" / "transport-observation-report" /
                                     "1.0.0.schema.json"});

    std::ifstream input{output_path, std::ios::binary};
    REQUIRE(input);
    const std::string contents{std::istreambuf_iterator<char>{input}, {}};
    REQUIRE(contents.find("\"schema_version\": \"1.0.0\"") != std::string::npos);
    REQUIRE(contents.find("\"arterial-sample\"") != std::string::npos);
    input.close();
    std::filesystem::remove(output_path);
}

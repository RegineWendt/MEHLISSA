// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>
#include <mehlissa/models/cosimulation/body_organ_coupler.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] mehlissa::models::body::VascularGraph load_graph() {
    const std::filesystem::path root{MEHLISSA_TEST_ROOT};
    return mehlissa::models::body::load_vascular_graph({
        root / "examples" / "body-models" / "synthetic-branching-circuit.json",
        root / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });
}

} // namespace

TEST_CASE("An entity completes a conservative body lung body round trip",
          "[m3][cosimulation][round-trip]") {
    const auto definition_file = GENERATE(std::string{"lung-compartment-contract-v1.json"},
                                          std::string{"lung-regional-contract-v1.json"});
    const auto step = GENERATE(500ms, 1s);
    const std::filesystem::path root{MEHLISSA_TEST_ROOT};
    const auto definition = mehlissa::models::organ::load_lung_model_definition(
        {root / "examples" / "organ-models" / definition_file,
         root / "data" / "schemas" / "lung-model-definition" / "1.0.0.schema.json"});
    auto body = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_graph(), std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 1}});
    auto lung = mehlissa::models::organ::make_lung_model(definition.model);
    auto* body_observer = body.get();
    auto* lung_observer = lung.get();

    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    host.add(std::move(body));
    host.add(std::move(lung));
    host.initialize();

    mehlissa::models::cosimulation::BodyOrganCoupler coupler{
        *body_observer,
        *lung_observer,
        {"artery-10", "pulmonary-arterial-departure", "pulmonary-arterial-entry",
         "pulmonary-venous-exit", "pulmonary-venous-return", "vein-90"}};

    coupler.send_to_organ(1, 0ns);
    CHECK(body_observer->particle_count() == 0);
    CHECK(body_observer->outside_body_particle_count() == 1);
    CHECK(coupler.in_flight_count() == 1);

    std::size_t returned{};
    for (auto elapsed = 0ns; elapsed < 2s; elapsed += step) {
        host.advance(step);
        returned += coupler.receive_from_organ(host.context().clock().now());
    }
    CHECK(returned == 1);

    REQUIRE(body_observer->particle_locations().size() == 1);
    CHECK(body_observer->particle_locations().front().particle_id == 1);
    CHECK(body_observer->particle_locations().front().segment_id == "vein-90");
    CHECK(body_observer->outside_body_particle_count() == 0);
    CHECK(coupler.in_flight_count() == 0);
    CHECK(coupler.pending_return_count() == 0);
    CHECK(coupler.completed_round_trip_count() == 1);
}

TEST_CASE("The literature pulmonary 0D card completes the same body lung body contract",
          "[m3][cosimulation][round-trip][pulmonary-0d]") {
    const auto step = GENERATE(100ms, 200ms);
    const std::filesystem::path root{MEHLISSA_TEST_ROOT};
    auto definition = mehlissa::models::organ::load_lung_model_definition(
        {root / "data" / "lung-models" / "healthy-adult-rest-supine-0d-v1.json",
         root / "data" / "schemas" / "lung-model-definition" / "1.1.0.schema.json"});
    definition.model.return_target_model_id = "synthetic-branching-circuit";
    auto body = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_graph(), std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 1}});
    auto lung = mehlissa::models::organ::make_lung_model(definition.model);
    auto* body_observer = body.get();
    auto* lung_observer = lung.get();

    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    host.add(std::move(body));
    host.add(std::move(lung));
    host.initialize();

    mehlissa::models::cosimulation::BodyOrganCoupler coupler{
        *body_observer,
        *lung_observer,
        {"artery-10", "pulmonary-arterial-departure", "pulmonary-arterial-entry",
         "pulmonary-venous-exit", "pulmonary-venous-return", "vein-90"}};
    coupler.send_to_organ(1, 0ns);

    std::size_t returned{};
    for (auto elapsed = 0ns; elapsed < 6400ms; elapsed += step) {
        host.advance(step);
        returned += coupler.receive_from_organ(host.context().clock().now());
    }

    CHECK(returned == 1);
    REQUIRE(body_observer->particle_locations().size() == 1);
    CHECK(body_observer->particle_locations().front().particle_id == 1);
    CHECK(body_observer->particle_locations().front().segment_id == "vein-90");
    CHECK(body_observer->outside_body_particle_count() == 0);
    CHECK(coupler.completed_round_trip_count() == 1);
}

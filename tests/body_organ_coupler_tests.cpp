// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>
#include <mehlissa/models/cosimulation/body_organ_coupler.hpp>
#include <mehlissa/models/organ/lung_compartment.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
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
    auto body = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        load_graph(), std::vector<mehlissa::models::body::InjectionEvent>{{0ns, "artery-10", 1}});
    auto lung = std::make_unique<mehlissa::models::organ::LungCompartment>(
        mehlissa::models::organ::LungCompartmentConfig{
            "organ.lung.compartment", "lung.compartment.v1", "pulmonary-arterial-entry",
            "pulmonary-venous-exit", "synthetic-branching-circuit", "pulmonary-venous-return", 2s});
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
    CHECK(lung_observer->resident_count() == 1);
    CHECK(coupler.in_flight_count() == 1);

    host.advance(1s);
    CHECK(coupler.receive_from_organ(host.context().clock().now()) == 0);
    host.advance(1s);
    CHECK(coupler.receive_from_organ(host.context().clock().now()) == 1);

    REQUIRE(body_observer->particle_locations().size() == 1);
    CHECK(body_observer->particle_locations().front().particle_id == 1);
    CHECK(body_observer->particle_locations().front().segment_id == "vein-90");
    CHECK(body_observer->outside_body_particle_count() == 0);
    CHECK(lung_observer->resident_count() == 0);
    CHECK(coupler.in_flight_count() == 0);
    CHECK(coupler.pending_return_count() == 0);
    CHECK(coupler.completed_round_trip_count() == 1);
}

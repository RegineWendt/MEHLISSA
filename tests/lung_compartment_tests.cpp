// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>
#include <mehlissa/models/organ/lung_compartment.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] mehlissa::models::organ::LungCompartmentConfig lung_config() {
    return {
        "organ.lung.compartment",
        "lung.compartment.v1",
        "pulmonary-arterial-entry",
        "pulmonary-venous-exit",
        "body.bvs95",
        "pulmonary-venous-return",
        2s,
    };
}

[[nodiscard]] mehlissa::models::coupling::EntityTransfer
body_to_lung(const std::uint64_t entity_id, const std::chrono::nanoseconds emitted_at = 0ns) {
    return {
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        entity_id,
        "nanodevice",
        "body.bvs95",
        "pulmonary-arterial-departure",
        "lung.compartment.v1",
        "pulmonary-arterial-entry",
        emitted_at,
    };
}

} // namespace

TEST_CASE("A lung compartment conserves entity identity across its two ports",
          "[m3][organ][coupling]") {
    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    auto component = std::make_unique<mehlissa::models::organ::LungCompartment>(lung_config());
    auto* lung = component.get();
    host.add(std::move(component));
    host.initialize();

    CHECK(lung->model_id() == "lung.compartment.v1");
    CHECK(lung->accepts_entity_at("pulmonary-arterial-entry"));
    CHECK_FALSE(lung->accepts_entity_at("pulmonary-venous-exit"));
    CHECK(lung->emits_entity_at("pulmonary-venous-exit"));
    CHECK_FALSE(lung->emits_entity_at("pulmonary-arterial-entry"));

    lung->accept_entity(body_to_lung(42));
    host.advance(1s);
    REQUIRE(lung->resident_count() == 1);
    REQUIRE(lung->outbound_count() == 0);

    host.advance(1s);
    REQUIRE(lung->resident_count() == 0);
    REQUIRE(lung->outbound_count() == 1);

    const auto outbound = lung->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().entity_id == 42);
    CHECK(outbound.front().entity_type == "nanodevice");
    CHECK(outbound.front().source_model_id == "lung.compartment.v1");
    CHECK(outbound.front().source_port_id == "pulmonary-venous-exit");
    CHECK(outbound.front().target_model_id == "body.bvs95");
    CHECK(outbound.front().target_port_id == "pulmonary-venous-return");
    CHECK(outbound.front().emitted_at == 2s);
    CHECK(lung->accepted_count() == 1);
    CHECK(lung->completed_count() == 1);
}

TEST_CASE("A lung compartment rejects invalid routes, times, and duplicate identities",
          "[m3][organ][coupling]") {
    mehlissa::core::ComponentHost host{std::uint64_t{1}};
    auto component = std::make_unique<mehlissa::models::organ::LungCompartment>(lung_config());
    auto* lung = component.get();

    REQUIRE_THROWS_AS(lung->accept_entity(body_to_lung(1)), mehlissa::core::MehlissaError);
    host.add(std::move(component));
    host.initialize();

    auto wrong_port = body_to_lung(1);
    wrong_port.target_port_id = "alveolar-entry";
    REQUIRE_THROWS_AS(lung->accept_entity(wrong_port), mehlissa::core::MehlissaError);
    REQUIRE_THROWS_AS(lung->accept_entity(body_to_lung(2, 1s)), mehlissa::core::MehlissaError);

    lung->accept_entity(body_to_lung(3));
    REQUIRE_THROWS_AS(lung->accept_entity(body_to_lung(3)), mehlissa::core::MehlissaError);
    REQUIRE(lung->resident_count() == 1);
}

TEST_CASE("The fixed-transit lung result is reproducible across compatible step sizes",
          "[m3][organ][determinism]") {
    const auto run = [](const std::chrono::nanoseconds step, const int steps) {
        mehlissa::core::ComponentHost host{std::uint64_t{17}};
        auto component = std::make_unique<mehlissa::models::organ::LungCompartment>(lung_config());
        auto* lung = component.get();
        host.add(std::move(component));
        host.initialize();
        lung->accept_entity(body_to_lung(99));
        for (int index = 0; index < steps; ++index) {
            host.advance(step);
        }
        return lung->take_outbound_entities();
    };

    const auto half_second = run(500ms, 4);
    const auto one_second = run(1s, 2);
    REQUIRE(half_second == one_second);
    REQUIRE(half_second.size() == 1);
    CHECK(half_second.front().emitted_at == 2s);
}

TEST_CASE("The entity-transfer contract rejects incomplete exchange objects", "[m3][coupling]") {
    auto transfer = body_to_lung(0);
    REQUIRE_THROWS_AS(mehlissa::models::coupling::validate_entity_transfer(transfer),
                      mehlissa::core::MehlissaError);

    transfer = body_to_lung(7);
    transfer.contract_version = "2.0.0";
    REQUIRE_THROWS_AS(mehlissa::models::coupling::validate_entity_transfer(transfer),
                      mehlissa::core::MehlissaError);
}

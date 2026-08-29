// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_recruitment_profile.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numbers>
#include <string>
#include <utility>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::capillary::CapillaryBed;
using mehlissa::models::capillary::CapillaryBedConfig;
using mehlissa::models::capillary::CapillaryBoundaryCondition;
using mehlissa::models::capillary::CapillaryRecruitmentProfile;
using mehlissa::models::capillary::CapillaryRegion;
using mehlissa::models::capillary::CapillaryRegionKind;

constexpr double synthetic_continuity_flow_m3_s = std::numbers::pi * 1.0e-13;

[[nodiscard]] CapillaryBedConfig capillary_config() {
    return {
        "capillary.synthetic",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        "venule-exit",
        "organ.synthetic",
        "capillary-return",
        8,
        4,
        mehlissa::core::cubic_meters_per_second(synthetic_continuity_flow_m3_s),
        {
            CapillaryRegion{"feeding-arteriole", CapillaryRegionKind::arteriole,
                            mehlissa::core::meters(0.0008), mehlissa::core::meters(0.00001), 1},
            CapillaryRegion{"exchange-bed", CapillaryRegionKind::capillary,
                            mehlissa::core::meters(0.0006), mehlissa::core::meters(0.00001), 4},
            CapillaryRegion{"draining-venule", CapillaryRegionKind::venule,
                            mehlissa::core::meters(0.0008), mehlissa::core::meters(0.00001), 1},
        },
    };
}

[[nodiscard]] CapillaryRecruitmentProfile
recruitment_profile(const CapillaryBoundaryCondition boundary_condition,
                    const std::chrono::nanoseconds activity_time = 1s) {
    return {
        "1.0.0",
        "synthetic-recruitment",
        "1.0.0",
        "Synthetic recruitment",
        "capillary.synthetic.reference.v2",
        boundary_condition,
        {{"always-perfused", 2}, {"rest-reserve", 2}, {"activity-reserve", 4}},
        {{"rest", 0s, {"always-perfused", "rest-reserve"}},
         {"activity", activity_time, {"always-perfused", "rest-reserve", "activity-reserve"}}},
        {"software contract tests", "synthetic activity", "software_test_surrogate",
         "Synthetic grouped recruitment verification."},
        {{"m4-contract", "MEHLISSA M4.4 contract", "docs/m4", "CC-BY-4.0",
          "software verification"}},
        {"Synthetic values are not physiological evidence."},
    };
}

[[nodiscard]] mehlissa::models::coupling::EntityTransfer entity_transfer() {
    return {
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        42,
        "nanodevice",
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        0s,
    };
}

[[nodiscard]] std::chrono::nanoseconds
complete_dynamic_transit(const std::chrono::nanoseconds step) {
    mehlissa::core::ComponentHost host{std::uint64_t{41}};
    auto component = std::make_unique<CapillaryBed>(
        capillary_config(),
        recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow, 400ms));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer());

    while (capillary->resident_entity_count() != 0) {
        host.advance(step);
    }
    const auto outbound = capillary->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    return outbound.front().emitted_at;
}

} // namespace

TEST_CASE("A strict capillary-recruitment profile loads with explicit evidence",
          "[m4][capillary][recruitment][schema]") {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    auto profile = mehlissa::models::capillary::load_capillary_recruitment_profile({
        root / "examples/capillary-models/synthetic-recruitment-fixed-flow-v1.json",
        root / "data/schemas/capillary-recruitment-profile/1.0.0.schema.json",
    });

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.compatible_model_id == "capillary.synthetic.reference.v2");
    CHECK(profile.boundary_condition == CapillaryBoundaryCondition::fixed_total_flow);
    CHECK(profile.sphincter_groups.size() == 3);
    CHECK(profile.states.size() == 3);
    CHECK(profile.states[1].id == "activity");
    CHECK(profile.states[1].effective_at == 1s);
    CHECK(profile.sources.size() == 2);
    CHECK(profile.limitations.size() == 4);

    mehlissa::core::ComponentHost host{std::uint64_t{40}};
    auto component = std::make_unique<CapillaryBed>(capillary_config(), std::move(profile));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    host.advance(1s);
    CHECK(capillary->recruitment_state_id() == "activity");
    CHECK(capillary->perfused_path_count() == 8);
    host.advance(1s);
    CHECK(capillary->recruitment_state_id() == "recovery");
    CHECK(capillary->perfused_path_count() == 4);
}

TEST_CASE("Fixed total flow recruitment redistributes velocity across open paths",
          "[m4][capillary][recruitment][continuity]") {
    mehlissa::core::ComponentHost host{std::uint64_t{42}};
    auto component = std::make_unique<CapillaryBed>(
        capillary_config(), recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    CHECK(capillary->has_recruitment_profile());
    CHECK(capillary->recruitment_state_id() == "rest");
    CHECK(capillary->perfused_path_count() == 4);
    CHECK(capillary->open_sphincter_group_count() == 2);
    CHECK(capillary->boundary_condition() == CapillaryBoundaryCondition::fixed_total_flow);

    host.advance(1s);

    CHECK(capillary->recruitment_state_id() == "activity");
    CHECK(capillary->perfused_path_count() == 8);
    CHECK(capillary->open_sphincter_group_count() == 3);
    CHECK(mehlissa::core::in_cubic_meters_per_second(capillary->volume_flow_rate()) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
    const auto& metrics = capillary->region_metrics(CapillaryRegionKind::capillary);
    CHECK(mehlissa::core::in_meters_per_second(metrics.mean_velocity) == Catch::Approx(0.0005));
    CHECK(metrics.transit_time == 1200ms);
}

TEST_CASE("Fixed pressure-drop recruitment uses an explicit equal-path conductance surrogate",
          "[m4][capillary][recruitment][boundary]") {
    mehlissa::core::ComponentHost host{std::uint64_t{43}};
    auto component = std::make_unique<CapillaryBed>(
        capillary_config(), recruitment_profile(CapillaryBoundaryCondition::fixed_pressure_drop));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    host.advance(1s);

    CHECK(capillary->perfused_path_count() == 8);
    CHECK(mehlissa::core::in_cubic_meters_per_second(capillary->volume_flow_rate()) ==
          Catch::Approx(2.0 * synthetic_continuity_flow_m3_s));
    const auto& capillary_metrics = capillary->region_metrics(CapillaryRegionKind::capillary);
    CHECK(mehlissa::core::in_meters_per_second(capillary_metrics.mean_velocity) ==
          Catch::Approx(0.001));
    CHECK(capillary_metrics.transit_time == 600ms);
    const auto& arteriole_metrics = capillary->region_metrics(CapillaryRegionKind::arteriole);
    CHECK(mehlissa::core::in_meters_per_second(arteriole_metrics.mean_velocity) ==
          Catch::Approx(0.008));
    CHECK(arteriole_metrics.transit_time == 100ms);
}

TEST_CASE("Recruitment preserves in-flight distance across synchronization step sizes",
          "[m4][capillary][recruitment][determinism]") {
    CHECK(complete_dynamic_transit(100ms) == 1400ms);
    CHECK(complete_dynamic_transit(350ms) == 1400ms);
}

TEST_CASE("Capillary recruitment rejects incompatible partitions and schedules",
          "[m4][capillary][recruitment][validation]") {
    auto wrong_target = recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow);
    wrong_target.compatible_model_id = "capillary.other";
    CHECK_THROWS_AS(CapillaryBed(capillary_config(), std::move(wrong_target)),
                    mehlissa::core::MehlissaError);

    auto incomplete_partition = recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow);
    incomplete_partition.sphincter_groups.back().path_count = 3;
    CHECK_THROWS_AS(CapillaryBed(capillary_config(), std::move(incomplete_partition)),
                    mehlissa::core::MehlissaError);

    auto initial_mismatch = recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow);
    initial_mismatch.states.front().open_sphincter_group_ids = {"always-perfused"};
    CHECK_THROWS_AS(CapillaryBed(capillary_config(), std::move(initial_mismatch)),
                    mehlissa::core::MehlissaError);

    auto non_monotonic = recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow);
    non_monotonic.states.back().effective_at = 0s;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_recruitment_profile(non_monotonic),
        mehlissa::core::MehlissaError);

    auto unknown_group = recruitment_profile(CapillaryBoundaryCondition::fixed_total_flow);
    unknown_group.states.back().open_sphincter_group_ids.push_back("unknown");
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_recruitment_profile(unknown_group),
        mehlissa::core::MehlissaError);
}

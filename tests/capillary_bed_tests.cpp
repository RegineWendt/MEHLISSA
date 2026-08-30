// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>
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
using mehlissa::models::capillary::CapillaryRegion;
using mehlissa::models::capillary::CapillaryRegionKind;
using mehlissa::models::coupling::PopulationTransfer;
using mehlissa::models::coupling::SubstanceAmountTransfer;
using mehlissa::models::coupling::TransferHeader;
using mehlissa::models::coupling::VolumeFlowTransfer;

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

[[nodiscard]] mehlissa::models::coupling::EntityTransfer
organ_to_capillary(const std::uint64_t entity_id, const std::chrono::nanoseconds emitted_at = 0ns) {
    return {
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        entity_id,
        "nanodevice",
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        emitted_at,
    };
}

[[nodiscard]] TransferHeader transfer_header(const std::string& transfer_id) {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        transfer_id,
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        0s,
    };
}

} // namespace

TEST_CASE("A capillary bed exposes explicit serial microvascular regions",
          "[m4][capillary][coupling]") {
    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    auto component = std::make_unique<CapillaryBed>(capillary_config());
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    CHECK(capillary->region_count() == 3);
    CHECK(capillary->total_parallel_path_count() == 8);
    CHECK(capillary->perfused_path_count() == 4);
    CHECK(mehlissa::core::in_cubic_meters_per_second(capillary->volume_flow_rate()) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
    CHECK(capillary->accepts_entity_at("arteriole-entry"));
    CHECK(capillary->emits_entity_at("venule-exit"));

    capillary->accept_entity(organ_to_capillary(42));
    CHECK(capillary->resident_entity_count_in(CapillaryRegionKind::arteriole) == 1);

    host.advance(200ms);
    CHECK(capillary->resident_entity_count_in(CapillaryRegionKind::capillary) == 1);
    host.advance(600ms);
    CHECK(capillary->resident_entity_count_in(CapillaryRegionKind::venule) == 1);
    host.advance(200ms);

    CHECK(capillary->resident_entity_count() == 0);
    const auto outbound = capillary->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().entity_id == 42);
    CHECK(outbound.front().entity_type == "nanodevice");
    CHECK(outbound.front().source_model_id == "capillary.synthetic.reference.v2");
    CHECK(outbound.front().source_port_id == "venule-exit");
    CHECK(outbound.front().target_model_id == "organ.synthetic");
    CHECK(outbound.front().target_port_id == "capillary-return");
    CHECK(outbound.front().emitted_at == 1s);
}

TEST_CASE("Capillary geometry derives velocity and transit from volume-flow continuity",
          "[m4][capillary][geometry][continuity]") {
    const CapillaryBed capillary{capillary_config()};
    const auto& arteriole = capillary.region_metrics(CapillaryRegionKind::arteriole);
    const auto& exchange_bed = capillary.region_metrics(CapillaryRegionKind::capillary);
    const auto& venule = capillary.region_metrics(CapillaryRegionKind::venule);

    const auto expected_single_area = std::numbers::pi * 0.00001 * 0.00001 / 4.0;
    CHECK(mehlissa::core::in_square_meters(arteriole.single_vessel_cross_section) ==
          Catch::Approx(expected_single_area));
    CHECK(mehlissa::core::in_square_meters(exchange_bed.total_cross_section) ==
          Catch::Approx(4.0 * expected_single_area));
    CHECK(mehlissa::core::in_cubic_meters(exchange_bed.blood_volume) ==
          Catch::Approx(4.0 * expected_single_area * 0.0006));
    CHECK(mehlissa::core::in_meters_per_second(arteriole.mean_velocity) == Catch::Approx(0.004));
    CHECK(mehlissa::core::in_meters_per_second(exchange_bed.mean_velocity) == Catch::Approx(0.001));
    CHECK(mehlissa::core::in_meters_per_second(venule.mean_velocity) == Catch::Approx(0.004));
    CHECK(arteriole.transit_time == 200ms);
    CHECK(exchange_bed.transit_time == 600ms);
    CHECK(venule.transit_time == 200ms);

    const auto capillary_flow = exchange_bed.total_cross_section * exchange_bed.mean_velocity;
    CHECK(mehlissa::core::in_cubic_meters_per_second(capillary_flow) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
}

TEST_CASE("The capillary baseline conserves population substance and continuity flow",
          "[m4][capillary][conservation]") {
    mehlissa::core::ComponentHost host{std::uint64_t{17}};
    auto component = std::make_unique<CapillaryBed>(capillary_config());
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    capillary->accept_conserved_transfer(
        PopulationTransfer{transfer_header("population-1"), "nanodevice", 10'000});
    capillary->accept_conserved_transfer(SubstanceAmountTransfer{
        transfer_header("substance-1"), "oxygen", mehlissa::core::millimoles(2.5)});
    capillary->accept_conserved_transfer(VolumeFlowTransfer{
        transfer_header("flow-1"),
        mehlissa::core::cubic_meters_per_second(synthetic_continuity_flow_m3_s), 1s});

    host.advance(500ms);
    CHECK(capillary->resident_conserved_transfer_count() == 3);
    CHECK(capillary->take_outbound_conserved_transfers().empty());
    host.advance(500ms);

    const auto returned = capillary->take_outbound_conserved_transfers();
    REQUIRE(returned.size() == 3);
    CHECK(capillary->resident_conserved_transfer_count() == 0);
    for (const auto& transfer : returned) {
        const auto& header = mehlissa::models::coupling::transfer_header(transfer);
        CHECK(header.source_model_id == "capillary.synthetic.reference.v2");
        CHECK(header.source_port_id == "venule-exit");
        CHECK(header.target_model_id == "organ.synthetic");
        CHECK(header.target_port_id == "capillary-return");
        CHECK(header.emitted_at == 1s);
    }

    const auto& population = std::get<PopulationTransfer>(returned[0]);
    CHECK(population.population_type == "nanodevice");
    CHECK(population.count == 10'000);
    const auto& substance = std::get<SubstanceAmountTransfer>(returned[1]);
    CHECK(substance.substance_id == "oxygen");
    CHECK(mehlissa::core::in_moles(substance.amount) == 0.0025);
    const auto& flow = std::get<VolumeFlowTransfer>(returned[2]);
    CHECK(mehlissa::core::in_cubic_meters_per_second(flow.flow_rate) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
    CHECK(mehlissa::core::in_cubic_meters(mehlissa::models::coupling::integrated_volume(flow)) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
}

TEST_CASE("A capillary bed rejects invalid recruitment topology and ownership",
          "[m4][capillary][validation]") {
    auto invalid_paths = capillary_config();
    invalid_paths.perfused_path_count = 9;
    CHECK_THROWS_AS(CapillaryBed{std::move(invalid_paths)}, mehlissa::core::MehlissaError);

    auto invalid_order = capillary_config();
    invalid_order.regions[1].kind = CapillaryRegionKind::venule;
    CHECK_THROWS_AS(CapillaryBed{std::move(invalid_order)}, mehlissa::core::MehlissaError);

    auto invalid_geometry = capillary_config();
    invalid_geometry.regions[0].diameter = mehlissa::core::meters(0.0);
    CHECK_THROWS_AS(CapillaryBed{std::move(invalid_geometry)}, mehlissa::core::MehlissaError);

    auto inconsistent_recruitment = capillary_config();
    inconsistent_recruitment.regions[1].parallel_vessel_count = 3;
    CHECK_THROWS_AS(CapillaryBed{std::move(inconsistent_recruitment)},
                    mehlissa::core::MehlissaError);

    auto invalid_flow = capillary_config();
    invalid_flow.volume_flow_rate = mehlissa::core::cubic_meters_per_second(0.0);
    CHECK_THROWS_AS(CapillaryBed{std::move(invalid_flow)}, mehlissa::core::MehlissaError);

    mehlissa::core::ComponentHost host{std::uint64_t{1}};
    auto component = std::make_unique<CapillaryBed>(capillary_config());
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    auto wrong_route = organ_to_capillary(1);
    wrong_route.target_port_id = "capillary-entry";
    CHECK_THROWS_AS(capillary->accept_entity(std::move(wrong_route)),
                    mehlissa::core::MehlissaError);
    CHECK_THROWS_AS(capillary->accept_entity(organ_to_capillary(2, 1s)),
                    mehlissa::core::MehlissaError);
    capillary->accept_entity(organ_to_capillary(3));
    CHECK_THROWS_AS(capillary->accept_entity(organ_to_capillary(3)), mehlissa::core::MehlissaError);

    CHECK_THROWS_AS(
        capillary->accept_conserved_transfer(VolumeFlowTransfer{
            transfer_header("wrong-flow"),
            mehlissa::core::cubic_meters_per_second(2.0 * synthetic_continuity_flow_m3_s), 1s}),
        mehlissa::core::MehlissaError);
}

TEST_CASE("The versioned synthetic capillary definition loads into executable state",
          "[m4][capillary][schema]") {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    const auto definition = mehlissa::models::capillary::load_capillary_bed_definition({
        root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
        root / "data/schemas/capillary-bed-definition/2.0.0.schema.json",
    });

    CHECK(definition.schema_version == "2.0.0");
    CHECK(definition.definition_id == "synthetic-arteriole-capillary-venule-v2");
    CHECK(definition.model.regions.size() == 3);
    CHECK(definition.model.regions[0].kind == CapillaryRegionKind::arteriole);
    CHECK(definition.model.regions[1].kind == CapillaryRegionKind::capillary);
    CHECK(definition.model.regions[2].kind == CapillaryRegionKind::venule);
    CHECK(definition.model.perfused_path_count == 4);
    CHECK(mehlissa::core::in_meters(definition.model.regions[1].length) == 0.0006);
    CHECK(definition.model.regions[1].parallel_vessel_count == 4);
    CHECK(definition.validity.evidence_class == "software_test_surrogate");
    CHECK(definition.sources.size() == 2);
    CHECK(definition.limitations.size() == 4);

    const CapillaryBed executable{definition.model};
    CHECK(executable.model_id() == "capillary.synthetic.reference.v2");
}

TEST_CASE("The pulmonary capillary card closes evidence volume geometry and transit",
          "[m4][capillary][schema][evidence]") {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    const auto definition = mehlissa::models::capillary::load_capillary_bed_definition({
        root / "examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json",
        root / "data/schemas/capillary-bed-definition/3.0.0.schema.json",
    });

    CHECK(definition.schema_version == "3.0.0");
    CHECK(definition.definition_id == "pulmonary-healthy-adult-rest-supine-v1");
    CHECK(definition.validity.evidence_class == "literature_parameterized");
    REQUIRE(definition.qualification.has_value());
    const auto& qualification = *definition.qualification;
    CHECK(qualification.geometry_semantics == "equivalent_parallel_tubes");
    CHECK(qualification.functional_blood_volume.value_si == Catch::Approx(85.9e-6));
    CHECK(qualification.morphometric_lumen_volume.value_si == Catch::Approx(196.0e-6));
    CHECK(qualification.morphometric_surface_area.value_si == Catch::Approx(130.0));
    CHECK(qualification.equivalent_diameter.value_si == Catch::Approx(6.30e-6));
    CHECK(qualification.reference_transit_time.value_si == Catch::Approx(0.859));
    CHECK(qualification.reference_transit_time.role == "derived");
    REQUIRE(qualification.functional_blood_volume.uncertainty.standard_deviation_si.has_value());
    CHECK(*qualification.functional_blood_volume.uncertainty.standard_deviation_si ==
          Catch::Approx(14.4243717367517e-6));

    const CapillaryBed executable{definition.model};
    const auto& capillary = executable.region_metrics(CapillaryRegionKind::capillary);
    CHECK(mehlissa::core::in_cubic_meters(capillary.blood_volume) == Catch::Approx(85.9e-6));
    CHECK(capillary.transit_time == 859ms);
    CHECK(executable.total_parallel_path_count() == 9'126'891'735ULL);
    CHECK(executable.perfused_path_count() == 4'000'000'000ULL);

    const auto& arteriole = executable.region_metrics(CapillaryRegionKind::arteriole);
    const auto& venule = executable.region_metrics(CapillaryRegionKind::venule);
    CHECK(mehlissa::core::in_cubic_meters(arteriole.blood_volume) == Catch::Approx(5.0e-6));
    CHECK(mehlissa::core::in_cubic_meters(venule.blood_volume) == Catch::Approx(5.0e-6));
    CHECK(arteriole.transit_time == 50ms);
    CHECK(venule.transit_time == 50ms);
}

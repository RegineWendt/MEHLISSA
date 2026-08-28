// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>
#include <mehlissa/models/organ/pulmonary_parallel_beds.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] mehlissa::models::organ::LungModelDefinition lobar_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-lobar-parallel-0d-v7.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.6.0.schema.json",
    });
}

} // namespace

TEST_CASE("The v7 lung definition creates five anatomical parallel beds",
          "[m3][organ][pulmonary-0d][parallel][anatomy]") {
    const auto definition = lobar_definition();
    REQUIRE(definition.model.zero_dimensional_parameters.has_value());
    const auto parameters = definition.model.zero_dimensional_parameters.value_or(
        mehlissa::models::organ::PulmonaryZeroDimensionalParameters{});
    REQUIRE(parameters.parallel_beds.size() == 5);
    CHECK(parameters.parallel_beds[0].id == "right-upper-lobe");
    CHECK(parameters.parallel_beds[1].id == "right-middle-lobe");
    CHECK(parameters.parallel_beds[2].id == "right-lower-lobe");
    CHECK(parameters.parallel_beds[3].id == "left-upper-lobe");
    CHECK(parameters.parallel_beds[4].id == "left-lower-lobe");

    auto model = mehlissa::models::organ::make_lung_model(definition.model);
    auto* parallel =
        dynamic_cast<mehlissa::models::organ::PulmonaryParallelBedsModel*>(model.get());
    REQUIRE(parallel != nullptr);
    const auto state = parallel->state();
    REQUIRE(state.beds.size() == 5);

    auto fraction_sum = 0.0;
    auto flow_sum = mehlissa::core::cubic_meters_per_second(0.0);
    auto compliance_sum = mehlissa::core::cubic_meters_per_pascal(0.0);
    auto conductance_sum = 0.0;
    auto blood_volume_sum = mehlissa::core::cubic_meters(0.0);
    for (const auto& bed : state.beds) {
        fraction_sum += bed.perfusion_fraction.si_value();
        flow_sum += bed.flow;
        compliance_sum += bed.compliance;
        conductance_sum += 1.0 / mehlissa::core::in_pascal_seconds_per_cubic_meter(bed.resistance);
        blood_volume_sum += bed.blood_volume;
    }
    CHECK(fraction_sum == Catch::Approx(1.0).epsilon(1.0e-12));
    CHECK(mehlissa::core::in_cubic_meters_per_second(flow_sum) ==
          Catch::Approx(
              mehlissa::core::in_cubic_meters_per_second(state.aggregate.pulmonary_outflow)));
    CHECK(mehlissa::core::in_cubic_meters_per_pascal(compliance_sum) ==
          Catch::Approx(mehlissa::core::in_cubic_meters_per_pascal(
              state.aggregate.effective_pulmonary_arterial_compliance)));
    CHECK(1.0 / conductance_sum == Catch::Approx(mehlissa::core::in_pascal_seconds_per_cubic_meter(
                                       state.aggregate.effective_pulmonary_vascular_resistance)));
    CHECK(mehlissa::core::in_cubic_meters(blood_volume_sum) ==
          Catch::Approx(
              mehlissa::core::in_cubic_meters_per_second(state.aggregate.pulmonary_outflow) * 6.4));
    CHECK(state.beds[0].flow.si_value() > state.beds[1].flow.si_value());
}

TEST_CASE("An entity follows one deterministic lobe path and returns exactly once",
          "[m3][organ][pulmonary-0d][parallel][coupling]") {
    auto definition = lobar_definition();
    auto parameters = definition.model.zero_dimensional_parameters.value_or(
        mehlissa::models::organ::PulmonaryZeroDimensionalParameters{});
    auto& beds = parameters.parallel_beds;
    for (std::size_t index = 0; index < beds.size(); ++index) {
        beds[index].transit_time = std::chrono::seconds{static_cast<std::int64_t>(index + 1)};
    }
    definition.model.zero_dimensional_parameters = parameters;
    auto model = mehlissa::models::organ::make_lung_model(definition.model);
    auto* parallel =
        dynamic_cast<mehlissa::models::organ::PulmonaryParallelBedsModel*>(model.get());
    REQUIRE(parallel != nullptr);
    const auto selected = parallel->bed_index_for_entity(42);

    mehlissa::core::ComponentHost host{std::uint64_t{29}};
    host.add(std::move(model));
    host.initialize();
    parallel->accept_entity({
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        42,
        "nanodevice",
        "body.bvs95",
        "pulmonary-arterial-departure",
        "lung.pulmonary-0d.healthy-adult-lobar-parallel.v7",
        "pulmonary-arterial-entry",
        0ns,
    });
    CHECK(parallel->resident_entity_count() == 1);
    host.advance(std::chrono::seconds{static_cast<std::int64_t>(selected + 1)});
    const auto outbound = parallel->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().entity_id == 42);
    CHECK(outbound.front().target_port_id == "pulmonary-venous-return");
    CHECK(parallel->resident_entity_count() == 0);
}

TEST_CASE("Parallel beds reject a non-conservative perfusion partition",
          "[m3][organ][pulmonary-0d][parallel][validation]") {
    auto definition = lobar_definition();
    auto parameters = definition.model.zero_dimensional_parameters.value_or(
        mehlissa::models::organ::PulmonaryZeroDimensionalParameters{});
    parameters.parallel_beds.front().perfusion_fraction =
        mehlissa::core::Dimensionless::from_si(0.5);
    definition.model.zero_dimensional_parameters = parameters;
    CHECK_THROWS_AS(mehlissa::models::organ::make_lung_model(definition.model),
                    mehlissa::core::MehlissaError);
}

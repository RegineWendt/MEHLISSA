// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::organ::PulmonaryZeroDimensionalConfig;
using mehlissa::models::organ::PulmonaryZeroDimensionalModel;
using mehlissa::models::organ::PulmonaryZeroDimensionalParameters;

[[nodiscard]] PulmonaryZeroDimensionalConfig reference_config() {
    return {
        "organ.lung.pulmonary-0d",
        "lung.pulmonary-0d.v1",
        "pulmonary-arterial-entry",
        "pulmonary-venous-exit",
        "body.bvs95",
        "pulmonary-venous-return",
        PulmonaryZeroDimensionalParameters{
            mehlissa::core::liters_per_minute(6.0),
            mehlissa::core::millimeters_of_mercury(8.0),
            mehlissa::core::wood_units(1.2),
            mehlissa::core::milliliters_per_millimeter_of_mercury(5.0),
            6400ms,
            mehlissa::core::Dimensionless::from_si(0.556298773690078),
        },
    };
}

[[nodiscard]] mehlissa::models::coupling::VolumeFlowTransfer
flow_transfer(const double liters_per_minute) {
    return {
        {std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
         "pulmonary-flow-1", "body.bvs95", "pulmonary-arterial-departure", "lung.pulmonary-0d.v1",
         "pulmonary-arterial-entry", 0ns},
        mehlissa::core::liters_per_minute(liters_per_minute),
        1s,
    };
}

} // namespace

TEST_CASE("Pulmonary 0D reference starts at its source-scoped mean-flow equilibrium",
          "[m3][organ][pulmonary-0d][physiology]") {
    PulmonaryZeroDimensionalModel model{reference_config()};
    const auto state = model.state();

    CHECK(mehlissa::core::in_millimeters_of_mercury(state.mean_pulmonary_arterial_pressure) ==
          Catch::Approx(15.2));
    CHECK(mehlissa::core::in_liters_per_minute(state.pulmonary_outflow) == Catch::Approx(6.0));
    CHECK(mehlissa::core::in_liters_per_minute(state.right_lung_flow) ==
          Catch::Approx(3.33779264214));
    CHECK(mehlissa::core::in_liters_per_minute(state.left_lung_flow) ==
          Catch::Approx(2.66220735786));
    CHECK(mehlissa::core::in_wood_units(state.right_lung_resistance) ==
          Catch::Approx(1.2 / 0.556298773690078));
    CHECK(mehlissa::core::in_seconds(state.pressure_time_constant) == Catch::Approx(0.36));
}

TEST_CASE("A flow transfer drives the exact RC response and completes unchanged transit",
          "[m3][organ][pulmonary-0d][coupling]") {
    auto model = std::make_unique<PulmonaryZeroDimensionalModel>(reference_config());
    auto* observer = model.get();
    mehlissa::core::ComponentHost host{std::uint64_t{7}};
    host.add(std::move(model));
    host.initialize();

    observer->accept_conserved_transfer(flow_transfer(8.0));
    host.advance(360ms);

    const auto state = observer->state();
    const auto expected_pressure = 17.6 + (15.2 - 17.6) * std::exp(-1.0);
    CHECK(mehlissa::core::in_millimeters_of_mercury(state.mean_pulmonary_arterial_pressure) ==
          Catch::Approx(expected_pressure));
    CHECK(mehlissa::core::in_liters_per_minute(state.prescribed_inflow) == Catch::Approx(8.0));

    host.advance(6040ms);
    const auto outbound = observer->take_outbound_conserved_transfers();
    REQUIRE(outbound.size() == 1);
    const auto* returned =
        std::get_if<mehlissa::models::coupling::VolumeFlowTransfer>(&outbound.front());
    REQUIRE(returned != nullptr);
    CHECK(mehlissa::core::in_liters_per_minute(returned->flow_rate) == Catch::Approx(8.0));
    CHECK(returned->header.source_port_id == "pulmonary-venous-exit");
    CHECK(returned->header.target_port_id == "pulmonary-venous-return");
    CHECK(returned->header.emitted_at == 6400ms);
}

TEST_CASE("Pulmonary 0D parameters reject nonphysical configurations",
          "[m3][organ][pulmonary-0d][validation]") {
    auto config = reference_config();
    config.parameters.right_lung_perfusion_fraction = mehlissa::core::Dimensionless::from_si(1.0);
    CHECK_THROWS_AS(PulmonaryZeroDimensionalModel{std::move(config)},
                    mehlissa::core::MehlissaError);
}

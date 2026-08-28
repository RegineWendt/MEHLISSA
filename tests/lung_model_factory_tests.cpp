// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::organ::LungModelConfig;
using mehlissa::models::organ::LungModelVariant;
using mehlissa::models::organ::PulmonaryTransitRegion;

[[nodiscard]] LungModelConfig compartment_config() {
    return {
        LungModelVariant::effective_compartment,
        "organ.lung",
        "lung.compartment.v1",
        "arterial-entry",
        "venous-exit",
        "body",
        "venous-return",
        2s,
        {},
        std::nullopt,
    };
}

[[nodiscard]] LungModelConfig regional_config() {
    return {
        LungModelVariant::regional_circulation,
        "organ.lung",
        "lung.regional.v1",
        "arterial-entry",
        "venous-exit",
        "body",
        "venous-return",
        0s,
        {{"artery", 500ms}, {"capillary-surrogate", 1s}, {"vein", 500ms}},
        std::nullopt,
    };
}

[[nodiscard]] LungModelConfig zero_dimensional_config() {
    return {
        LungModelVariant::pulmonary_zero_dimensional,
        "organ.lung",
        "lung.pulmonary-0d.v1",
        "arterial-entry",
        "venous-exit",
        "body",
        "venous-return",
        0s,
        {},
        mehlissa::models::organ::PulmonaryZeroDimensionalParameters{
            mehlissa::core::liters_per_minute(5.0),
            mehlissa::core::millimeters_of_mercury(8.0),
            mehlissa::core::wood_units(1.2),
            mehlissa::core::milliliters_per_millimeter_of_mercury(5.0),
            6400ms,
            mehlissa::core::Dimensionless::from_si(0.5563),
            std::nullopt,
            std::nullopt,
            std::nullopt,
        },
    };
}

} // namespace

TEST_CASE("The lung model factory selects each implementation behind one contract",
          "[m3][organ][factory]") {
    auto compartment = mehlissa::models::organ::make_lung_model(compartment_config());
    auto regional = mehlissa::models::organ::make_lung_model(regional_config());
    auto zero_dimensional = mehlissa::models::organ::make_lung_model(zero_dimensional_config());

    CHECK(compartment->model_id() == std::string_view{"lung.compartment.v1"});
    CHECK(regional->model_id() == std::string_view{"lung.regional.v1"});
    CHECK(zero_dimensional->model_id() == std::string_view{"lung.pulmonary-0d.v1"});
    CHECK(compartment->accepts_entity_at("arterial-entry"));
    CHECK(regional->accepts_entity_at("arterial-entry"));
    CHECK(zero_dimensional->accepts_entity_at("arterial-entry"));
    CHECK(compartment->emits_entity_at("venous-exit"));
    CHECK(regional->emits_entity_at("venous-exit"));
    CHECK(zero_dimensional->emits_entity_at("venous-exit"));
}

TEST_CASE("A lung scenario cannot mix variant-specific parameters", "[m3][organ][factory]") {
    auto compartment = compartment_config();
    compartment.regions.push_back(PulmonaryTransitRegion{"unexpected", 1s});
    CHECK_THROWS_AS(mehlissa::models::organ::make_lung_model(std::move(compartment)),
                    mehlissa::core::MehlissaError);

    auto regional = regional_config();
    regional.compartment_transit_time = 1s;
    CHECK_THROWS_AS(mehlissa::models::organ::make_lung_model(std::move(regional)),
                    mehlissa::core::MehlissaError);

    auto zero_dimensional = zero_dimensional_config();
    zero_dimensional.regions.push_back(PulmonaryTransitRegion{"unexpected", 1s});
    CHECK_THROWS_AS(mehlissa::models::organ::make_lung_model(std::move(zero_dimensional)),
                    mehlissa::core::MehlissaError);
}

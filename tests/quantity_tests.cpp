// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/quantity.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <concepts>

namespace {

template <typename Left, typename Right>
concept Addable = requires(Left left, Right right) { left + right; };

template <typename From, typename To>
concept ImplicitlyConvertible = std::convertible_to<From, To>;

static_assert(Addable<mehlissa::core::Length, mehlissa::core::Length>);
static_assert(!Addable<mehlissa::core::Length, mehlissa::core::Amount>);
static_assert(!Addable<mehlissa::core::Speed, mehlissa::core::Time>);
static_assert(!ImplicitlyConvertible<double, mehlissa::core::Length>);
static_assert(!ImplicitlyConvertible<mehlissa::core::Length, double>);
static_assert(std::same_as<decltype(mehlissa::core::meters(1.0) / mehlissa::core::seconds(1.0)),
                           mehlissa::core::Speed>);
static_assert(std::same_as<decltype(mehlissa::core::moles(1.0) / mehlissa::core::cubic_meters(1.0)),
                           mehlissa::core::Concentration>);
static_assert(
    std::same_as<decltype(mehlissa::core::wood_units(1.0) * mehlissa::core::liters_per_minute(1.0)),
                 mehlissa::core::Pressure>);
static_assert(std::same_as<decltype(mehlissa::core::per_pascal(1.0) * mehlissa::core::pascals(1.0)),
                           mehlissa::core::Dimensionless>);
static_assert(std::same_as<decltype(mehlissa::core::wood_units(1.0) *
                                    mehlissa::core::milliliters_per_millimeter_of_mercury(1.0)),
                           mehlissa::core::Time>);
static_assert(std::same_as<decltype(mehlissa::core::cubic_meters_per_mole_second(1.0) *
                                    mehlissa::core::moles_per_cubic_meter(1.0)),
                           mehlissa::core::FirstOrderRate>);

} // namespace

TEST_CASE("Length unit conversions preserve the SI value", "[core][quantity]") {
    const auto length = mehlissa::core::meters(1.0) + mehlissa::core::millimeters(250.0) +
                        mehlissa::core::micrometers(500.0);

    REQUIRE(mehlissa::core::in_meters(length) == Catch::Approx(1.2505));
}

TEST_CASE("Derived dimensions produce speed and area", "[core][quantity]") {
    const mehlissa::core::Speed speed = mehlissa::core::meters(2.0) / mehlissa::core::seconds(4.0);
    const mehlissa::core::Area area =
        mehlissa::core::millimeters(3.0) * mehlissa::core::millimeters(4.0);

    REQUIRE(mehlissa::core::in_meters_per_second(speed) == Catch::Approx(0.5));
    REQUIRE(mehlissa::core::in_square_meters(area) == Catch::Approx(12.0e-6));
}

TEST_CASE("Amount and volume derive concentration without hidden conversion", "[core][quantity]") {
    const mehlissa::core::Concentration derived =
        mehlissa::core::millimoles(2.0) / mehlissa::core::liters(1.0);
    const auto declared = mehlissa::core::millimoles_per_liter(2.0);

    REQUIRE(derived == declared);
    REQUIRE(mehlissa::core::in_moles_per_cubic_meter(derived) == Catch::Approx(2.0));
}

TEST_CASE("Volume per time derives a dimension-safe flow rate", "[core][quantity]") {
    const auto flow = mehlissa::core::liters(6.0) / mehlissa::core::minutes(1.0);

    STATIC_REQUIRE(std::same_as<std::remove_cvref_t<decltype(flow)>, mehlissa::core::FlowRate>);
    REQUIRE(mehlissa::core::in_cubic_meters_per_second(flow) == Catch::Approx(0.0001));
}

TEST_CASE("Scalar operations retain the physical dimension", "[core][quantity]") {
    const auto doubled = 2.0 * mehlissa::core::millimeters(5.0);
    const auto halved = doubled / 2.0;

    REQUIRE(mehlissa::core::in_meters(doubled) == Catch::Approx(0.01));
    REQUIRE(halved == mehlissa::core::millimeters(5.0));
}

TEST_CASE("Pulmonary pressure resistance and compliance conversions preserve dimensions",
          "[core][quantity][m3]") {
    const auto pressure = mehlissa::core::wood_units(1.2) * mehlissa::core::liters_per_minute(5.0);
    const auto time_constant = mehlissa::core::wood_units(1.2) *
                               mehlissa::core::milliliters_per_millimeter_of_mercury(5.0);

    REQUIRE(mehlissa::core::in_millimeters_of_mercury(pressure) == Catch::Approx(6.0));
    REQUIRE(mehlissa::core::in_seconds(time_constant) == Catch::Approx(0.36));
    REQUIRE(mehlissa::core::in_wood_units(mehlissa::core::wood_units(1.2)) == Catch::Approx(1.2));
    REQUIRE(mehlissa::core::in_per_millimeter_of_mercury(
                mehlissa::core::per_millimeter_of_mercury(0.02)) == Catch::Approx(0.02));
    REQUIRE(mehlissa::core::in_milliliters_per_millimeter_of_mercury(
                mehlissa::core::milliliters_per_millimeter_of_mercury(5.0)) == Catch::Approx(5.0));
}

TEST_CASE("Association rate and concentration derive a first-order binding rate",
          "[core][quantity][m5]") {
    const auto binding_rate = mehlissa::core::cubic_meters_per_mole_second(1000.0) *
                              mehlissa::core::moles_per_cubic_meter(0.0003);

    REQUIRE(mehlissa::core::in_per_second(binding_rate) == Catch::Approx(0.3));
}

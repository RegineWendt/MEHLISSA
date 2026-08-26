// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/position.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Three-dimensional vessel distances are calculated in meters", "[core][geometry]") {
    const mehlissa::core::Position3D start{};
    const mehlissa::core::Position3D end{
        mehlissa::core::meters(3.0),
        mehlissa::core::meters(4.0),
        mehlissa::core::meters(12.0),
    };

    REQUIRE(mehlissa::core::in_meters(mehlissa::core::distance(start, end)) == Catch::Approx(13.0));
    REQUIRE(mehlissa::core::in_meters(mehlissa::core::distance(end, start)) == Catch::Approx(13.0));
}

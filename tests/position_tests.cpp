// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/position.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Three-dimensional vessel distances are calculated in meters", "[core][geometry]") {
    const mehlissa::core::Position3D start{};
    const mehlissa::core::Position3D end{3.0, 4.0, 12.0};

    REQUIRE(mehlissa::core::distance_meters(start, end) == Catch::Approx(13.0));
    REQUIRE(mehlissa::core::distance_meters(end, start) == Catch::Approx(13.0));
}

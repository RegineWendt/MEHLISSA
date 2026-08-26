// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/random_stream.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <stdexcept>

TEST_CASE("Named random streams are reproducible", "[core][rng]") {
    mehlissa::core::RandomStream first{42, "particle-routing"};
    mehlissa::core::RandomStream second{42, "particle-routing"};

    REQUIRE(first.derived_seed() == second.derived_seed());
    for (std::size_t sample = 0; sample < 32; ++sample) {
        REQUIRE(first.next_u64() == second.next_u64());
    }
}

TEST_CASE("Different random stream names derive independent sequences", "[core][rng]") {
    mehlissa::core::RandomStream routing{42, "particle-routing"};
    mehlissa::core::RandomStream injection{42, "particle-injection"};

    REQUIRE(routing.derived_seed() != injection.derived_seed());
    REQUIRE(routing.next_u64() != injection.next_u64());
}

TEST_CASE("Random streams require an explicit name", "[core][rng]") {
    REQUIRE_THROWS_AS(mehlissa::core::RandomStream(42, ""), std::invalid_argument);
}

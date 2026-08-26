// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/simulation_context.hpp>

#include <mehlissa/core/error.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

TEST_CASE("A simulation context owns reproducible named random streams",
          "[core][context][random]") {
    mehlissa::core::SimulationContext first{std::uint64_t{42}};
    mehlissa::core::SimulationContext second{std::uint64_t{42}};

    auto& first_transport = first.random_stream("transport");
    auto& same_transport = first.random_stream("transport");
    auto& second_transport = second.random_stream("transport");

    REQUIRE(&first_transport == &same_transport);
    REQUIRE(first.random_stream_count() == 1);
    REQUIRE(first_transport.next_u64() == second_transport.next_u64());
    REQUIRE(first_transport.next_u64() == second_transport.next_u64());
    REQUIRE(first.master_seed() == std::uint64_t{42});
    REQUIRE(first.random_stream_states() ==
            std::vector<mehlissa::core::RandomStreamState>{{"transport", 2}});
}

TEST_CASE("A simulation context rejects an unnamed random stream", "[core][context][random]") {
    mehlissa::core::SimulationContext context{std::uint64_t{1}};

    REQUIRE_THROWS_AS(context.random_stream(""), mehlissa::core::MehlissaError);
    REQUIRE(context.random_stream_count() == 0);
}

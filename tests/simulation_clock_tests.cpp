#include <mehlissa/core/simulation_clock.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <limits>
#include <stdexcept>

TEST_CASE("Simulation time advances exactly below one second", "[core][time]") {
    using namespace std::chrono_literals;

    mehlissa::core::SimulationClock clock;
    clock.advance(1ns);
    clock.advance(499ms);

    REQUIRE(clock.now() == 499'000'001ns);
}

TEST_CASE("Simulation time advances strictly monotonically", "[core][time]") {
    using namespace std::chrono_literals;

    mehlissa::core::SimulationClock clock;
    REQUIRE_THROWS_AS(clock.advance(0ns), std::invalid_argument);
    REQUIRE_THROWS_AS(clock.advance(-1ns), std::invalid_argument);
    REQUIRE(clock.now() == 0ns);
}

TEST_CASE("Simulation time rejects negative starts and overflow", "[core][time]") {
    using Duration = mehlissa::core::SimulationClock::Duration;

    REQUIRE_THROWS_AS(mehlissa::core::SimulationClock(Duration{-1}), std::invalid_argument);

    mehlissa::core::SimulationClock clock{Duration{std::numeric_limits<Duration::rep>::max()}};
    REQUIRE_THROWS_AS(clock.advance(Duration{1}), std::overflow_error);
}

#include <mehlissa/core/position.hpp>
#include <mehlissa/core/random_stream.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <stdexcept>

namespace {

[[nodiscard]] bool clock_is_exact_and_monotone() {
    using namespace std::chrono_literals;

    mehlissa::core::SimulationClock clock;
    clock.advance(1ns);
    clock.advance(499ms);
    if (clock.now() != 499'000'001ns) {
        return false;
    }

    try {
        clock.advance(0ns);
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

[[nodiscard]] bool geometry_is_three_dimensional() {
    const mehlissa::core::Position3D start{};
    const mehlissa::core::Position3D end{3.0, 4.0, 12.0};
    return std::abs(mehlissa::core::distance_meters(start, end) - 13.0) < 1.0e-12;
}

[[nodiscard]] bool named_random_streams_are_reproducible() {
    mehlissa::core::RandomStream first{42, "smoke-test"};
    mehlissa::core::RandomStream second{42, "smoke-test"};
    for (std::size_t sample = 0; sample < 32; ++sample) {
        if (first.next_u64() != second.next_u64()) {
            return false;
        }
    }
    return true;
}

} // namespace

int run_smoke_tests() {
    if (!clock_is_exact_and_monotone()) {
        std::fputs("Simulation-clock smoke test failed\n", stderr);
        return 1;
    }
    if (!geometry_is_three_dimensional()) {
        std::fputs("Geometry smoke test failed\n", stderr);
        return 2;
    }
    if (!named_random_streams_are_reproducible()) {
        std::fputs("Random-stream smoke test failed\n", stderr);
        return 3;
    }

    std::fputs("MEHLISSA core smoke tests passed\n", stdout);
    return 0;
}

int main() noexcept {
    try {
        return run_smoke_tests();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "MEHLISSA core smoke test failed: %s\n", error.what());
    } catch (...) {
        std::fputs("MEHLISSA core smoke test failed with an unknown error\n", stderr);
    }
    return 4;
}

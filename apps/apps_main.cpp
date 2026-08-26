// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/simulation_clock.hpp>

#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <exception>

namespace {

int run() {
    using namespace std::chrono_literals;

    mehlissa::core::SimulationClock clock;
    clock.advance(1ns);

    const auto current_time = static_cast<std::int64_t>(clock.now().count());
    std::printf("MEHLISSA Next bootstrap\nsimulation_time_ns=%" PRId64 "\n", current_time);
    return 0;
}

} // namespace

int main() noexcept {
    try {
        return run();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "MEHLISSA failed: %s\n", error.what());
    } catch (...) {
        std::fputs("MEHLISSA failed with an unknown error\n", stderr);
    }
    return 1;
}

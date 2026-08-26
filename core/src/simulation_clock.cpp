// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/simulation_clock.hpp>

#include <mehlissa/core/error.hpp>

#include <limits>

namespace mehlissa::core {

SimulationClock::SimulationClock(const Duration start) : current_(start) {
    if (start < Duration::zero()) {
        throw MehlissaError{ErrorCode::invariant_violated,
                            "Simulation time cannot start before zero"};
    }
}

SimulationClock::Duration SimulationClock::now() const noexcept { return current_; }

void SimulationClock::advance(const Duration delta) {
    if (delta <= Duration::zero()) {
        throw MehlissaError{ErrorCode::invariant_violated,
                            "Simulation time must advance by a positive duration"};
    }

    const auto maximum = std::numeric_limits<Duration::rep>::max();
    if (delta.count() > maximum - current_.count()) {
        throw MehlissaError{ErrorCode::numeric_overflow, "Simulation time overflow"};
    }

    current_ = Duration{current_.count() + delta.count()};
}

} // namespace mehlissa::core

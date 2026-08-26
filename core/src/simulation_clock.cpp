#include <mehlissa/core/simulation_clock.hpp>

#include <limits>
#include <stdexcept>

namespace mehlissa::core {

SimulationClock::SimulationClock(const Duration start) : current_(start) {
    if (start < Duration::zero()) {
        throw std::invalid_argument("Simulation time cannot start before zero");
    }
}

SimulationClock::Duration SimulationClock::now() const noexcept { return current_; }

void SimulationClock::advance(const Duration delta) {
    if (delta <= Duration::zero()) {
        throw std::invalid_argument("Simulation time must advance by a positive duration");
    }

    const auto maximum = std::numeric_limits<Duration::rep>::max();
    if (delta.count() > maximum - current_.count()) {
        throw std::overflow_error("Simulation time overflow");
    }

    current_ = Duration{current_.count() + delta.count()};
}

} // namespace mehlissa::core

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_SIMULATION_CLOCK_HPP
#define MEHLISSA_CORE_SIMULATION_CLOCK_HPP

#include <chrono>

namespace mehlissa::core {

class SimulationClock final {
  public:
    using Duration = std::chrono::nanoseconds;

    explicit SimulationClock(Duration start = Duration::zero());

    [[nodiscard]] Duration now() const noexcept;
    void advance(Duration delta);

  private:
    Duration current_;
};

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_SIMULATION_CLOCK_HPP

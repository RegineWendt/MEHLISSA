// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_SIMULATION_CONTEXT_HPP
#define MEHLISSA_CORE_SIMULATION_CONTEXT_HPP

#include <mehlissa/core/random_stream.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mehlissa::core {

class ComponentHost;

struct RandomStreamState final {
    std::string name;
    std::uint64_t draw_count{};

    [[nodiscard]] bool operator==(const RandomStreamState&) const noexcept = default;
};

class SimulationContext final {
  public:
    explicit SimulationContext(std::uint64_t master_seed);

    SimulationContext(const SimulationContext&) = delete;
    SimulationContext& operator=(const SimulationContext&) = delete;
    SimulationContext(SimulationContext&&) = delete;
    SimulationContext& operator=(SimulationContext&&) = delete;

    [[nodiscard]] const SimulationClock& clock() const noexcept;
    [[nodiscard]] std::uint64_t master_seed() const noexcept;
    [[nodiscard]] RandomStream& random_stream(std::string_view name);
    [[nodiscard]] std::size_t random_stream_count() const noexcept;
    [[nodiscard]] std::vector<RandomStreamState> random_stream_states() const;

  private:
    friend class ComponentHost;

    SimulationClock clock_;
    std::uint64_t master_seed_{};
    std::unordered_map<std::string, RandomStream> random_streams_;
};

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_SIMULATION_CONTEXT_HPP

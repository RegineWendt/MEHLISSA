// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_COMPONENT_HOST_HPP
#define MEHLISSA_CORE_COMPONENT_HOST_HPP

#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/core/simulation_context.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::core {

class SimulationComponent {
  public:
    SimulationComponent() = default;
    virtual ~SimulationComponent() = default;

    SimulationComponent(const SimulationComponent&) = delete;
    SimulationComponent& operator=(const SimulationComponent&) = delete;
    SimulationComponent(SimulationComponent&&) = delete;
    SimulationComponent& operator=(SimulationComponent&&) = delete;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual void initialize(SimulationContext& context) = 0;
    virtual void advance(SimulationContext& context, SimulationClock::Duration delta) = 0;
    virtual void finalize(SimulationContext& context) noexcept = 0;
};

class ComponentHost final {
  public:
    enum class State : std::uint8_t { building, initialized, finalized };

    explicit ComponentHost(std::uint64_t master_seed);
    ~ComponentHost() noexcept;

    ComponentHost(const ComponentHost&) = delete;
    ComponentHost& operator=(const ComponentHost&) = delete;
    ComponentHost(ComponentHost&&) = delete;
    ComponentHost& operator=(ComponentHost&&) = delete;

    void add(std::unique_ptr<SimulationComponent> component);
    void initialize();
    void advance(SimulationClock::Duration delta);
    void finalize() noexcept;

    [[nodiscard]] State state() const noexcept;
    [[nodiscard]] std::size_t component_count() const noexcept;
    [[nodiscard]] const SimulationContext& context() const noexcept;

  private:
    struct ComponentEntry final {
        std::string name;
        std::unique_ptr<SimulationComponent> component;
    };

    SimulationContext context_;
    std::vector<ComponentEntry> components_;
    std::size_t initialized_count_{};
    State state_{State::building};
};

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_COMPONENT_HOST_HPP

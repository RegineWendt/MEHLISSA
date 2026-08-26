// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace mehlissa::core {

ComponentHost::ComponentHost(const std::uint64_t master_seed) : context_{master_seed} {}

ComponentHost::~ComponentHost() noexcept { finalize(); }

void ComponentHost::add(std::unique_ptr<SimulationComponent> component) {
    if (state_ != State::building) {
        throw std::logic_error{"Components can only be added before initialization"};
    }
    if (!component) {
        throw std::invalid_argument{"A simulation component must not be null"};
    }

    const std::string component_name{component->name()};
    if (component_name.empty()) {
        throw std::invalid_argument{"A simulation component must have a non-empty name"};
    }
    const auto duplicate = std::ranges::find(components_, component_name, &ComponentEntry::name);
    if (duplicate != components_.end()) {
        throw std::invalid_argument{"Duplicate simulation component name: " + component_name};
    }

    components_.push_back(ComponentEntry{component_name, std::move(component)});
}

void ComponentHost::initialize() {
    if (state_ != State::building) {
        throw std::logic_error{"A component host can only be initialized once"};
    }

    try {
        for (auto& entry : components_) {
            entry.component->initialize(context_);
            ++initialized_count_;
        }
        state_ = State::initialized;
    } catch (...) {
        finalize();
        throw;
    }
}

void ComponentHost::advance(const SimulationClock::Duration delta) {
    if (state_ != State::initialized) {
        throw std::logic_error{"Only an initialized component host can advance"};
    }

    auto next_clock = context_.clock_;
    next_clock.advance(delta);
    for (auto& entry : components_) {
        entry.component->advance(context_, delta);
    }
    context_.clock_ = next_clock;
}

void ComponentHost::finalize() noexcept {
    while (initialized_count_ > 0) {
        --initialized_count_;
        components_[initialized_count_].component->finalize(context_);
    }
    state_ = State::finalized;
}

ComponentHost::State ComponentHost::state() const noexcept { return state_; }

std::size_t ComponentHost::component_count() const noexcept { return components_.size(); }

const SimulationContext& ComponentHost::context() const noexcept { return context_; }

} // namespace mehlissa::core

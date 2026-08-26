// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/simulation_context.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <string>

namespace mehlissa::core {

SimulationContext::SimulationContext(const std::uint64_t master_seed) : master_seed_{master_seed} {}

const SimulationClock& SimulationContext::clock() const noexcept { return clock_; }

std::uint64_t SimulationContext::master_seed() const noexcept { return master_seed_; }

RandomStream& SimulationContext::random_stream(const std::string_view name) {
    if (name.empty()) {
        throw MehlissaError{ErrorCode::invariant_violated,
                            "A random stream must have a non-empty name"};
    }

    const std::string owned_name{name};
    const auto [stream, inserted] =
        random_streams_.try_emplace(owned_name, master_seed_, owned_name);
    static_cast<void>(inserted);
    return stream->second;
}

std::size_t SimulationContext::random_stream_count() const noexcept {
    return random_streams_.size();
}

std::vector<RandomStreamState> SimulationContext::random_stream_states() const {
    std::vector<RandomStreamState> states;
    states.reserve(random_streams_.size());
    for (const auto& [name, stream] : random_streams_) {
        states.push_back(RandomStreamState{name, stream.draw_count()});
    }
    std::ranges::sort(states, {}, &RandomStreamState::name);
    return states;
}

} // namespace mehlissa::core

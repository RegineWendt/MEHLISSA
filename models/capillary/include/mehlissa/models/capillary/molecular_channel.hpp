// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_HPP
#define MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <string>
#include <string_view>

namespace mehlissa::models::capillary {

struct MolecularChannelRequest final {
    std::string request_id;
    std::string signal_id;
    std::string context_model_id;
    std::string region_id;
    core::Amount emitted_amount{};
    core::Length transmitter_receiver_separation{};
    core::Volume passive_receiver_volume{};
    core::SimulationClock::Duration observation_time{};
};

struct MolecularChannelResponse final {
    std::string request_id;
    std::string signal_id;
    std::string channel_model_id;
    core::SimulationClock::Duration observation_time{};
    core::Concentration expected_receiver_concentration{};
    core::Amount expected_receiver_amount{};
    double expected_receiver_fraction{};
};

class MolecularChannel {
  public:
    virtual ~MolecularChannel() = default;

    [[nodiscard]] virtual std::string_view kind() const noexcept = 0;
    [[nodiscard]] virtual std::string_view model_id() const noexcept = 0;
    [[nodiscard]] virtual MolecularChannelResponse
    evaluate(const MolecularChannelRequest& request) const = 0;
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_COMPARISON_HPP
#define MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_COMPARISON_HPP

#include <mehlissa/models/capillary/brownian_particle_channel.hpp>

#include <cstdint>

namespace mehlissa::models::capillary {

struct MolecularChannelComparisonGate final {
    std::uint64_t minimum_receiver_observation_count{};
    double maximum_absolute_standardized_error{};
    double maximum_relative_fraction_error{};
};

struct MolecularChannelComparisonResult final {
    MolecularChannelResponse reference;
    BrownianParticleChannelEvaluation particle;
    double absolute_fraction_error{};
    double relative_fraction_error{};
    double standardized_error{};
    bool passes{};
};

[[nodiscard]] MolecularChannelComparisonResult compare_molecular_channels(
    const MolecularChannel& reference, const BrownianParticleChannel& particle,
    const MolecularChannelRequest& request, const MolecularChannelComparisonGate& gate);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_COMPARISON_HPP

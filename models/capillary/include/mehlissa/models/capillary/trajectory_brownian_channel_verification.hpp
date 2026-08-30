// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_VERIFICATION_HPP
#define MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_VERIFICATION_HPP

#include <mehlissa/models/capillary/trajectory_brownian_channel.hpp>

#include <cstdint>

namespace mehlissa::models::capillary {

struct TrajectoryBrownianVerificationGate final {
    std::uint64_t minimum_receiver_observation_count{};
    double maximum_absolute_reference_standardized_error{};
    double maximum_relative_fraction_error{};
    double maximum_refinement_standardized_difference{};
    double maximum_relative_mean_squared_displacement_error{};
};

struct TrajectoryBrownianVerificationResult final {
    MolecularChannelResponse reference;
    TrajectoryBrownianChannelEvaluation coarse;
    TrajectoryBrownianChannelEvaluation refined;
    double coarse_reference_standardized_error{};
    double refined_reference_standardized_error{};
    double coarse_relative_fraction_error{};
    double refined_relative_fraction_error{};
    double refinement_standardized_difference{};
    double coarse_relative_mean_squared_displacement_error{};
    double refined_relative_mean_squared_displacement_error{};
    bool passes{};
};

[[nodiscard]] TrajectoryBrownianVerificationResult verify_trajectory_brownian_refinement(
    const MolecularChannel& reference, const TrajectoryBrownianChannel& coarse,
    const TrajectoryBrownianChannel& refined, const MolecularChannelRequest& request,
    const TrajectoryBrownianVerificationGate& gate);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_VERIFICATION_HPP

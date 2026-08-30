// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_PROFILE_HPP

#include <mehlissa/models/capillary/molecular_channel_profile.hpp>
#include <mehlissa/models/capillary/trajectory_brownian_channel.hpp>
#include <mehlissa/models/capillary/trajectory_brownian_channel_verification.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto trajectory_brownian_channel_profile_schema_version = "1.0.0";

struct TrajectoryBrownianChannelProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    std::string compatible_analytical_profile_id;
    TrajectoryBrownianChannelConfig refined_channel;
    std::uint64_t coarse_step_count{};
    TrajectoryBrownianVerificationGate verification_gate;
    MolecularChannelValidity validity;
    std::vector<MolecularChannelSource> sources;
    std::vector<std::string> limitations;
};

struct TrajectoryBrownianChannelProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_trajectory_brownian_channel_profile(const TrajectoryBrownianChannelProfile& profile);

[[nodiscard]] TrajectoryBrownianChannelProfile load_trajectory_brownian_channel_profile(
    const TrajectoryBrownianChannelProfileLoadRequest& request);

[[nodiscard]] TrajectoryBrownianChannel
make_trajectory_brownian_channel(const TrajectoryBrownianChannelProfile& trajectory_profile,
                                 const MolecularChannelProfile& analytical_profile,
                                 bool coarse_resolution = false);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_PROFILE_HPP

#include <mehlissa/models/capillary/brownian_particle_channel.hpp>
#include <mehlissa/models/capillary/molecular_channel_comparison.hpp>
#include <mehlissa/models/capillary/molecular_channel_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto brownian_particle_channel_profile_schema_version = "1.0.0";

struct BrownianParticleChannelProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    std::string compatible_analytical_profile_id;
    BrownianParticleChannelConfig channel;
    MolecularChannelComparisonGate comparison_gate;
    MolecularChannelValidity validity;
    std::vector<MolecularChannelSource> sources;
    std::vector<std::string> limitations;
};

struct BrownianParticleChannelProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_brownian_particle_channel_profile(const BrownianParticleChannelProfile& profile);

[[nodiscard]] BrownianParticleChannelProfile
load_brownian_particle_channel_profile(const BrownianParticleChannelProfileLoadRequest& request);

[[nodiscard]] BrownianParticleChannel
make_brownian_particle_channel(const BrownianParticleChannelProfile& particle_profile,
                               const MolecularChannelProfile& analytical_profile);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_PROFILE_HPP

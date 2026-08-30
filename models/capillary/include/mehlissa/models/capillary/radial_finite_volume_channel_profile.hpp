// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_PROFILE_HPP

#include <mehlissa/models/capillary/molecular_channel_profile.hpp>
#include <mehlissa/models/capillary/radial_finite_volume_channel.hpp>
#include <mehlissa/models/capillary/radial_finite_volume_channel_verification.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto radial_finite_volume_channel_profile_schema_version = "1.0.0";

struct RadialFiniteVolumeChannelProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    std::string compatible_analytical_profile_id;
    RadialFiniteVolumeChannelConfig refined_channel;
    std::uint64_t coarse_radial_cell_count{};
    RadialFiniteVolumeVerificationGate verification_gate;
    MolecularChannelValidity validity;
    std::vector<MolecularChannelSource> sources;
    std::vector<std::string> limitations;
};

struct RadialFiniteVolumeChannelProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_radial_finite_volume_channel_profile(const RadialFiniteVolumeChannelProfile& profile);

[[nodiscard]] RadialFiniteVolumeChannelProfile load_radial_finite_volume_channel_profile(
    const RadialFiniteVolumeChannelProfileLoadRequest& request);

[[nodiscard]] RadialFiniteVolumeChannel
make_radial_finite_volume_channel(const RadialFiniteVolumeChannelProfile& field_profile,
                                  const MolecularChannelProfile& analytical_profile,
                                  bool coarse_resolution = false);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_PROFILE_HPP

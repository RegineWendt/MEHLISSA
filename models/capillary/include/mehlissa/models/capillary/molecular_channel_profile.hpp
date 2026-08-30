// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_PROFILE_HPP

#include <mehlissa/models/capillary/analytical_diffusion_channel.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto molecular_channel_profile_schema_version = "1.0.0";

struct MolecularChannelValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct MolecularChannelSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct MolecularChannelReferenceCase final {
    std::string request_id;
    std::string signal_id;
    std::string compatible_capillary_definition_id;
    std::string capillary_region_id;
    double separation_fraction_of_diameter{};
    double receiver_radius_fraction_of_separation{};
    core::Amount emitted_amount{};
    std::string observation_time_mode;
};

struct MolecularChannelProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    AnalyticalDiffusionChannelConfig channel;
    MolecularChannelReferenceCase reference_case;
    MolecularChannelValidity validity;
    std::vector<MolecularChannelSource> sources;
    std::vector<std::string> limitations;
};

struct MolecularChannelProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_molecular_channel_profile(const MolecularChannelProfile& profile);

[[nodiscard]] MolecularChannelProfile
load_molecular_channel_profile(const MolecularChannelProfileLoadRequest& request);

[[nodiscard]] std::unique_ptr<MolecularChannel>
make_molecular_channel(const MolecularChannelProfile& profile);

[[nodiscard]] MolecularChannelRequest
make_capillary_reference_request(const MolecularChannelProfile& profile,
                                 const CapillaryBedDefinition& capillary_definition);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_MOLECULAR_CHANNEL_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_PROFILE_HPP

#include <mehlissa/models/capillary/axial_advection_reaction_case.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/molecular_channel_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto axial_advection_reaction_profile_schema_version = "1.0.0";

struct AxialAdvectionReactionProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    std::string compatible_capillary_definition_id;
    std::string capillary_region_id;
    AxialAdvectionReactionConfig channel;
    AxialParticleConfig particle;
    std::uint64_t coarse_field_cell_count{};
    AxialFieldConfig refined_field;
    AxialAdvectionReactionVerificationGate verification_gate;
    MolecularChannelValidity validity;
    std::vector<MolecularChannelSource> sources;
    std::vector<std::string> limitations;
};

struct AxialAdvectionReactionProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_axial_advection_reaction_profile(const AxialAdvectionReactionProfile& profile);

[[nodiscard]] AxialAdvectionReactionProfile
load_axial_advection_reaction_profile(const AxialAdvectionReactionProfileLoadRequest& request);

[[nodiscard]] AxialAdvectionReactionVerificationResult
verify_axial_advection_reaction_profile(const AxialAdvectionReactionProfile& profile,
                                        const CapillaryBedDefinition& definition);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_RECRUITMENT_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_RECRUITMENT_PROFILE_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_recruitment_profile_schema_version = "1.0.0";

enum class CapillaryBoundaryCondition : std::uint8_t { fixed_total_flow, fixed_pressure_drop };

[[nodiscard]] std::string_view to_string(CapillaryBoundaryCondition condition) noexcept;

struct PrecapillarySphincterGroup final {
    std::string id;
    std::uint64_t path_count{};
};

struct CapillaryRecruitmentState final {
    std::string id;
    core::SimulationClock::Duration effective_at{};
    std::vector<std::string> open_sphincter_group_ids;
};

struct CapillaryRecruitmentValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct CapillaryRecruitmentSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct CapillaryRecruitmentProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string compatible_model_id;
    CapillaryBoundaryCondition boundary_condition{};
    std::vector<PrecapillarySphincterGroup> sphincter_groups;
    std::vector<CapillaryRecruitmentState> states;
    CapillaryRecruitmentValidity validity;
    std::vector<CapillaryRecruitmentSource> sources;
    std::vector<std::string> limitations;
};

struct CapillaryRecruitmentProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_capillary_recruitment_profile(const CapillaryRecruitmentProfile& profile);

[[nodiscard]] CapillaryRecruitmentProfile
load_capillary_recruitment_profile(const CapillaryRecruitmentProfileLoadRequest& request);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_RECRUITMENT_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_PROFILE_HPP

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_entity_observation_profile_schema_version = "1.0.0";

struct CapillaryEntityInteractionRule final {
    std::string entity_type;
    double retention_rate_per_second{};
    double adhesion_rate_per_second{};
    double extravasation_rate_per_second{};
};

struct CapillaryEntityObservationValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct CapillaryEntityObservationSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct CapillaryEntityObservationProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string compatible_model_id;
    std::string unmatched_entity_policy;
    std::size_t maximum_buffered_records{};
    std::vector<CapillaryEntityInteractionRule> entity_rules;
    CapillaryEntityObservationValidity validity;
    std::vector<CapillaryEntityObservationSource> sources;
    std::vector<std::string> limitations;
};

struct CapillaryEntityObservationProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_capillary_entity_observation_profile(
    const CapillaryEntityObservationProfile& profile);

[[nodiscard]] CapillaryEntityObservationProfile load_capillary_entity_observation_profile(
    const CapillaryEntityObservationProfileLoadRequest& request);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_PROFILE_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_exchange_profile_schema_version = "1.0.0";

struct CapillarySubstanceExchangeRule final {
    std::string substance_id;
    double blood_to_endothelium_fraction{};
    double endothelium_to_interstitium_fraction{};
    double interstitium_to_cell_fraction{};
};

struct CapillaryExchangeValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct CapillaryExchangeSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct CapillaryExchangeProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string compatible_model_id;
    std::string unmatched_substance_policy;
    std::vector<CapillarySubstanceExchangeRule> substance_rules;
    CapillaryExchangeValidity validity;
    std::vector<CapillaryExchangeSource> sources;
    std::vector<std::string> limitations;
};

struct CapillaryExchangeProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_capillary_exchange_profile(const CapillaryExchangeProfile& profile);

[[nodiscard]] CapillaryExchangeProfile
load_capillary_exchange_profile(const CapillaryExchangeProfileLoadRequest& request);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP

#include <mehlissa/models/capillary/capillary_bed.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_bed_definition_schema_version = "1.0.0";

struct CapillaryBedValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct CapillaryBedSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct CapillaryBedDefinition final {
    std::string schema_version;
    std::string definition_id;
    std::string definition_version;
    std::string title;
    CapillaryBedConfig model;
    CapillaryBedValidity validity;
    std::vector<CapillaryBedSource> sources;
    std::vector<std::string> limitations;
};

struct CapillaryBedDefinitionLoadRequest final {
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] CapillaryBedDefinition
load_capillary_bed_definition(const CapillaryBedDefinitionLoadRequest& request);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP

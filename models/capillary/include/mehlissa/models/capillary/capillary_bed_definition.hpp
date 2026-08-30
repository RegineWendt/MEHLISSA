// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP

#include <mehlissa/models/capillary/capillary_bed.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_bed_definition_schema_version = "3.0.0";

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

struct CapillaryEvidenceUncertainty final {
    std::string kind;
    std::optional<double> standard_deviation_si;
    std::optional<double> lower_si;
    std::optional<double> upper_si;
};

struct CapillaryEvidenceQuantity final {
    double value_si{};
    std::string unit;
    CapillaryEvidenceUncertainty uncertainty;
    std::vector<std::string> source_ids;
    std::string role;
    std::string derivation;
};

struct CapillaryBedQualification final {
    std::string geometry_semantics;
    CapillaryEvidenceQuantity reference_flow;
    CapillaryEvidenceQuantity functional_blood_volume;
    CapillaryEvidenceQuantity morphometric_lumen_volume;
    CapillaryEvidenceQuantity morphometric_surface_area;
    CapillaryEvidenceQuantity equivalent_diameter;
    CapillaryEvidenceQuantity representative_path_length;
    CapillaryEvidenceQuantity reference_transit_time;
    std::string boundary_region_semantics;
    CapillaryEvidenceQuantity boundary_region_volume_each;
    double consistency_tolerance_fraction{};
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
    std::optional<CapillaryBedQualification> qualification;
};

struct CapillaryBedDefinitionLoadRequest final {
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] CapillaryBedDefinition
load_capillary_bed_definition(const CapillaryBedDefinitionLoadRequest& request);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_DEFINITION_HPP

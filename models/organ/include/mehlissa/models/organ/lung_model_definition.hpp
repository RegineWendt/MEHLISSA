// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP
#define MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP

#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::models::organ {

inline constexpr auto earliest_supported_lung_model_definition_schema_version = "1.0.0";
inline constexpr auto latest_supported_lung_model_definition_schema_version = "1.5.0";

struct LungModelValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct LungModelSource final {
    std::string id;
    std::string citation;
    std::string url;
    std::string license;
    std::string role;
};

struct ExternalPulmonaryDataReference final {
    std::string source_id;
    std::filesystem::path path;
    std::string sha256;
    std::string format;
    std::string coordinate_system;
    std::string length_unit;
    std::string flow_unit;
    std::vector<std::string> transformations;
};

struct LungModelParameterUncertainty final {
    std::string kind;
    std::optional<double> lower_si;
    std::optional<double> upper_si;
};

struct LungModelEvidenceQuantity final {
    double value_si{};
    std::string unit;
    LungModelParameterUncertainty uncertainty;
    std::string source_id;
    std::string role;
    std::string derivation;
};

struct PulmonaryFlowAdaptationEvidence final {
    LungModelEvidenceQuantity reference_cardiac_output;
    LungModelEvidenceQuantity resistance_exponent;
    LungModelEvidenceQuantity compliance_exponent;
    LungModelEvidenceQuantity maximum_flow_ratio;
};

struct PulmonaryAgeConditioningEvidence final {
    LungModelEvidenceQuantity default_age_years;
    LungModelEvidenceQuantity minimum_supported_age_years;
    LungModelEvidenceQuantity young_upper_age_years;
    LungModelEvidenceQuantity older_lower_age_years;
    LungModelEvidenceQuantity maximum_supported_age_years;
    LungModelEvidenceQuantity young_resistance_multiplier;
    LungModelEvidenceQuantity older_resistance_multiplier;
};

struct PulmonaryPressureDistensibilityEvidence final {
    LungModelEvidenceQuantity reference_cardiac_output;
    LungModelEvidenceQuantity reference_left_atrial_pressure;
    LungModelEvidenceQuantity coefficient;
    std::optional<LungModelEvidenceQuantity> older_coefficient;
};

struct PulmonaryHemodynamicEvidence final {
    LungModelEvidenceQuantity baseline_cardiac_output;
    LungModelEvidenceQuantity left_atrial_pressure;
    LungModelEvidenceQuantity pulmonary_vascular_resistance;
    LungModelEvidenceQuantity pulmonary_arterial_compliance;
    LungModelEvidenceQuantity pulmonary_transit_time;
    LungModelEvidenceQuantity right_lung_perfusion_fraction;
    LungModelEvidenceQuantity mean_pulmonary_arterial_pressure_target;
    std::optional<PulmonaryFlowAdaptationEvidence> flow_adaptation;
    std::optional<PulmonaryAgeConditioningEvidence> age_conditioning;
    std::optional<PulmonaryPressureDistensibilityEvidence> pressure_distensibility;
};

struct LungModelDefinition final {
    std::string schema_version;
    std::string definition_id;
    std::string definition_version;
    std::string title;
    LungModelConfig model;
    LungModelValidity validity;
    std::vector<LungModelSource> sources;
    std::vector<std::string> limitations;
    std::optional<ExternalPulmonaryDataReference> external_data;
    std::optional<PulmonaryHemodynamicEvidence> hemodynamics;
};

struct LungModelDefinitionLoadRequest final {
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] LungModelDefinition
load_lung_model_definition(const LungModelDefinitionLoadRequest& request);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP

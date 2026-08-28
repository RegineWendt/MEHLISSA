// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_VALIDATION_HPP
#define MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_VALIDATION_HPP

#include <mehlissa/models/organ/lung_model_definition.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::models::organ {

inline constexpr auto pulmonary_zero_dimensional_validation_schema_version = "1.0.0";

enum class PulmonaryValidationScope : std::uint8_t {
    declared_scope,
    near_scope_crosscheck,
    stress_test
};

enum class PulmonaryValidationBoundaryPolicy : std::uint8_t {
    locked_left_atrial_pressure,
    measured_wedge_pressure
};

enum class PulmonaryValidationAcceptanceRole : std::uint8_t { input, required, diagnostic };

struct PulmonaryValidationObservation final {
    double mean_si{};
    double standard_deviation_si{};
    std::string unit;
    PulmonaryValidationAcceptanceRole role{};
};

struct PulmonaryValidationSource final {
    std::string id;
    std::string citation;
    std::string url;
    std::string license;
    std::string measurement_method;
};

struct PulmonaryValidationCondition final {
    std::string id;
    std::string source_id;
    std::string description;
    PulmonaryValidationScope scope{};
    PulmonaryValidationBoundaryPolicy boundary_policy{};
    PulmonaryValidationObservation cardiac_output;
    std::optional<PulmonaryValidationObservation> pulmonary_arterial_wedge_pressure;
    PulmonaryValidationObservation mean_pulmonary_arterial_pressure;
    std::optional<PulmonaryValidationObservation> pulmonary_arterial_compliance;
    std::optional<PulmonaryValidationObservation> rc_time_constant;
};

struct PulmonaryZeroDimensionalValidationCase final {
    std::string schema_version;
    std::string validation_id;
    std::string title;
    std::string model_definition_id;
    double maximum_absolute_z_score{};
    std::vector<PulmonaryValidationSource> sources;
    std::vector<PulmonaryValidationCondition> conditions;
    std::vector<std::string> limitations;
};

struct PulmonaryValidationCaseLoadRequest final {
    std::filesystem::path validation_path;
    std::filesystem::path schema_path;
};

struct PulmonaryValidationEndpointResult final {
    std::string endpoint;
    PulmonaryValidationAcceptanceRole role{};
    double observed_mean_si{};
    double observed_standard_deviation_si{};
    double predicted_si{};
    double absolute_z_score{};
    bool accepted{};
};

struct PulmonaryValidationConditionResult final {
    std::string condition_id;
    PulmonaryValidationScope scope{};
    std::vector<PulmonaryValidationEndpointResult> endpoints;
};

struct PulmonaryZeroDimensionalValidationReport final {
    std::string validation_id;
    std::string model_definition_id;
    bool source_independence_verified{};
    bool required_endpoints_pass{};
    std::size_t required_endpoint_count{};
    std::size_t accepted_required_endpoint_count{};
    std::size_t failed_diagnostic_endpoint_count{};
    std::vector<PulmonaryValidationConditionResult> conditions;
};

[[nodiscard]] PulmonaryZeroDimensionalValidationCase
load_pulmonary_zero_dimensional_validation_case(const PulmonaryValidationCaseLoadRequest& request);

[[nodiscard]] PulmonaryZeroDimensionalValidationReport
evaluate_pulmonary_zero_dimensional_validation(
    const PulmonaryZeroDimensionalValidationCase& validation,
    const LungModelDefinition& model_definition);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_VALIDATION_HPP

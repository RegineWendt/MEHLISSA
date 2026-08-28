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
inline constexpr auto pulmonary_zero_dimensional_multipoint_validation_schema_version = "1.0.0";
inline constexpr auto pulmonary_zero_dimensional_population_multipoint_validation_schema_version =
    "1.1.0";

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
    std::string data_access;
    std::string cohort_independence;
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

enum class PulmonaryMultipointEvidenceStatus : std::uint8_t {
    measured_validation,
    synthetic_test_only
};

enum class PulmonaryBodyPosition : std::uint8_t { supine, semiupright, upright };

struct PulmonaryMultipointStage final {
    std::string id;
    std::size_t ordinal{};
    double workload_watts{};
    double cardiac_output_si{};
    double pulmonary_arterial_wedge_pressure_si{};
    double mean_pulmonary_arterial_pressure_si{};
    std::optional<double> systolic_pulmonary_arterial_pressure_si;
    std::optional<double> diastolic_pulmonary_arterial_pressure_si;
    std::optional<double> heart_rate_per_minute;
};

struct PulmonaryMultipointSubject final {
    std::string id;
    std::string source_id;
    std::string phenotype;
    PulmonaryBodyPosition body_position{};
    std::string cardiac_output_method;
    std::vector<PulmonaryMultipointStage> stages;
};

struct PulmonaryZeroDimensionalMultipointValidationCase final {
    std::string schema_version;
    std::string validation_id;
    std::string title;
    std::string model_definition_id;
    PulmonaryMultipointEvidenceStatus evidence_status{};
    std::size_t minimum_stage_count{};
    std::vector<PulmonaryValidationSource> sources;
    std::vector<PulmonaryMultipointSubject> subjects;
    std::vector<std::string> limitations;
};

struct PulmonaryMultipointValidationCaseLoadRequest final {
    std::filesystem::path validation_path;
    std::filesystem::path schema_path;
    bool allow_synthetic_test_data{};
};

struct PulmonaryLinearFit final {
    double slope{};
    double intercept{};
    double coefficient_of_determination{};
};

struct PulmonaryMultipointStageResult final {
    std::string stage_id;
    double workload_watts{};
    double cardiac_output_si{};
    double observed_mean_pulmonary_arterial_pressure_si{};
    double predicted_mean_pulmonary_arterial_pressure_si{};
    double pressure_residual_si{};
    double observed_pulmonary_vascular_resistance_si{};
    double predicted_pulmonary_vascular_resistance_si{};
    std::optional<double> observed_pulmonary_arterial_compliance_si;
    double predicted_pulmonary_arterial_compliance_si{};
    std::optional<double> observed_rc_time_constant_si;
    double predicted_rc_time_constant_si{};
};

struct PulmonaryMultipointSubjectResult final {
    std::string subject_id;
    PulmonaryLinearFit observed_mpap_flow_fit;
    PulmonaryLinearFit predicted_mpap_flow_fit;
    PulmonaryLinearFit observed_pawp_flow_fit;
    double mean_pressure_error_si{};
    double root_mean_square_pressure_error_si{};
    std::vector<PulmonaryMultipointStageResult> stages;
};

struct PulmonaryZeroDimensionalMultipointValidationReport final {
    std::string validation_id;
    std::string model_definition_id;
    bool source_independence_verified{};
    bool measured_evidence{};
    std::size_t subject_count{};
    std::size_t stage_count{};
    std::vector<PulmonaryMultipointSubjectResult> subjects;
};

enum class PulmonaryPopulationStatisticKind : std::uint8_t {
    mean_standard_deviation,
    mean_confidence_interval_95
};

enum class PulmonaryPopulationFlowBasis : std::uint8_t { absolute_cardiac_output, cardiac_index };

enum class PulmonaryPopulationChallengeKind : std::uint8_t { rest, passive_leg_raise, exercise };

struct PulmonaryPopulationObservation final {
    double mean_si{};
    std::optional<double> standard_deviation_si;
    std::optional<double> confidence_interval_95_lower_si;
    std::optional<double> confidence_interval_95_upper_si;
    std::string reported_unit;
};

struct PulmonaryPopulationStage final {
    std::string id;
    std::size_t ordinal{};
    PulmonaryPopulationChallengeKind challenge_kind{};
    std::optional<double> workload_watts_mean;
    PulmonaryPopulationObservation cardiac_flow;
    PulmonaryPopulationObservation pulmonary_arterial_wedge_pressure;
    PulmonaryPopulationObservation mean_pulmonary_arterial_pressure;
    std::optional<PulmonaryPopulationObservation> heart_rate;
};

struct PulmonaryPopulationSeries final {
    std::string id;
    std::string source_id;
    std::string phenotype;
    std::size_t sample_size{};
    PulmonaryBodyPosition body_position{};
    std::string cardiac_output_method;
    PulmonaryPopulationStatisticKind statistic_kind{};
    PulmonaryPopulationFlowBasis flow_basis{};
    std::optional<double> reference_body_surface_area_m2;
    std::optional<double> representative_age_years;
    std::string cohort_overlap;
    std::vector<PulmonaryPopulationStage> stages;
};

struct PulmonaryZeroDimensionalPopulationMultipointValidationCase final {
    std::string schema_version;
    std::string validation_id;
    std::string title;
    std::string model_definition_id;
    std::size_t minimum_stage_count{};
    double standard_deviation_multiplier{};
    std::vector<PulmonaryValidationSource> sources;
    std::vector<PulmonaryPopulationSeries> series;
    std::vector<std::string> limitations;
};

struct PulmonaryPopulationMultipointValidationCaseLoadRequest final {
    std::filesystem::path validation_path;
    std::filesystem::path schema_path;
};

struct PulmonaryPopulationStageResult final {
    std::string stage_id;
    std::optional<double> workload_watts_mean;
    double cardiac_output_si{};
    double observed_mean_pulmonary_arterial_pressure_si{};
    double predicted_mean_pulmonary_arterial_pressure_si{};
    double pressure_residual_si{};
    double acceptance_lower_si{};
    double acceptance_upper_si{};
    std::optional<double> absolute_z_score;
    bool accepted{};
};

struct PulmonaryPopulationSeriesResult final {
    std::string series_id;
    std::size_t sample_size{};
    std::optional<double> representative_age_years;
    double age_resistance_multiplier{1.0};
    PulmonaryLinearFit observed_mpap_flow_fit;
    PulmonaryLinearFit predicted_mpap_flow_fit;
    double root_mean_square_pressure_error_si{};
    std::size_t accepted_stage_count{};
    std::vector<PulmonaryPopulationStageResult> stages;
};

struct PulmonaryZeroDimensionalPopulationMultipointValidationReport final {
    std::string validation_id;
    std::string model_definition_id;
    bool source_independence_verified{};
    bool published_population_evidence{};
    bool all_stages_agree{};
    std::size_t series_count{};
    std::size_t stage_count{};
    std::size_t accepted_stage_count{};
    std::vector<PulmonaryPopulationSeriesResult> series;
};

[[nodiscard]] PulmonaryZeroDimensionalValidationCase
load_pulmonary_zero_dimensional_validation_case(const PulmonaryValidationCaseLoadRequest& request);

[[nodiscard]] PulmonaryZeroDimensionalValidationReport
evaluate_pulmonary_zero_dimensional_validation(
    const PulmonaryZeroDimensionalValidationCase& validation,
    const LungModelDefinition& model_definition);

[[nodiscard]] PulmonaryZeroDimensionalMultipointValidationCase
load_pulmonary_zero_dimensional_multipoint_validation_case(
    const PulmonaryMultipointValidationCaseLoadRequest& request);

[[nodiscard]] PulmonaryZeroDimensionalMultipointValidationReport
evaluate_pulmonary_zero_dimensional_multipoint_validation(
    const PulmonaryZeroDimensionalMultipointValidationCase& validation,
    const LungModelDefinition& model_definition);

[[nodiscard]] PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_pulmonary_zero_dimensional_population_multipoint_validation_case(
    const PulmonaryPopulationMultipointValidationCaseLoadRequest& request);

[[nodiscard]] PulmonaryZeroDimensionalPopulationMultipointValidationReport
evaluate_pulmonary_zero_dimensional_population_multipoint_validation(
    const PulmonaryZeroDimensionalPopulationMultipointValidationCase& validation,
    const LungModelDefinition& model_definition);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_VALIDATION_HPP

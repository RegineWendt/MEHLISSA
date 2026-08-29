// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_PULMONARY_LOBAR_PERFUSION_VALIDATION_HPP
#define MEHLISSA_MODELS_ORGAN_PULMONARY_LOBAR_PERFUSION_VALIDATION_HPP

#include <mehlissa/models/organ/lung_model_definition.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::organ {

inline constexpr auto pulmonary_lobar_perfusion_validation_schema_version = "1.0.0";

enum class PulmonaryLobarSide : std::uint8_t { left, right };

struct PulmonaryLobarPerfusionValidationSource final {
    std::string id;
    std::string citation;
    std::string url;
    std::string license;
    std::string measurement_method;
    std::string cohort;
    std::size_t sample_size{};
    std::string body_position;
    std::string data_access;
    std::string cohort_independence;
};

struct PulmonaryLobarPerfusionReferenceBed final {
    std::string bed_id;
    PulmonaryLobarSide side{};
    double reported_fraction{};
};

struct PulmonaryLobarPerfusionReferenceSeries final {
    std::string id;
    std::string source_id;
    std::string reconstruction;
    std::string normalization_policy;
    std::vector<PulmonaryLobarPerfusionReferenceBed> beds;
};

struct PulmonaryLobarPerfusionValidationCase final {
    std::string schema_version;
    std::string validation_id;
    std::string title;
    std::string model_definition_id;
    double maximum_absolute_lobar_error_percentage_points{};
    double maximum_rmse_percentage_points{};
    double maximum_right_lung_error_percentage_points{};
    std::vector<PulmonaryLobarPerfusionValidationSource> sources;
    std::vector<PulmonaryLobarPerfusionReferenceSeries> series;
    std::vector<std::string> limitations;
};

struct PulmonaryLobarPerfusionValidationLoadRequest final {
    std::filesystem::path validation_path;
    std::filesystem::path schema_path;
};

struct PulmonaryLobarPerfusionBedResult final {
    std::string bed_id;
    PulmonaryLobarSide side{};
    double reference_reported_fraction{};
    double reference_normalized_fraction{};
    double predicted_fraction{};
    double residual_percentage_points{};
    double absolute_error_percentage_points{};
    bool accepted{};
};

struct PulmonaryLobarPerfusionSeriesResult final {
    std::string series_id;
    double reference_reported_total{};
    double root_mean_square_error_percentage_points{};
    double maximum_absolute_error_percentage_points{};
    double reference_right_lung_fraction{};
    double predicted_right_lung_fraction{};
    double right_lung_error_percentage_points{};
    std::size_t accepted_bed_count{};
    bool accepted{};
    std::vector<PulmonaryLobarPerfusionBedResult> beds;
};

struct PulmonaryLobarPerfusionValidationReport final {
    std::string validation_id;
    std::string model_definition_id;
    bool source_independence_verified{};
    bool all_series_pass{};
    std::size_t series_count{};
    std::size_t accepted_series_count{};
    std::vector<PulmonaryLobarPerfusionSeriesResult> series;
};

[[nodiscard]] PulmonaryLobarPerfusionValidationCase load_pulmonary_lobar_perfusion_validation_case(
    const PulmonaryLobarPerfusionValidationLoadRequest& request);

[[nodiscard]] PulmonaryLobarPerfusionValidationReport evaluate_pulmonary_lobar_perfusion_validation(
    const PulmonaryLobarPerfusionValidationCase& validation,
    const LungModelDefinition& model_definition);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_PULMONARY_LOBAR_PERFUSION_VALIDATION_HPP

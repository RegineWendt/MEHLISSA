// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>
#include <mehlissa/models/organ/pulmonary_zero_dimensional_validation.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string_view>

namespace {

[[nodiscard]] std::filesystem::path root() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::models::organ::LungModelDefinition load_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-rest-supine-0d-v1.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.1.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition load_flow_adaptive_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-rest-exercise-0d-v2.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.2.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition load_age_conditioned_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-rest-exercise-age-0d-v3.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.3.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_invasive_young_resistance_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-rest-exercise-age-invasive-0d-v4.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.3.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_pressure_distensible_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-pressure-distensible-age-0d-v5.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.4.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition load_age_distensible_model_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-pressure-distensible-age-0d-v6.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.5.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::PulmonaryZeroDimensionalValidationCase load_validation() {
    return mehlissa::models::organ::load_pulmonary_zero_dimensional_validation_case({
        root() / "data" / "validation" / "pulmonary-zero-dimensional" /
            "healthy-adult-independent-v1.json",
        root() / "data" / "schemas" / "pulmonary-zero-dimensional-validation" / "1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::PulmonaryZeroDimensionalValidationCase
load_flow_adaptive_validation() {
    return mehlissa::models::organ::load_pulmonary_zero_dimensional_validation_case({
        root() / "data" / "validation" / "pulmonary-zero-dimensional" /
            "healthy-adult-independent-v2.json",
        root() / "data" / "schemas" / "pulmonary-zero-dimensional-validation" / "1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::PulmonaryZeroDimensionalMultipointValidationCase
load_synthetic_multipoint_validation(const bool allow_synthetic = true) {
    return mehlissa::models::organ::load_pulmonary_zero_dimensional_multipoint_validation_case({
        root() / "tests" / "data" / "pulmonary-zero-dimensional-validation" /
            "synthetic-multipoint-v1.json",
        root() / "data" / "schemas" / "pulmonary-zero-dimensional-multipoint-validation" /
            "1.0.0.schema.json",
        allow_synthetic,
    });
}

[[nodiscard]]
mehlissa::models::organ::PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_population_multipoint_validation() {
    return mehlissa::models::organ::
        load_pulmonary_zero_dimensional_population_multipoint_validation_case({
            root() / "data" / "validation" / "pulmonary-zero-dimensional" /
                "healthy-population-multipoint-v1.json",
            root() / "data" / "schemas" /
                "pulmonary-zero-dimensional-population-multipoint-validation" / "1.0.0.schema.json",
        });
}

[[nodiscard]]
mehlissa::models::organ::PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_age_conditioned_population_multipoint_validation() {
    return mehlissa::models::organ::
        load_pulmonary_zero_dimensional_population_multipoint_validation_case({
            root() / "data" / "validation" / "pulmonary-zero-dimensional" /
                "healthy-population-multipoint-v2.json",
            root() / "data" / "schemas" /
                "pulmonary-zero-dimensional-population-multipoint-validation" / "1.1.0.schema.json",
        });
}

[[nodiscard]]
mehlissa::models::organ::PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_invasive_young_resistance_population_multipoint_validation() {
    return mehlissa::models::organ::
        load_pulmonary_zero_dimensional_population_multipoint_validation_case({
            root() / "data" / "validation" / "pulmonary-zero-dimensional" /
                "healthy-population-multipoint-v3.json",
            root() / "data" / "schemas" /
                "pulmonary-zero-dimensional-population-multipoint-validation" / "1.1.0.schema.json",
        });
}

[[nodiscard]]
mehlissa::models::organ::PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_pressure_distensible_population_validation() {
    return mehlissa::models::organ::
        load_pulmonary_zero_dimensional_population_multipoint_validation_case({
            root() / "data" / "validation" / "pulmonary-zero-dimensional" /
                "healthy-pressure-distensible-population-v1.json",
            root() / "data" / "schemas" /
                "pulmonary-zero-dimensional-population-multipoint-validation" / "1.1.0.schema.json",
        });
}

[[nodiscard]]
mehlissa::models::organ::PulmonaryZeroDimensionalPopulationMultipointValidationCase
load_age_distensible_population_validation() {
    return mehlissa::models::organ::
        load_pulmonary_zero_dimensional_population_multipoint_validation_case({
            root() / "data" / "validation" / "pulmonary-zero-dimensional" /
                "healthy-pressure-distensible-population-v2.json",
            root() / "data" / "schemas" /
                "pulmonary-zero-dimensional-population-multipoint-validation" / "1.1.0.schema.json",
        });
}

[[nodiscard]] const mehlissa::models::organ::PulmonaryValidationEndpointResult*
find_endpoint(const mehlissa::models::organ::PulmonaryValidationConditionResult& condition,
              const std::string_view endpoint) {
    for (const auto& candidate : condition.endpoints) {
        if (candidate.endpoint == endpoint) {
            return &candidate;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("Independent pulmonary observations support the resting 0D candidate and expose its "
          "exercise RC limitation",
          "[m3][organ][pulmonary-0d][independent-validation]") {
    const auto validation = load_validation();
    const auto model_definition = load_model_definition();
    const auto report = mehlissa::models::organ::evaluate_pulmonary_zero_dimensional_validation(
        validation, model_definition);

    CHECK(report.source_independence_verified);
    CHECK(report.required_endpoints_pass);
    CHECK(report.required_endpoint_count == 6);
    CHECK(report.accepted_required_endpoint_count == 6);
    CHECK(report.failed_diagnostic_endpoint_count == 1);
    if (report.conditions.size() != 3) {
        FAIL("Expected three independent validation conditions");
        return;
    }

    const auto* supine_pressure =
        find_endpoint(report.conditions[0], "mean_pulmonary_arterial_pressure");
    const auto* invasive_rest_pressure =
        find_endpoint(report.conditions[1], "mean_pulmonary_arterial_pressure");
    const auto* exercise_pressure =
        find_endpoint(report.conditions[2], "mean_pulmonary_arterial_pressure");
    const auto* exercise_rc = find_endpoint(report.conditions[2], "rc_time_constant");
    if (supine_pressure == nullptr || invasive_rest_pressure == nullptr ||
        exercise_pressure == nullptr || exercise_rc == nullptr) {
        FAIL("Expected pressure and RC validation endpoints");
        return;
    }

    CHECK(supine_pressure->absolute_z_score == Catch::Approx(0.375));
    CHECK(invasive_rest_pressure->absolute_z_score == Catch::Approx(0.08));
    CHECK(exercise_pressure->absolute_z_score == Catch::Approx(0.688));
    CHECK(exercise_pressure->accepted);
    CHECK(exercise_rc->absolute_z_score == Catch::Approx(18.5714285714));
    CHECK_FALSE(exercise_rc->accepted);
}

TEST_CASE("Pulmonary validation rejects reuse of a model evidence source",
          "[m3][organ][pulmonary-0d][independent-validation]") {
    auto validation = load_validation();
    const auto model_definition = load_model_definition();
    if (validation.sources.empty() || model_definition.sources.empty()) {
        FAIL("Expected validation and model sources");
        return;
    }
    validation.sources.front().url = model_definition.sources.front().url;

    CHECK_THROWS_AS(mehlissa::models::organ::evaluate_pulmonary_zero_dimensional_validation(
                        validation, model_definition),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Independent Bentley stress data assess the Claessen-calibrated flow adaptation",
          "[m3][organ][pulmonary-0d][independent-validation][exercise]") {
    const auto validation = load_flow_adaptive_validation();
    const auto model_definition = load_flow_adaptive_model_definition();
    const auto report = mehlissa::models::organ::evaluate_pulmonary_zero_dimensional_validation(
        validation, model_definition);

    CHECK(report.source_independence_verified);
    CHECK(report.required_endpoints_pass);
    CHECK(report.required_endpoint_count == 6);
    CHECK(report.accepted_required_endpoint_count == 6);
    CHECK(report.failed_diagnostic_endpoint_count == 1);
    REQUIRE(report.conditions.size() == 3);

    const auto* exercise_pressure =
        find_endpoint(report.conditions[2], "mean_pulmonary_arterial_pressure");
    const auto* exercise_compliance =
        find_endpoint(report.conditions[2], "pulmonary_arterial_compliance");
    const auto* exercise_rc = find_endpoint(report.conditions[2], "rc_time_constant");
    REQUIRE(exercise_pressure != nullptr);
    REQUIRE(exercise_compliance != nullptr);
    REQUIRE(exercise_rc != nullptr);

    CHECK(exercise_pressure->absolute_z_score == Catch::Approx(0.3693733682083));
    CHECK(exercise_compliance->absolute_z_score == Catch::Approx(0.7690925068798));
    CHECK(exercise_rc->absolute_z_score == Catch::Approx(3.0053708756663));
    CHECK(exercise_pressure->accepted);
    CHECK(exercise_compliance->accepted);
    CHECK_FALSE(exercise_rc->accepted);
    CHECK(exercise_rc->absolute_z_score < 18.5714285714);
}

TEST_CASE("Subject-level multipoint evaluation preserves stages and pressure-flow trajectories",
          "[m3][organ][pulmonary-0d][multipoint-validation]") {
    const auto validation = load_synthetic_multipoint_validation();
    const auto model_definition = load_flow_adaptive_model_definition();
    REQUIRE_FALSE(validation.sources.empty());
    CHECK(validation.sources.front().data_access ==
          "Repository test fixture; forbidden as scientific evidence");
    const auto report =
        mehlissa::models::organ::evaluate_pulmonary_zero_dimensional_multipoint_validation(
            validation, model_definition);

    CHECK(report.source_independence_verified);
    CHECK_FALSE(report.measured_evidence);
    CHECK(report.subject_count == 1);
    CHECK(report.stage_count == 3);
    REQUIRE(report.subjects.size() == 1);
    const auto& subject = report.subjects.front();
    REQUIRE(subject.stages.size() == 3);
    CHECK(subject.mean_pressure_error_si == Catch::Approx(-10.0));
    CHECK(subject.root_mean_square_pressure_error_si == Catch::Approx(31.0912635103));
    CHECK(subject.observed_mpap_flow_fit.slope > 0.0);
    CHECK(subject.predicted_mpap_flow_fit.slope > 0.0);
    CHECK(subject.observed_pawp_flow_fit.slope > 0.0);
    CHECK(subject.observed_mpap_flow_fit.coefficient_of_determination > 0.99);
    CHECK(subject.stages.front().pressure_residual_si == Catch::Approx(-20.0));
    CHECK(subject.stages[1].pressure_residual_si == Catch::Approx(30.0));
    CHECK(subject.stages.back().pressure_residual_si == Catch::Approx(-40.0));
    CHECK(subject.stages.front().observed_pulmonary_arterial_compliance_si.has_value());
    CHECK(subject.stages.front().observed_rc_time_constant_si.has_value());
}

TEST_CASE("Synthetic multipoint fixtures cannot be loaded as scientific evidence",
          "[m3][organ][pulmonary-0d][multipoint-validation][provenance]") {
    CHECK_THROWS_AS(load_synthetic_multipoint_validation(false), mehlissa::core::MehlissaError);
}

TEST_CASE("Multipoint evaluation rejects reuse of a calibration source",
          "[m3][organ][pulmonary-0d][multipoint-validation][provenance]") {
    auto validation = load_synthetic_multipoint_validation();
    const auto model_definition = load_flow_adaptive_model_definition();
    REQUIRE_FALSE(validation.sources.empty());
    REQUIRE_FALSE(model_definition.sources.empty());
    validation.sources.front().url = model_definition.sources.front().url;

    CHECK_THROWS_AS(
        mehlissa::models::organ::evaluate_pulmonary_zero_dimensional_multipoint_validation(
            validation, model_definition),
        mehlissa::core::MehlissaError);
}

TEST_CASE("Published population multipoint validation preserves independent series and SI flow",
          "[m3][organ][pulmonary-0d][population-multipoint-validation]") {
    const auto validation = load_population_multipoint_validation();
    const auto model_definition = load_flow_adaptive_model_definition();
    REQUIRE(validation.sources.size() == 2);
    REQUIRE(validation.series.size() == 4);
    CHECK(validation.series.front().sample_size == 193);
    CHECK(validation.series.front().stages.front().cardiac_flow.mean_si ==
          Catch::Approx(7.4 / 60000.0));

    std::size_t wolsk_sample_size{};
    for (const auto& series : validation.series) {
        if (series.source_id == "wolsk-2017-age-hemodynamics") {
            wolsk_sample_size += series.sample_size;
        }
    }
    CHECK(wolsk_sample_size == 62);

    const auto report = mehlissa::models::organ::
        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(validation,
                                                                             model_definition);
    CHECK(report.source_independence_verified);
    CHECK(report.published_population_evidence);
    CHECK(report.series_count == 4);
    CHECK(report.stage_count == 18);
    CHECK(report.accepted_stage_count == 10);
    CHECK(report.all_stages_agree == (report.accepted_stage_count == report.stage_count));
    REQUIRE(report.series.size() == 4);
    CHECK(report.series[0].accepted_stage_count == 3);
    CHECK(report.series[1].accepted_stage_count == 0);
    CHECK(report.series[2].accepted_stage_count == 5);
    CHECK(report.series[3].accepted_stage_count == 2);
    CHECK_FALSE(report.all_stages_agree);
    CHECK_FALSE(report.series[1].stages.front().accepted);
    CHECK(report.series[3].stages[2].accepted);
    CHECK(report.series[3].stages[4].accepted);
    REQUIRE(report.series.front().stages.size() == 3);
    CHECK(report.series.front().stages.front().absolute_z_score.has_value());
    CHECK(report.series.front().stages.front().cardiac_output_si == Catch::Approx(7.4 / 60000.0));
    REQUIRE(report.series[1].stages.size() == 5);
    CHECK_FALSE(report.series[1].stages.front().absolute_z_score.has_value());
    CHECK(report.series[1].stages.front().cardiac_output_si == Catch::Approx(2.9 * 1.9 / 60000.0));
}

TEST_CASE("Population multipoint evaluation rejects reuse of a calibration source",
          "[m3][organ][pulmonary-0d][population-multipoint-validation][provenance]") {
    auto validation = load_population_multipoint_validation();
    const auto model_definition = load_flow_adaptive_model_definition();
    REQUIRE_FALSE(validation.sources.empty());
    REQUIRE_FALSE(model_definition.sources.empty());
    validation.sources.front().url = model_definition.sources.front().url;

    CHECK_THROWS_AS(mehlissa::models::organ::
                        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(
                            validation, model_definition),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Independent age calibration conditions published population series without refitting",
          "[m3][organ][pulmonary-0d][population-multipoint-validation][age]") {
    const auto validation = load_age_conditioned_population_multipoint_validation();
    const auto model_definition = load_age_conditioned_model_definition();
    REQUIRE(validation.series.size() == 4);
    CHECK_FALSE(validation.series[0].representative_age_years.has_value());
    CHECK(validation.series[1].representative_age_years.value_or(0.0) == Catch::Approx(29.5));
    CHECK(validation.series[2].representative_age_years.value_or(0.0) == Catch::Approx(49.5));
    CHECK(validation.series[3].representative_age_years.value_or(0.0) == Catch::Approx(70.0));

    const auto report = mehlissa::models::organ::
        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(validation,
                                                                             model_definition);
    REQUIRE(report.series.size() == 4);
    CHECK(report.source_independence_verified);
    CHECK(report.stage_count == 18);
    CHECK(report.series[0].age_resistance_multiplier == Catch::Approx(1.0));
    CHECK(report.series[1].age_resistance_multiplier == Catch::Approx(0.923348796112216));
    CHECK(report.series[2].age_resistance_multiplier == Catch::Approx(1.0));
    CHECK(report.series[3].age_resistance_multiplier == Catch::Approx(1.12245955546343));
    CHECK(report.accepted_stage_count == 14);
    CHECK(report.series[0].accepted_stage_count == 3);
    CHECK(report.series[1].accepted_stage_count == 1);
    CHECK(report.series[2].accepted_stage_count == 5);
    CHECK(report.series[3].accepted_stage_count == 5);
    CHECK_FALSE(report.all_stages_agree);
}

TEST_CASE("Invasive young resistance resolves the disjoint Wolsk population series without refit",
          "[m3][organ][pulmonary-0d][population-multipoint-validation][age][invasive]") {
    const auto validation = load_invasive_young_resistance_population_multipoint_validation();
    const auto model_definition = load_invasive_young_resistance_model_definition();
    REQUIRE(validation.sources.size() == 1);
    REQUIRE(validation.series.size() == 3);
    CHECK(validation.sources.front().id == "wolsk-2017-age-hemodynamics");

    const auto report = mehlissa::models::organ::
        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(validation,
                                                                             model_definition);
    REQUIRE(report.series.size() == 3);
    CHECK(report.source_independence_verified);
    CHECK(report.stage_count == 15);
    CHECK(report.accepted_stage_count == 15);
    CHECK(report.all_stages_agree);
    CHECK(report.series[0].age_resistance_multiplier == Catch::Approx(0.71875));
    CHECK(report.series[1].age_resistance_multiplier == Catch::Approx(1.0));
    CHECK(report.series[2].age_resistance_multiplier == Catch::Approx(1.12245955546343));
    CHECK(report.series[0].accepted_stage_count == 5);
    CHECK(report.series[1].accepted_stage_count == 5);
    CHECK(report.series[2].accepted_stage_count == 5);
}

TEST_CASE("Fixed healthy distensibility exposes the older-stratum structural limitation",
          "[m3][organ][pulmonary-0d][population-multipoint-validation][distensibility]") {
    const auto validation = load_pressure_distensible_population_validation();
    const auto model_definition = load_pressure_distensible_model_definition();
    const auto report = mehlissa::models::organ::
        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(validation,
                                                                             model_definition);

    REQUIRE(report.series.size() == 3);
    CHECK(report.source_independence_verified);
    CHECK(report.stage_count == 15);
    CHECK(report.accepted_stage_count == 11);
    CHECK_FALSE(report.all_stages_agree);
    CHECK(report.series[0].accepted_stage_count == 5);
    CHECK(report.series[1].accepted_stage_count == 5);
    CHECK(report.series[2].accepted_stage_count == 1);
    CHECK(mehlissa::core::in_millimeters_of_mercury(
              mehlissa::core::pascals(report.series[0].root_mean_square_pressure_error_si)) ==
          Catch::Approx(1.28351281810565));
    CHECK(mehlissa::core::in_millimeters_of_mercury(
              mehlissa::core::pascals(report.series[2].root_mean_square_pressure_error_si)) ==
          Catch::Approx(5.41120283374481));
}

TEST_CASE("Independent older distensibility improves error without fitting Wolsk",
          "[m3][organ][pulmonary-0d][population-multipoint-validation][age]"
          "[distensibility]") {
    const auto validation = load_age_distensible_population_validation();
    const auto model_definition = load_age_distensible_model_definition();
    const auto report = mehlissa::models::organ::
        evaluate_pulmonary_zero_dimensional_population_multipoint_validation(validation,
                                                                             model_definition);

    REQUIRE(report.series.size() == 3);
    CHECK(report.source_independence_verified);
    CHECK(report.stage_count == 15);
    CHECK(report.accepted_stage_count == 11);
    CHECK_FALSE(report.all_stages_agree);
    CHECK(report.series[0].accepted_stage_count == 5);
    CHECK(report.series[1].accepted_stage_count == 5);
    CHECK(report.series[2].accepted_stage_count == 1);
    CHECK(mehlissa::core::in_millimeters_of_mercury(
              mehlissa::core::pascals(report.series[2].root_mean_square_pressure_error_si)) ==
          Catch::Approx(4.60307575340965));
}

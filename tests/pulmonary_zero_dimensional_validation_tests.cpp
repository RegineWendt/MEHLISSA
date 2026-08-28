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

[[nodiscard]] mehlissa::models::organ::PulmonaryZeroDimensionalValidationCase load_validation() {
    return mehlissa::models::organ::load_pulmonary_zero_dimensional_validation_case({
        root() / "data" / "validation" / "pulmonary-zero-dimensional" /
            "healthy-adult-independent-v1.json",
        root() / "data" / "schemas" / "pulmonary-zero-dimensional-validation" / "1.0.0.schema.json",
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

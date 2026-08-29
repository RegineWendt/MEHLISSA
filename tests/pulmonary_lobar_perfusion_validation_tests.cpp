// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>
#include <mehlissa/models/organ/pulmonary_lobar_perfusion_validation.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] mehlissa::models::organ::LungModelDefinition load_definition() {
    return mehlissa::models::organ::load_lung_model_definition({
        root() / "data" / "lung-models" / "healthy-adult-lobar-parallel-0d-v7.json",
        root() / "data" / "schemas" / "lung-model-definition" / "1.6.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::organ::PulmonaryLobarPerfusionValidationCase load_validation() {
    return mehlissa::models::organ::load_pulmonary_lobar_perfusion_validation_case({
        root() / "data" / "validation" / "pulmonary-lobar-perfusion" /
            "healthy-normal-spect-v1.json",
        root() / "data" / "schemas" / "pulmonary-lobar-perfusion-validation" / "1.0.0.schema.json",
    });
}

} // namespace

TEST_CASE("Published normal SPECT values remain traceable and preserve reported rounding",
          "[m3][organ][pulmonary][lobar][validation][provenance]") {
    const auto validation = load_validation();
    REQUIRE(validation.sources.size() == 1);
    REQUIRE(validation.series.size() == 2);
    CHECK(validation.sources.front().sample_size == 73);
    CHECK(validation.sources.front().license == "CC-BY-4.0");
    CHECK(validation.series.front().beds.size() == 5);

    auto reported_total = 0.0;
    for (const auto& bed : validation.series.front().beds) {
        reported_total += bed.reported_fraction;
    }
    CHECK(reported_total == Catch::Approx(1.001));
}

TEST_CASE("Executable five-bed perfusion agrees with both independent SPECT reconstructions",
          "[m3][organ][pulmonary][lobar][validation]") {
    const auto report = mehlissa::models::organ::evaluate_pulmonary_lobar_perfusion_validation(
        load_validation(), load_definition());

    CHECK(report.source_independence_verified);
    CHECK(report.all_series_pass);
    CHECK(report.series_count == 2);
    CHECK(report.accepted_series_count == 2);
    REQUIRE(report.series.size() == 2);
    CHECK(report.series[0].accepted_bed_count == 5);
    CHECK(report.series[1].accepted_bed_count == 5);
    CHECK(report.series[0].root_mean_square_error_percentage_points < 2.0);
    CHECK(report.series[1].root_mean_square_error_percentage_points < 2.0);
    CHECK(report.series[0].right_lung_error_percentage_points < 3.0);
    CHECK(report.series[1].right_lung_error_percentage_points < 3.0);
    CHECK(report.series[0].reference_reported_total == Catch::Approx(1.001));
    CHECK(report.series[0].beds.front().reference_reported_fraction == Catch::Approx(0.222));
    CHECK(report.series[0].beds.front().reference_normalized_fraction ==
          Catch::Approx(0.222 / 1.001));
}

TEST_CASE("Lobar validation rejects calibration-source reuse",
          "[m3][organ][pulmonary][lobar][validation][provenance]") {
    auto validation = load_validation();
    const auto definition = load_definition();
    REQUIRE_FALSE(validation.sources.empty());
    REQUIRE_FALSE(definition.sources.empty());
    validation.sources.front().url = definition.sources.front().url;

    CHECK_THROWS_AS(mehlissa::models::organ::evaluate_pulmonary_lobar_perfusion_validation(
                        validation, definition),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Lobar validation rejects an incomplete reference partition",
          "[m3][organ][pulmonary][lobar][validation][semantics]") {
    auto validation = load_validation();
    REQUIRE_FALSE(validation.series.empty());
    REQUIRE_FALSE(validation.series.front().beds.empty());
    validation.series.front().beds.pop_back();

    CHECK_THROWS_AS(mehlissa::models::organ::evaluate_pulmonary_lobar_perfusion_validation(
                        validation, load_definition()),
                    mehlissa::core::MehlissaError);
}

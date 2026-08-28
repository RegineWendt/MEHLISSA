// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <string_view>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_1(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.1.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_2(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.2.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_3(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.3.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_4(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.4.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_5(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.5.0.schema.json"});
}

[[nodiscard]] mehlissa::models::organ::LungModelDefinition
load_definition_1_6(const std::filesystem::path& path) {
    return mehlissa::models::organ::load_lung_model_definition(
        {path, root() / "data" / "schemas" / "lung-model-definition" / "1.6.0.schema.json"});
}

} // namespace

TEST_CASE("Checked-in lung definitions are executable and evidence scoped",
          "[m3][organ][definition]") {
    const auto compartment =
        load_definition(root() / "examples" / "organ-models" / "lung-compartment-contract-v1.json");
    CHECK(compartment.schema_version == std::string_view{"1.0.0"});
    CHECK(compartment.model.variant ==
          mehlissa::models::organ::LungModelVariant::effective_compartment);
    CHECK(compartment.model.compartment_transit_time == 2s);
    CHECK(compartment.validity.evidence_class == "software_test_surrogate");
    CHECK_FALSE(compartment.external_data.has_value());
    CHECK_FALSE(compartment.sources.empty());
    CHECK_FALSE(compartment.limitations.empty());
    const auto compartment_model = mehlissa::models::organ::make_lung_model(compartment.model);
    CHECK(compartment_model->model_id() == std::string_view{"lung.compartment.contract.v1"});

    const auto regional =
        load_definition(root() / "examples" / "organ-models" / "lung-regional-contract-v1.json");
    CHECK(regional.model.variant ==
          mehlissa::models::organ::LungModelVariant::regional_circulation);
    REQUIRE(regional.model.regions.size() == 3);
    CHECK(regional.model.regions[0].transit_time == 500ms);
    CHECK(regional.model.regions[1].transit_time == 1s);
    CHECK(regional.model.regions[2].transit_time == 500ms);
    const auto regional_model = mehlissa::models::organ::make_lung_model(regional.model);
    CHECK(regional_model->model_id() == std::string_view{"lung.regional.contract.v1"});
}

TEST_CASE("A definition cannot mix variant timing structures", "[m3][organ][definition]") {
    CHECK_THROWS_AS(
        load_definition(root() / "tests" / "data" / "organ-models" / "invalid-mixed-variant.json"),
        mehlissa::core::MehlissaError);
}

TEST_CASE("External pulmonary data metadata preserves provenance axes and units",
          "[m3][organ][definition]") {
    const auto definition = load_definition(root() / "tests" / "data" / "organ-models" /
                                            "external-data-reference.json");
    if (!definition.external_data.has_value()) {
        FAIL("Expected external pulmonary data metadata");
        return;
    }
    const auto& external = *definition.external_data;
    CHECK(external.source_id == "external-test-source");
    CHECK(external.path == std::filesystem::path{"fixtures/pulmonary-reference.vtp"});
    CHECK(external.sha256.size() == 64);
    CHECK(external.coordinate_system == "right-handed Cartesian, source axes retained");
    CHECK(external.length_unit == "m");
    CHECK(external.flow_unit == "m3/s");
    REQUIRE(external.transformations.size() == 1);
}

TEST_CASE("Literature-parameterized pulmonary 0D definition preserves evidence and executes",
          "[m3][organ][definition][physiology]") {
    const auto definition = load_definition_1_1(root() / "data" / "lung-models" /
                                                "healthy-adult-rest-supine-0d-v1.json");

    CHECK(definition.schema_version == std::string_view{"1.1.0"});
    CHECK(definition.model.variant ==
          mehlissa::models::organ::LungModelVariant::pulmonary_zero_dimensional);
    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value()) {
        FAIL("Expected pulmonary 0D parameters and hemodynamic evidence");
        return;
    }
    const auto& parameters = definition.model.zero_dimensional_parameters.value();
    const auto& hemodynamics = definition.hemodynamics.value();
    CHECK(mehlissa::core::in_liters_per_minute(parameters.baseline_cardiac_output) ==
          Catch::Approx(6.0));
    CHECK(parameters.pulmonary_transit_time == 6400ms);
    CHECK(hemodynamics.mean_pulmonary_arterial_pressure_target.role == "derived");
    CHECK(hemodynamics.right_lung_perfusion_fraction.uncertainty.kind ==
          "propagated_standard_deviation");
    CHECK(hemodynamics.pulmonary_transit_time.uncertainty.kind == "interquartile_range");
    CHECK(hemodynamics.right_lung_perfusion_fraction.uncertainty.lower_si.has_value());

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() == std::string_view{"lung.pulmonary-0d.healthy-adult-rest-supine.v1"});
}

TEST_CASE("Flow-adaptive pulmonary 0D definition preserves independent calibration evidence",
          "[m3][organ][definition][physiology][exercise]") {
    const auto definition = load_definition_1_2(root() / "data" / "lung-models" /
                                                "healthy-adult-rest-exercise-0d-v2.json");

    CHECK(definition.schema_version == std::string_view{"1.2.0"});
    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value() ||
        !definition.model.zero_dimensional_parameters->flow_adaptation.has_value() ||
        !definition.hemodynamics->flow_adaptation.has_value()) {
        FAIL("Expected executable flow adaptation and its evidence");
        return;
    }

    const auto& adaptation = definition.model.zero_dimensional_parameters->flow_adaptation.value();
    const auto& evidence = definition.hemodynamics->flow_adaptation.value();
    CHECK(mehlissa::core::in_liters_per_minute(adaptation.reference_cardiac_output) ==
          Catch::Approx(6.0));
    CHECK(adaptation.resistance_exponent == Catch::Approx(-0.202148574863478));
    CHECK(adaptation.compliance_exponent == Catch::Approx(-0.669350145160558));
    CHECK(adaptation.maximum_flow_ratio.si_value() == Catch::Approx(16.2 / 6.2));
    CHECK(evidence.resistance_exponent.source_id == "claessen-2015");
    CHECK(evidence.compliance_exponent.uncertainty.kind == "not_propagated");

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() ==
          std::string_view{"lung.pulmonary-0d.healthy-adult-rest-exercise.v2"});
}

TEST_CASE("Age-conditioned pulmonary 0D definition preserves separate calibration evidence",
          "[m3][organ][definition][physiology][age]") {
    const auto definition = load_definition_1_3(root() / "data" / "lung-models" /
                                                "healthy-adult-rest-exercise-age-0d-v3.json");

    CHECK(definition.schema_version == std::string_view{"1.3.0"});
    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value() ||
        !definition.model.zero_dimensional_parameters->age_conditioning.has_value() ||
        !definition.hemodynamics->age_conditioning.has_value()) {
        FAIL("Expected executable age conditioning and its evidence");
        return;
    }

    const auto& conditioning =
        definition.model.zero_dimensional_parameters->age_conditioning.value();
    const auto& evidence = definition.hemodynamics->age_conditioning.value();
    CHECK(conditioning.age_years == Catch::Approx(55.0));
    CHECK(conditioning.minimum_supported_age_years == Catch::Approx(18.0));
    CHECK(conditioning.maximum_supported_age_years == Catch::Approx(85.0));
    CHECK(conditioning.young_resistance_multiplier.si_value() == Catch::Approx(0.923348796112216));
    CHECK(conditioning.older_resistance_multiplier.si_value() == Catch::Approx(1.12245955546343));
    CHECK(evidence.young_resistance_multiplier.source_id == "kane-2016");
    CHECK(evidence.older_resistance_multiplier.role == "derived");

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() ==
          std::string_view{"lung.pulmonary-0d.healthy-adult-rest-exercise-age.v3"});
}

TEST_CASE("Invasive young-resistance pulmonary definition narrows its evidence scope",
          "[m3][organ][definition][physiology][age][invasive]") {
    const auto definition = load_definition_1_3(
        root() / "data" / "lung-models" / "healthy-adult-rest-exercise-age-invasive-0d-v4.json");

    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value() ||
        !definition.model.zero_dimensional_parameters->age_conditioning.has_value() ||
        !definition.hemodynamics->age_conditioning.has_value()) {
        FAIL("Expected executable invasive age conditioning and its evidence");
        return;
    }

    const auto& conditioning =
        definition.model.zero_dimensional_parameters->age_conditioning.value();
    const auto& evidence = definition.hemodynamics->age_conditioning.value();
    CHECK(conditioning.minimum_supported_age_years == Catch::Approx(24.0));
    CHECK(conditioning.young_resistance_multiplier.si_value() == Catch::Approx(0.71875));
    CHECK(conditioning.older_resistance_multiplier.si_value() == Catch::Approx(1.12245955546343));
    CHECK(evidence.young_resistance_multiplier.source_id == "kovacs-2012-pvr");
    CHECK(evidence.older_resistance_multiplier.source_id == "kane-2016");

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() ==
          std::string_view{"lung.pulmonary-0d.healthy-adult-rest-exercise-age-invasive.v4"});

    auto below_supported_age = definition.model;
    below_supported_age.zero_dimensional_parameters->age_conditioning->age_years = 23.9;
    CHECK_THROWS_AS(mehlissa::models::organ::make_lung_model(below_supported_age),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Pressure-distensible definition binds the Linehan equation to human alpha evidence",
          "[m3][organ][definition][physiology][distensibility]") {
    const auto definition = load_definition_1_4(
        root() / "data" / "lung-models" / "healthy-adult-pressure-distensible-age-0d-v5.json");

    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value() ||
        !definition.model.zero_dimensional_parameters->pressure_distensibility.has_value() ||
        !definition.hemodynamics->pressure_distensibility.has_value()) {
        FAIL("Expected executable pressure distensibility and its evidence");
        return;
    }
    const auto& parameters =
        definition.model.zero_dimensional_parameters->pressure_distensibility.value();
    const auto& evidence = definition.hemodynamics->pressure_distensibility.value();
    CHECK(definition.schema_version == std::string_view{"1.4.0"});
    CHECK(mehlissa::core::in_liters_per_minute(parameters.reference_cardiac_output) ==
          Catch::Approx(6.0));
    CHECK(mehlissa::core::in_millimeters_of_mercury(parameters.reference_left_atrial_pressure) ==
          Catch::Approx(8.0));
    CHECK(mehlissa::core::in_per_millimeter_of_mercury(parameters.coefficient) ==
          Catch::Approx(0.02));
    CHECK(evidence.coefficient.source_id == "reeves-2005");
    CHECK_FALSE(parameters.older_coefficient.has_value());
    CHECK_FALSE(evidence.older_coefficient.has_value());
    CHECK_FALSE(definition.model.zero_dimensional_parameters->flow_adaptation.has_value());

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() ==
          std::string_view{"lung.pulmonary-0d.healthy-adult-pressure-distensible-age.v5"});
}

TEST_CASE("Age-conditioned distensibility binds the older invasive aggregate",
          "[m3][organ][definition][physiology][age][distensibility]") {
    const auto definition = load_definition_1_5(
        root() / "data" / "lung-models" / "healthy-adult-pressure-distensible-age-0d-v6.json");

    if (!definition.model.zero_dimensional_parameters.has_value() ||
        !definition.hemodynamics.has_value() ||
        !definition.model.zero_dimensional_parameters->pressure_distensibility.has_value() ||
        !definition.model.zero_dimensional_parameters->age_conditioning.has_value() ||
        !definition.hemodynamics->pressure_distensibility.has_value() ||
        !definition.model.zero_dimensional_parameters->pressure_distensibility->older_coefficient
             .has_value() ||
        !definition.hemodynamics->pressure_distensibility->older_coefficient.has_value()) {
        FAIL("Expected age-conditioned pressure distensibility and its evidence");
        return;
    }
    const auto& parameters =
        definition.model.zero_dimensional_parameters->pressure_distensibility.value();
    const auto& evidence = definition.hemodynamics->pressure_distensibility.value();
    CHECK(definition.schema_version == std::string_view{"1.5.0"});
    CHECK(mehlissa::core::in_per_millimeter_of_mercury(*parameters.older_coefficient) ==
          Catch::Approx(0.015));
    CHECK(evidence.older_coefficient->source_id == "reeves-2005");
    CHECK(evidence.older_coefficient->uncertainty.kind == "standard_error");
    CHECK(definition.model.zero_dimensional_parameters->age_conditioning->older_lower_age_years ==
          Catch::Approx(60.0));

    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(model->model_id() ==
          std::string_view{"lung.pulmonary-0d.healthy-adult-pressure-distensible-age.v6"});
}

TEST_CASE("Lobar parallel definition binds five evidence-qualified executable beds",
          "[m3][organ][definition][physiology][parallel]") {
    const auto definition = load_definition_1_6(root() / "data" / "lung-models" /
                                                "healthy-adult-lobar-parallel-0d-v7.json");
    CHECK(definition.schema_version == std::string_view{"1.6.0"});
    REQUIRE(definition.model.zero_dimensional_parameters.has_value());
    REQUIRE(definition.hemodynamics.has_value());
    const auto& parameters = definition.model.zero_dimensional_parameters.value().parallel_beds;
    const auto& evidence = definition.hemodynamics.value().parallel_beds;
    REQUIRE(parameters.size() == 5);
    REQUIRE(evidence.size() == parameters.size());
    CHECK(parameters[1].id == "right-middle-lobe");
    CHECK(parameters[1].perfusion_fraction.si_value() == Catch::Approx(0.087833145238301935));
    CHECK(evidence[1].perfusion_fraction.source_id == "lee-2022");
    CHECK(evidence[1].transit_time.source_id == "swift-2012");
    const auto model = mehlissa::models::organ::make_lung_model(definition.model);
    CHECK(dynamic_cast<mehlissa::models::organ::PulmonaryParallelBedsModel*>(model.get()) !=
          nullptr);
}

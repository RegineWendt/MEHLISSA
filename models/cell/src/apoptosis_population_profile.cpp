// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/apoptosis_population_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::cell {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot open " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid, "Invalid JSON in " + std::string{role} + " '" +
                                                   path.string() + "': " + error.what());
    }
}

[[nodiscard]] ApoptosisPopulationConfig decode_model(const Json& model) {
    return {model.at("model_id").as<std::string>(),
            model.at("population_id").as<std::string>(),
            model.at("drug_id").as<std::string>(),
            core::moles(model.at("half_max_effect_amount_mol").as<double>()),
            model.at("hill_coefficient").as<double>(),
            model.at("apoptosis_commitment_threshold").as<double>()};
}

[[nodiscard]] ApoptosisPopulationProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& reference = document.at("reference_case");
    const auto& expected = reference.at("expected");
    const auto& validity = document.at("validity");
    ApoptosisPopulationProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("implementation_kind").as<std::string>(),
        decode_model(document.at("model")),
        {reference.at("request_id").as<std::string>(),
         std::chrono::duration_cast<core::SimulationClock::Duration>(
             std::chrono::duration<double>{reference.at("observed_at_s").as<double>()}),
         reference.at("maximum_reported_cohorts").as<std::size_t>(),
         {}},
        {expected.at("total_cells").as<std::uint64_t>(),
         expected.at("viable_cells").as<std::uint64_t>(),
         expected.at("apoptosis_committed_cells").as<std::uint64_t>(),
         expected.at("apoptosis_committed_fraction").as<double>(),
         expected.at("cell_weighted_mean_effect_fraction").as<double>(),
         expected.at("fraction_tolerance").as<double>()},
        {},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};
    for (const auto& cohort : reference.at("cohorts").array_range()) {
        result.reference_request.cohorts.push_back(
            {cohort.at("cohort_id").as<std::string>(), cohort.at("cell_count").as<std::uint64_t>(),
             core::moles(cohort.at("intracellular_drug_amount_mol").as<double>())});
    }
    for (const auto& sensitivity : document.at("sensitivity_cases").array_range()) {
        result.sensitivity_cases.push_back(
            {sensitivity.at("case_id").as<std::string>(),
             core::moles(sensitivity.at("half_max_effect_amount_mol").as<double>()),
             sensitivity.at("hill_coefficient").as<double>(),
             sensitivity.at("apoptosis_commitment_threshold").as<double>(),
             sensitivity.at("expected_apoptosis_committed_fraction").as<double>(),
             sensitivity.at("expected_mean_effect_fraction").as<double>()});
    }
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

[[nodiscard]] bool close(const double left, const double right, const double tolerance) noexcept {
    return std::abs(left - right) <= tolerance;
}

} // namespace

void validate_apoptosis_population_profile(const ApoptosisPopulationProfile& profile) {
    if (profile.schema_version != apoptosis_population_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != cohort_compressed_apoptosis_population_kind ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty() || profile.sensitivity_cases.empty() ||
        !std::isfinite(profile.expected.fraction_tolerance) ||
        profile.expected.fraction_tolerance <= 0.0) {
        invalid(core::ErrorCode::data_invalid,
                "Apoptosis-population profile is incomplete or inconsistent");
    }

    const CohortCompressedApoptosisPopulationModel model{profile.model};
    const auto response = model.evaluate(profile.reference_request);
    if (response.total_cells != profile.expected.total_cells ||
        response.viable_cells != profile.expected.viable_cells ||
        response.apoptosis_committed_cells != profile.expected.apoptosis_committed_cells ||
        !close(response.apoptosis_committed_fraction, profile.expected.apoptosis_committed_fraction,
               profile.expected.fraction_tolerance) ||
        !close(response.cell_weighted_mean_effect_fraction,
               profile.expected.cell_weighted_mean_effect_fraction,
               profile.expected.fraction_tolerance)) {
        invalid(core::ErrorCode::data_invalid,
                "Apoptosis-population reference values do not match the model");
    }

    std::unordered_set<std::string> case_ids;
    for (const auto& sensitivity : profile.sensitivity_cases) {
        if (sensitivity.case_id.empty() || !case_ids.insert(sensitivity.case_id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-population sensitivity cases must be named uniquely");
        }
        auto config = profile.model;
        config.half_max_effect_amount = sensitivity.half_max_effect_amount;
        config.hill_coefficient = sensitivity.hill_coefficient;
        config.apoptosis_commitment_threshold = sensitivity.apoptosis_commitment_threshold;
        const auto result =
            CohortCompressedApoptosisPopulationModel{config}.evaluate(profile.reference_request);
        if (!close(result.apoptosis_committed_fraction,
                   sensitivity.expected_apoptosis_committed_fraction,
                   profile.expected.fraction_tolerance) ||
            !close(result.cell_weighted_mean_effect_fraction,
                   sensitivity.expected_mean_effect_fraction,
                   profile.expected.fraction_tolerance)) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-population sensitivity reference does not match the model");
        }
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-population sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-population limitations must not be empty");
        }
    }
}

ApoptosisPopulationProfile
load_apoptosis_population_profile(const ApoptosisPopulationProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "apoptosis-population schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "apoptosis-population profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_apoptosis_population_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Apoptosis-population profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::cell

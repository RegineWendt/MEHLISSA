// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::capillary {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

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

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::schema_invalid, "Invalid capillary-entity-observation schema '" +
                                                     path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-entity-observation profile does not satisfy its schema '" +
                    path.string() + "': " + error.what());
    }
}

[[nodiscard]] CapillaryEntityObservationProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& validity = document.at("validity");
    CapillaryEntityObservationProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("compatible_model_id").as<std::string>(),
        document.at("unmatched_entity_policy").as<std::string>(),
        document.at("maximum_buffered_records").as<std::size_t>(),
        {},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& rule : document.at("entity_rules").array_range()) {
        result.entity_rules.push_back({rule.at("entity_type").as<std::string>(),
                                       rule.at("retention_rate_per_second").as<double>(),
                                       rule.at("adhesion_rate_per_second").as<double>(),
                                       rule.at("extravasation_rate_per_second").as<double>()});
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

[[nodiscard]] bool valid_rate(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0;
}

} // namespace

void validate_capillary_entity_observation_profile(
    const CapillaryEntityObservationProfile& profile) {
    if (profile.schema_version != capillary_entity_observation_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.compatible_model_id.empty() ||
        profile.unmatched_entity_policy != "observe_residence_only" ||
        profile.maximum_buffered_records == 0 || profile.entity_rules.empty() ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-entity-observation profile metadata is incomplete or invalid");
    }
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-entity-observation evidence class is unsupported");
    }

    std::unordered_set<std::string> entity_types;
    for (const auto& rule : profile.entity_rules) {
        const auto total_rate = rule.retention_rate_per_second + rule.adhesion_rate_per_second +
                                rule.extravasation_rate_per_second;
        if (rule.entity_type.empty() || !entity_types.insert(rule.entity_type).second ||
            !valid_rate(rule.retention_rate_per_second) ||
            !valid_rate(rule.adhesion_rate_per_second) ||
            !valid_rate(rule.extravasation_rate_per_second) || !std::isfinite(total_rate) ||
            total_rate <= 0.0) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary entity rules require unique entity types and a positive finite "
                    "combined interaction rate");
        }
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-entity-observation source metadata must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-entity-observation limitations must not be empty");
        }
    }
}

CapillaryEntityObservationProfile load_capillary_entity_observation_profile(
    const CapillaryEntityObservationProfileLoadRequest& request) {
    const auto schema_document =
        read_json(request.schema_path, "capillary-entity-observation schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document =
        read_json(request.profile_path, "capillary-entity-observation profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_capillary_entity_observation_profile(profile);
    return profile;
}

} // namespace mehlissa::models::capillary

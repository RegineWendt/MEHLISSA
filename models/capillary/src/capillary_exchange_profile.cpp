// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_exchange_profile.hpp>

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
        invalid(core::ErrorCode::schema_invalid,
                "Invalid capillary-exchange schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-exchange profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] CapillaryExchangeProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& validity = document.at("validity");
    CapillaryExchangeProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("compatible_model_id").as<std::string>(),
        document.at("unmatched_substance_policy").as<std::string>(),
        {},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& rule : document.at("substance_rules").array_range()) {
        result.substance_rules.push_back(
            {rule.at("substance_id").as<std::string>(),
             rule.at("blood_to_endothelium_fraction").as<double>(),
             rule.at("endothelium_to_interstitium_fraction").as<double>(),
             rule.at("interstitium_to_cell_fraction").as<double>()});
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

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

void validate_capillary_exchange_profile(const CapillaryExchangeProfile& profile) {
    if (profile.schema_version != capillary_exchange_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.compatible_model_id.empty() ||
        profile.unmatched_substance_policy != "pass_through" || profile.substance_rules.empty() ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-exchange profile metadata is incomplete or invalid");
    }
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid, "Capillary-exchange evidence class is unsupported");
    }

    std::unordered_set<std::string> substance_ids;
    for (const auto& rule : profile.substance_rules) {
        if (rule.substance_id.empty() || !substance_ids.insert(rule.substance_id).second ||
            !valid_fraction(rule.blood_to_endothelium_fraction) ||
            rule.blood_to_endothelium_fraction >= 1.0 ||
            !valid_fraction(rule.endothelium_to_interstitium_fraction) ||
            !valid_fraction(rule.interstitium_to_cell_fraction)) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-exchange rules require unique substances and valid staged "
                    "fractions; some amount must remain in blood");
        }
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-exchange source metadata must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-exchange limitations must not be empty");
        }
    }
}

CapillaryExchangeProfile
load_capillary_exchange_profile(const CapillaryExchangeProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "capillary-exchange schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "capillary-exchange profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_capillary_exchange_profile(profile);
    return profile;
}

} // namespace mehlissa::models::capillary

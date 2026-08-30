// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_entity_disposition_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

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
        invalid(core::ErrorCode::schema_invalid, "Invalid capillary-entity-disposition schema '" +
                                                     path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-entity-disposition profile does not satisfy its schema '" +
                    path.string() + "': " + error.what());
    }
}

[[nodiscard]] EntityDispositionTarget decode_target(const Json& target) {
    return {target.at("model_id").as<std::string>(), target.at("compartment_id").as<std::string>()};
}

[[nodiscard]] CapillaryEntityDispositionProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& targets = document.at("targets");
    const auto& validity = document.at("validity");
    CapillaryEntityDispositionProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("compatible_model_id").as<std::string>(),
        document.at("compatible_observation_profile_id").as<std::string>(),
        document.at("random_stream_name").as<std::string>(),
        document.at("source_port_id").as<std::string>(),
        decode_target(targets.at("retained")),
        decode_target(targets.at("adhered")),
        decode_target(targets.at("extravasated")),
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
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

[[nodiscard]] bool valid_target(const EntityDispositionTarget& target) noexcept {
    return !target.model_id.empty() && !target.compartment_id.empty();
}

} // namespace

void validate_capillary_entity_disposition_profile(
    const CapillaryEntityDispositionProfile& profile) {
    if (profile.schema_version != capillary_entity_disposition_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.compatible_model_id.empty() || profile.compatible_observation_profile_id.empty() ||
        profile.random_stream_name.empty() || profile.source_port_id.empty() ||
        !valid_target(profile.retention_target) || !valid_target(profile.adhesion_target) ||
        !valid_target(profile.extravasation_target) || profile.validity.population.empty() ||
        profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-entity-disposition profile metadata or targets are invalid");
    }
    if (profile.retention_target.model_id != profile.adhesion_target.model_id ||
        profile.retention_target.model_id != profile.extravasation_target.model_id ||
        profile.retention_target.compartment_id == profile.adhesion_target.compartment_id ||
        profile.retention_target.compartment_id == profile.extravasation_target.compartment_id ||
        profile.adhesion_target.compartment_id == profile.extravasation_target.compartment_id) {
        invalid(core::ErrorCode::data_invalid,
                "Disposition targets require one owner model and distinct compartments");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-entity-disposition sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-entity-disposition limitations must not be empty");
        }
    }
}

CapillaryEntityDispositionProfile load_capillary_entity_disposition_profile(
    const CapillaryEntityDispositionProfileLoadRequest& request) {
    const auto schema_document =
        read_json(request.schema_path, "capillary-entity-disposition schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document =
        read_json(request.profile_path, "capillary-entity-disposition profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_capillary_entity_disposition_profile(profile);
    return profile;
}

const EntityDispositionTarget&
disposition_target(const CapillaryEntityDispositionProfile& profile,
                   const coupling::EntityDispositionKind kind) noexcept {
    switch (kind) {
    case coupling::EntityDispositionKind::retained:
        return profile.retention_target;
    case coupling::EntityDispositionKind::adhered:
        return profile.adhesion_target;
    case coupling::EntityDispositionKind::extravasated:
        return profile.extravasation_target;
    }
    return profile.retention_target;
}

} // namespace mehlissa::models::capillary

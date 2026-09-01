// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/active_gateway_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <fstream>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::iot {
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

[[nodiscard]] LocalMessageKind decode_kind(const std::string_view value) {
    if (value == "detection") {
        return LocalMessageKind::detection;
    }
    if (value == "measurement") {
        return LocalMessageKind::measurement;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown active-gateway uplink kind");
}

[[nodiscard]] ActiveGatewayProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& gateway = document.at("gateway");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    ActiveGatewayProfile result{document.at("schema_version").as<std::string>(),
                                identity.at("id").as<std::string>(),
                                identity.at("version").as<std::string>(),
                                identity.at("title").as<std::string>(),
                                {gateway.at("id").as<std::string>(),
                                 gateway.at("endpoint_profile_id").as<std::string>(),
                                 gateway.at("measurement_id_prefix").as<std::string>(),
                                 gateway.at("command_message_id_prefix").as<std::string>(),
                                 {},
                                 gateway.at("maximum_measurements").as<std::uint64_t>(),
                                 gateway.at("maximum_commands").as<std::uint64_t>()},
                                {reference.at("uplink_source_device_id").as<std::string>(),
                                 reference.at("downlink_target_device_id").as<std::string>(),
                                 reference.at("command_id").as<std::string>()},
                                {validity.at("population").as<std::string>(),
                                 validity.at("physiological_state").as<std::string>(),
                                 validity.at("evidence_class").as<std::string>(),
                                 validity.at("description").as<std::string>()},
                                {},
                                {}};

    for (const auto& kind : gateway.at("accepted_uplink_kinds").array_range()) {
        result.gateway.accepted_uplink_kinds.push_back(decode_kind(kind.as<std::string_view>()));
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

} // namespace

void validate_active_gateway_profile(const ActiveGatewayProfile& profile) {
    validate_active_gateway_config(profile.gateway);
    if (profile.schema_version != active_gateway_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.reference_case.uplink_source_device_id.empty() ||
        profile.reference_case.downlink_target_device_id.empty() ||
        profile.reference_case.command_id.empty() ||
        profile.reference_case.uplink_source_device_id == profile.gateway.gateway_id ||
        profile.reference_case.downlink_target_device_id == profile.gateway.gateway_id ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Active-gateway profile is incomplete or internally inconsistent");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Active-gateway sources must be complete and unique");
        }
    }
    if (std::ranges::any_of(profile.limitations,
                            [](const auto& limitation) { return limitation.empty(); })) {
        invalid(core::ErrorCode::data_invalid, "Active-gateway limitations must not be empty");
    }
}

ActiveGatewayProfile load_active_gateway_profile(const ActiveGatewayProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "active-gateway schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "active-gateway profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_active_gateway_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Active-gateway profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

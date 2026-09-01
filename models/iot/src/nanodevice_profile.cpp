// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <fstream>
#include <string>
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

[[nodiscard]] NanodeviceCapability decode_capability(const std::string_view value) {
    if (value == "sense") {
        return NanodeviceCapability::sense;
    }
    if (value == "transmit") {
        return NanodeviceCapability::transmit;
    }
    if (value == "receive") {
        return NanodeviceCapability::receive;
    }
    if (value == "relay") {
        return NanodeviceCapability::relay;
    }
    if (value == "collect") {
        return NanodeviceCapability::collect;
    }
    if (value == "actuate") {
        return NanodeviceCapability::actuate;
    }
    if (value == "release_payload") {
        return NanodeviceCapability::release_payload;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown nanodevice capability");
}

[[nodiscard]] NanodeviceLifecycleState decode_state(const std::string_view value) {
    if (value == "dormant") {
        return NanodeviceLifecycleState::dormant;
    }
    if (value == "active") {
        return NanodeviceLifecycleState::active;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown initial nanodevice state");
}

[[nodiscard]] NanodevicePayload decode_payload(const Json& value) {
    NanodevicePayload payload{value.at("id").as<std::string>(), value.at("type").as<std::string>(),
                              std::nullopt, std::nullopt};
    if (value.contains("amount_mol")) {
        payload.amount = core::moles(value.at("amount_mol").as<double>());
    }
    if (value.contains("unit_count")) {
        payload.unit_count = value.at("unit_count").as<std::uint64_t>();
    }
    return payload;
}

[[nodiscard]] NanodeviceProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& device = document.at("device");
    const auto& target = device.at("target");
    const auto& resources = device.at("resources");
    const auto& validity = document.at("validity");
    NanodeviceProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {device.at("id").as<std::string>(),
         device.at("type").as<std::string>(),
         decode_state(device.at("initial_state").as<std::string>()),
         {target.at("kind").as<std::string>(), target.at("id").as<std::string>()},
         {},
         {},
         {core::joules(resources.at("initial_energy_j").as<double>()),
          core::joules(resources.at("transmit_energy_per_message_j").as<double>()),
          core::joules(resources.at("receive_energy_per_message_j").as<double>()),
          resources.at("maximum_message_size_bytes").as<std::uint64_t>(),
          resources.at("message_storage_capacity_bytes").as<std::uint64_t>(),
          resources.at("maximum_transmissions").as<std::uint64_t>(),
          resources.at("maximum_receptions").as<std::uint64_t>()}},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};

    for (const auto& capability : device.at("capabilities").array_range()) {
        result.device.capabilities.push_back(decode_capability(capability.as<std::string_view>()));
    }
    for (const auto& payload : device.at("payloads").array_range()) {
        result.device.payloads.push_back(decode_payload(payload));
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

void validate_nanodevice_profile(const NanodeviceProfile& profile) {
    validate_nanodevice_config(profile.device);
    if (profile.schema_version != nanodevice_profile_schema_version || profile.profile_id.empty() ||
        profile.profile_version.empty() || profile.title.empty() ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Nanodevice profile is incomplete or uses an unsupported schema");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Nanodevice sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid, "Nanodevice limitations must not be empty");
        }
    }
}

NanodeviceProfile load_nanodevice_profile(const NanodeviceProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "nanodevice schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "nanodevice profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_nanodevice_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Nanodevice profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

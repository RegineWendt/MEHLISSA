// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/local_communication_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
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

[[nodiscard]] core::SimulationClock::Duration seconds(const double value) {
    const auto maximum = std::chrono::duration<double>{core::SimulationClock::Duration::max()};
    if (!std::isfinite(value) || value < 0.0 || value > maximum.count()) {
        invalid(core::ErrorCode::numeric_overflow,
                "Local-communication duration is outside the simulation clock range");
    }
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] ScheduledLinkOutcome decode_outcome(const std::string_view value) {
    if (value == "delivered") {
        return ScheduledLinkOutcome::delivered;
    }
    if (value == "lost") {
        return ScheduledLinkOutcome::lost;
    }
    if (value == "corrupted") {
        return ScheduledLinkOutcome::corrupted;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown scheduled one-hop outcome");
}

[[nodiscard]] LocalCommunicationProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& adapter = document.at("detection_adapter");
    const auto& link = document.at("link");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    LocalCommunicationProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {adapter.at("id").as<std::string>(), adapter.at("source_device_id").as<std::string>(),
         adapter.at("target_device_id").as<std::string>(),
         adapter.at("message_id_prefix").as<std::string>(),
         adapter.at("correlation_id").as<std::string>(),
         seconds(adapter.at("valid_for_seconds").as<double>()),
         adapter.at("hop_limit").as<std::uint32_t>(), adapter.at("size_bytes").as<std::uint64_t>(),
         adapter.at("content_type").as<std::string>()},
        {link.at("id").as<std::string>(),
         seconds(link.at("latency_seconds").as<double>()),
         core::joules(link.at("energy_per_attempt_j").as<double>()),
         {}},
        {reference.at("source_profile_id").as<std::string>(),
         reference.at("event_id").as<std::string>(),
         decode_outcome(reference.at("expected_outcome").as<std::string>()),
         seconds(reference.at("expected_latency_seconds").as<double>()),
         core::joules(reference.at("expected_link_energy_j").as<double>())},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};

    for (const auto& outcome : link.at("repeating_outcomes").array_range()) {
        result.link.repeating_outcomes.push_back(decode_outcome(outcome.as<std::string_view>()));
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

void validate_local_communication_profile(const LocalCommunicationProfile& profile) {
    validate_detection_message_adapter_config(profile.detection_adapter);
    validate_scheduled_one_hop_link_config(profile.link);
    if (profile.schema_version != local_communication_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.reference_case.source_profile_id.empty() ||
        profile.reference_case.event_id.empty() ||
        profile.reference_case.expected_outcome != profile.link.repeating_outcomes.front() ||
        profile.reference_case.expected_latency != profile.link.latency ||
        core::in_joules(profile.reference_case.expected_link_energy) !=
            core::in_joules(profile.link.energy_per_attempt) ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Local-communication profile is incomplete or internally inconsistent");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Local-communication sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Local-communication limitations must not be empty");
        }
    }
}

LocalCommunicationProfile
load_local_communication_profile(const LocalCommunicationProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "local-communication schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "local-communication profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_local_communication_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Local-communication profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

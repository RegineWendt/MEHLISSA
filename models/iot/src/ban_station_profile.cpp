// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/ban_station_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
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

[[nodiscard]] core::SimulationClock::Duration seconds(const double value) {
    const auto maximum = std::chrono::duration<double>{core::SimulationClock::Duration::max()};
    if (!std::isfinite(value) || value < 0.0 || value > maximum.count()) {
        invalid(core::ErrorCode::numeric_overflow,
                "BAN-station duration is outside the simulation clock range");
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
    invalid(core::ErrorCode::data_invalid, "Unknown scheduled BAN outcome");
}

[[nodiscard]] ScheduledBanTransportConfig decode_transport(const Json& value) {
    ScheduledBanTransportConfig result{
        value.at("id").as<std::string>(),
        seconds(value.at("latency_seconds").as<double>()),
        core::joules(value.at("transmitter_energy_per_attempt_j").as<double>()),
        core::joules(value.at("receiver_energy_per_delivery_j").as<double>()),
        core::joules(value.at("link_energy_per_attempt_j").as<double>()),
        {}};
    for (const auto& outcome : value.at("repeating_outcomes").array_range()) {
        result.repeating_outcomes.push_back(decode_outcome(outcome.as<std::string_view>()));
    }
    return result;
}

template <typename Target> void decode_strings(const Json& values, Target& target) {
    for (const auto& value : values.array_range()) {
        target.push_back(value.as<std::string>());
    }
}

[[nodiscard]] BanStationProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& gateway = document.at("gateway_adapter");
    const auto& station = document.at("station");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    BanStationProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {gateway.at("id").as<std::string>(), gateway.at("gateway_id").as<std::string>(),
         gateway.at("station_id").as<std::string>(),
         gateway.at("measurement_frame_id_prefix").as<std::string>(),
         gateway.at("maximum_uplink_frames").as<std::uint64_t>(),
         gateway.at("maximum_downlink_frames").as<std::uint64_t>()},
        {station.at("id").as<std::string>(),
         station.at("decision_id_prefix").as<std::string>(),
         station.at("command_frame_id_prefix").as<std::string>(),
         {},
         {},
         {},
         station.at("maximum_measurements").as<std::uint64_t>(),
         station.at("maximum_approved_commands").as<std::uint64_t>()},
        decode_transport(document.at("uplink_transport")),
        decode_transport(document.at("downlink_transport")),
        {reference.at("measurement_id").as<std::string>(),
         reference.at("command_id").as<std::string>(),
         reference.at("target_device_id").as<std::string>(),
         seconds(reference.at("expected_uplink_latency_seconds").as<double>()),
         seconds(reference.at("expected_downlink_latency_seconds").as<double>())},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};

    decode_strings(station.at("accepted_gateway_ids"), result.station.accepted_gateway_ids);
    decode_strings(station.at("allowed_target_device_ids"),
                   result.station.allowed_target_device_ids);
    decode_strings(station.at("allowed_command_content_types"),
                   result.station.allowed_command_content_types);
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    decode_strings(document.at("limitations"), result.limitations);
    return result;
}

} // namespace

void validate_ban_station_profile(const BanStationProfile& profile) {
    validate_gateway_ban_adapter_config(profile.gateway_adapter);
    validate_external_station_config(profile.station);
    validate_scheduled_ban_transport_config(profile.uplink_transport);
    validate_scheduled_ban_transport_config(profile.downlink_transport);
    if (profile.schema_version != ban_station_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.gateway_adapter.station_id != profile.station.station_id ||
        std::ranges::find(profile.station.accepted_gateway_ids,
                          profile.gateway_adapter.gateway_id) ==
            profile.station.accepted_gateway_ids.end() ||
        profile.reference_case.measurement_id.empty() ||
        profile.reference_case.command_id.empty() ||
        std::ranges::find(profile.station.allowed_target_device_ids,
                          profile.reference_case.target_device_id) ==
            profile.station.allowed_target_device_ids.end() ||
        profile.reference_case.expected_uplink_latency != profile.uplink_transport.latency ||
        profile.reference_case.expected_downlink_latency != profile.downlink_transport.latency ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "BAN-station profile is incomplete or internally inconsistent");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "BAN-station sources must be complete and unique");
        }
    }
    if (std::ranges::any_of(profile.limitations,
                            [](const auto& limitation) { return limitation.empty(); })) {
        invalid(core::ErrorCode::data_invalid, "BAN-station limitations must not be empty");
    }
}

BanStationProfile load_ban_station_profile(const BanStationProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "BAN-station schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "BAN-station profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_ban_station_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "BAN-station profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

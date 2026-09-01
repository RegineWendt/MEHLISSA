// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/network_simulator_adapter_profile.hpp>

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
                "Network-simulator profile duration is outside the simulation clock range");
    }
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] NetworkSimulationOutcome decode_outcome(const std::string_view value) {
    if (value == "delivered") {
        return NetworkSimulationOutcome::delivered;
    }
    if (value == "lost") {
        return NetworkSimulationOutcome::lost;
    }
    if (value == "corrupted") {
        return NetworkSimulationOutcome::corrupted;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown network-simulator profile outcome");
}

[[nodiscard]] NetworkSimulatorAdapterProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& adapter = document.at("adapter");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    NetworkSimulatorAdapterProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {adapter.at("id").as<std::string>(), adapter.at("simulator_id").as<std::string>(),
         adapter.at("simulator_version").as<std::string>(),
         adapter.at("scenario_id").as<std::string>(),
         adapter.at("request_id_prefix").as<std::string>(),
         adapter.at("maximum_attempts").as<std::uint64_t>()},
        {reference.at("frame_id").as<std::string>(),
         decode_outcome(reference.at("expected_outcome").as<std::string_view>()),
         seconds(reference.at("expected_latency_seconds").as<double>()),
         core::joules(reference.at("expected_transmitter_energy_j").as<double>()),
         core::joules(reference.at("expected_receiver_energy_j").as<double>()),
         core::joules(reference.at("expected_link_energy_j").as<double>())},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};

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

[[nodiscard]] bool nonnegative_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) >= 0.0;
}

} // namespace

void validate_network_simulator_adapter_profile(const NetworkSimulatorAdapterProfile& profile) {
    validate_external_network_simulator_adapter_config(profile.adapter);
    if (profile.schema_version != network_simulator_adapter_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.reference_case.frame_id.empty() ||
        profile.reference_case.expected_latency < core::SimulationClock::Duration::zero() ||
        !nonnegative_finite(profile.reference_case.expected_transmitter_energy) ||
        !nonnegative_finite(profile.reference_case.expected_receiver_energy) ||
        !nonnegative_finite(profile.reference_case.expected_link_energy) ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulator adapter profile is incomplete or internally inconsistent");
    }
    if (profile.reference_case.expected_outcome != NetworkSimulationOutcome::delivered &&
        profile.reference_case.expected_outcome != NetworkSimulationOutcome::lost &&
        profile.reference_case.expected_outcome != NetworkSimulationOutcome::corrupted) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulator adapter profile has unknown reference outcome");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Network-simulator adapter sources must be complete and unique");
        }
    }
    if (std::ranges::any_of(profile.limitations,
                            [](const auto& limitation) { return limitation.empty(); })) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulator adapter limitations must not be empty");
    }
}

NetworkSimulatorAdapterProfile
load_network_simulator_adapter_profile(const NetworkSimulatorAdapterProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "network-simulator adapter schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "network-simulator adapter profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_network_simulator_adapter_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Network-simulator adapter profile validation failed: " +
                    std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

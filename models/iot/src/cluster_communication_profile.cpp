// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/cluster_communication_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <utility>

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
                "Cluster-communication duration is outside the simulation clock range");
    }
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] ClusterMemberRole decode_role(const std::string_view value) {
    if (value == "endpoint") {
        return ClusterMemberRole::endpoint;
    }
    if (value == "relay") {
        return ClusterMemberRole::relay;
    }
    if (value == "collector") {
        return ClusterMemberRole::collector;
    }
    if (value == "gateway") {
        return ClusterMemberRole::gateway;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown cluster member role");
}

[[nodiscard]] ClusterRouteStrategy decode_strategy(const std::string_view value) {
    if (value == "fewest_hops") {
        return ClusterRouteStrategy::fewest_hops;
    }
    if (value == "lowest_total_latency") {
        return ClusterRouteStrategy::lowest_total_latency;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown cluster route strategy");
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
    invalid(core::ErrorCode::data_invalid, "Unknown cluster-link outcome");
}

[[nodiscard]] ClusterCommunicationProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& cluster = document.at("cluster");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    ClusterCommunicationProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {cluster.at("id").as<std::string>(),
         cluster.at("maximum_hops").as<std::uint32_t>(),
         {},
         {}},
        {reference.at("source_device_id").as<std::string>(),
         reference.at("target_device_id").as<std::string>(),
         decode_strategy(reference.at("strategy").as<std::string>()),
         {},
         seconds(reference.at("expected_total_latency_seconds").as<double>()),
         core::joules(reference.at("expected_total_link_energy_j").as<double>())},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};

    for (const auto& member : cluster.at("members").array_range()) {
        result.cluster.members.push_back({member.at("device_id").as<std::string>(),
                                          decode_role(member.at("role").as<std::string>())});
    }
    for (const auto& link : cluster.at("links").array_range()) {
        ClusterDirectedLinkConfig edge{link.at("source_device_id").as<std::string>(),
                                       link.at("target_device_id").as<std::string>(),
                                       {link.at("id").as<std::string>(),
                                        seconds(link.at("latency_seconds").as<double>()),
                                        core::joules(link.at("energy_per_attempt_j").as<double>()),
                                        {}}};
        for (const auto& outcome : link.at("repeating_outcomes").array_range()) {
            edge.link.repeating_outcomes.push_back(decode_outcome(outcome.as<std::string_view>()));
        }
        result.cluster.links.push_back(std::move(edge));
    }
    for (const auto& device_id : reference.at("expected_route_device_ids").array_range()) {
        result.reference_case.expected_route_device_ids.push_back(device_id.as<std::string>());
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

void validate_cluster_communication_profile(const ClusterCommunicationProfile& profile) {
    validate_nanodevice_cluster_config(profile.cluster);
    NanodeviceCluster cluster{profile.cluster};
    const auto route = cluster.select_route(profile.reference_case.source_device_id,
                                            profile.reference_case.target_device_id,
                                            profile.reference_case.strategy);
    auto link_energy = core::joules(0.0);
    for (const auto index : route.link_indices) {
        link_energy += profile.cluster.links[index].link.energy_per_attempt;
    }
    const auto expected_energy = core::in_joules(profile.reference_case.expected_total_link_energy);
    const auto energy_tolerance = std::max(1.0e-18, std::abs(expected_energy) * 1.0e-12);

    if (profile.schema_version != cluster_communication_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        route.device_ids != profile.reference_case.expected_route_device_ids ||
        route.total_configured_latency != profile.reference_case.expected_total_latency ||
        std::abs(core::in_joules(link_energy) - expected_energy) > energy_tolerance ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Cluster-communication profile is incomplete or internally inconsistent");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Cluster-communication sources must be complete and unique");
        }
    }
    if (std::ranges::any_of(profile.limitations,
                            [](const auto& limitation) { return limitation.empty(); })) {
        invalid(core::ErrorCode::data_invalid,
                "Cluster-communication limitations must not be empty");
    }
}

ClusterCommunicationProfile
load_cluster_communication_profile(const ClusterCommunicationProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "cluster-communication schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "cluster-communication profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_cluster_communication_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Cluster-communication profile validation failed: " + std::string{error.what()});
    }
}

} // namespace mehlissa::models::iot

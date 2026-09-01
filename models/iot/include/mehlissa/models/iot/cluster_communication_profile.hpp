// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_CLUSTER_COMMUNICATION_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_CLUSTER_COMMUNICATION_PROFILE_HPP

#include <mehlissa/models/iot/multi_hop_network.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view cluster_communication_profile_schema_version = "1.0.0";

struct ClusterCommunicationValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct ClusterCommunicationSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct ClusterCommunicationReferenceCase final {
    std::string source_device_id;
    std::string target_device_id;
    ClusterRouteStrategy strategy{ClusterRouteStrategy::fewest_hops};
    std::vector<std::string> expected_route_device_ids;
    core::SimulationClock::Duration expected_total_latency{};
    core::Energy expected_total_link_energy{};
};

struct ClusterCommunicationProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    NanodeviceClusterConfig cluster;
    ClusterCommunicationReferenceCase reference_case;
    ClusterCommunicationValidity validity;
    std::vector<ClusterCommunicationSource> sources;
    std::vector<std::string> limitations;
};

struct ClusterCommunicationProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_cluster_communication_profile(const ClusterCommunicationProfile& profile);

[[nodiscard]] ClusterCommunicationProfile
load_cluster_communication_profile(const ClusterCommunicationProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_CLUSTER_COMMUNICATION_PROFILE_HPP

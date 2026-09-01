// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_LOCAL_COMMUNICATION_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_LOCAL_COMMUNICATION_PROFILE_HPP

#include <mehlissa/models/iot/molecular_detection.hpp>
#include <mehlissa/models/iot/one_hop_link.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view local_communication_profile_schema_version = "1.0.0";

struct LocalCommunicationValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct LocalCommunicationSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct LocalCommunicationReferenceCase final {
    std::string source_profile_id;
    std::string event_id;
    ScheduledLinkOutcome expected_outcome{ScheduledLinkOutcome::delivered};
    core::SimulationClock::Duration expected_latency{};
    core::Energy expected_link_energy{};
};

struct LocalCommunicationProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    DetectionMessageAdapterConfig detection_adapter;
    ScheduledOneHopLinkConfig link;
    LocalCommunicationReferenceCase reference_case;
    LocalCommunicationValidity validity;
    std::vector<LocalCommunicationSource> sources;
    std::vector<std::string> limitations;
};

struct LocalCommunicationProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_local_communication_profile(const LocalCommunicationProfile& profile);

[[nodiscard]] LocalCommunicationProfile
load_local_communication_profile(const LocalCommunicationProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_LOCAL_COMMUNICATION_PROFILE_HPP

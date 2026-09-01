// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_BAN_STATION_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_BAN_STATION_PROFILE_HPP

#include <mehlissa/models/iot/ban_station.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view ban_station_profile_schema_version = "1.0.0";

struct BanStationValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct BanStationSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct BanStationReferenceCase final {
    std::string measurement_id;
    std::string command_id;
    std::string target_device_id;
    core::SimulationClock::Duration expected_uplink_latency{};
    core::SimulationClock::Duration expected_downlink_latency{};
};

struct BanStationProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    GatewayBanAdapterConfig gateway_adapter;
    ExternalStationConfig station;
    ScheduledBanTransportConfig uplink_transport;
    ScheduledBanTransportConfig downlink_transport;
    BanStationReferenceCase reference_case;
    BanStationValidity validity;
    std::vector<BanStationSource> sources;
    std::vector<std::string> limitations;
};

struct BanStationProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_ban_station_profile(const BanStationProfile& profile);

[[nodiscard]] BanStationProfile
load_ban_station_profile(const BanStationProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_BAN_STATION_PROFILE_HPP

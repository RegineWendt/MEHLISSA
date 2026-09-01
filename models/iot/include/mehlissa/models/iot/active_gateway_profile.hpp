// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_PROFILE_HPP

#include <mehlissa/models/iot/active_gateway.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view active_gateway_profile_schema_version = "1.0.0";

struct ActiveGatewayValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct ActiveGatewaySource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct ActiveGatewayReferenceCase final {
    std::string uplink_source_device_id;
    std::string downlink_target_device_id;
    std::string command_id;
};

struct ActiveGatewayProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    ActiveGatewayConfig gateway;
    ActiveGatewayReferenceCase reference_case;
    ActiveGatewayValidity validity;
    std::vector<ActiveGatewaySource> sources;
    std::vector<std::string> limitations;
};

struct ActiveGatewayProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_active_gateway_profile(const ActiveGatewayProfile& profile);

[[nodiscard]] ActiveGatewayProfile
load_active_gateway_profile(const ActiveGatewayProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_ACTIVE_GATEWAY_PROFILE_HPP

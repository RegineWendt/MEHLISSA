// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_NANODEVICE_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_NANODEVICE_PROFILE_HPP

#include <mehlissa/models/iot/nanodevice.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr auto nanodevice_profile_schema_version = "1.0.0";

struct NanodeviceValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct NanodeviceSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct NanodeviceProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    NanodeviceConfig device;
    NanodeviceValidity validity;
    std::vector<NanodeviceSource> sources;
    std::vector<std::string> limitations;
};

struct NanodeviceProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_nanodevice_profile(const NanodeviceProfile& profile);
[[nodiscard]] NanodeviceProfile
load_nanodevice_profile(const NanodeviceProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_NANODEVICE_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_PROFILE_HPP
#define MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_PROFILE_HPP

#include <mehlissa/models/iot/network_simulator_adapter.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view network_simulator_adapter_profile_schema_version = "1.0.0";

struct NetworkSimulatorAdapterValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct NetworkSimulatorAdapterSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct NetworkSimulatorAdapterReferenceCase final {
    std::string frame_id;
    NetworkSimulationOutcome expected_outcome{NetworkSimulationOutcome::lost};
    core::SimulationClock::Duration expected_latency{};
    core::Energy expected_transmitter_energy{};
    core::Energy expected_receiver_energy{};
    core::Energy expected_link_energy{};
};

struct NetworkSimulatorAdapterProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    ExternalNetworkSimulatorAdapterConfig adapter;
    NetworkSimulatorAdapterReferenceCase reference_case;
    NetworkSimulatorAdapterValidity validity;
    std::vector<NetworkSimulatorAdapterSource> sources;
    std::vector<std::string> limitations;
};

struct NetworkSimulatorAdapterProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_network_simulator_adapter_profile(const NetworkSimulatorAdapterProfile& profile);

[[nodiscard]] NetworkSimulatorAdapterProfile
load_network_simulator_adapter_profile(const NetworkSimulatorAdapterProfileLoadRequest& request);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_NETWORK_SIMULATOR_ADAPTER_PROFILE_HPP

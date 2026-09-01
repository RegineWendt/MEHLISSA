// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_MOLECULAR_DETECTION_HPP
#define MEHLISSA_MODELS_IOT_MOLECULAR_DETECTION_HPP

#include <mehlissa/models/iot/local_message.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace mehlissa::models::iot {

inline constexpr std::string_view molecular_detection_contract_version = "1.0.0";

struct MolecularDetectionEvent final {
    std::string contract_version;
    std::string event_id;
    std::string source_model_id;
    std::string source_request_id;
    std::string detector_device_id;
    std::string signal_id;
    std::string compartment_id;
    core::SimulationClock::Duration detected_at{};
    double observed_fraction{};
};

struct DetectionMessageAdapterConfig final {
    std::string adapter_id;
    std::string source_device_id;
    std::string target_device_id;
    std::string message_id_prefix;
    std::string correlation_id;
    core::SimulationClock::Duration valid_for{};
    std::uint32_t hop_limit{};
    std::uint64_t size_bytes{};
    std::string content_type;
};

void validate_molecular_detection_event(const MolecularDetectionEvent& event);
void validate_detection_message_adapter_config(const DetectionMessageAdapterConfig& config);

[[nodiscard]] LocalMessageRequest
make_detection_message_request(const DetectionMessageAdapterConfig& config,
                               const MolecularDetectionEvent& event);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_MOLECULAR_DETECTION_HPP

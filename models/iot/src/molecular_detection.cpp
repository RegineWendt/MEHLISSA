// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/molecular_detection.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>

#include <cmath>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

void validate_molecular_detection_event(const MolecularDetectionEvent& event) {
    if (event.contract_version != molecular_detection_contract_version || event.event_id.empty() ||
        event.source_model_id.empty() || event.source_request_id.empty() ||
        event.detector_device_id.empty() || event.signal_id.empty() ||
        event.compartment_id.empty() ||
        event.detected_at < core::SimulationClock::Duration::zero() ||
        !valid_fraction(event.observed_fraction)) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular detection requires complete identity, time, and bounded observation");
    }
}

void validate_detection_message_adapter_config(const DetectionMessageAdapterConfig& config) {
    if (config.adapter_id.empty() || config.source_device_id.empty() ||
        config.target_device_id.empty() || config.source_device_id == config.target_device_id ||
        config.message_id_prefix.empty() || config.correlation_id.empty() ||
        config.valid_for <= core::SimulationClock::Duration::zero() || config.hop_limit == 0 ||
        config.size_bytes == 0 || config.content_type.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Detection-message adapter configuration is incomplete or invalid");
    }
}

LocalMessageRequest make_detection_message_request(const DetectionMessageAdapterConfig& config,
                                                   const MolecularDetectionEvent& event) {
    validate_detection_message_adapter_config(config);
    validate_molecular_detection_event(event);
    if (event.detector_device_id != config.source_device_id) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular detection does not belong to the configured source device");
    }

    const jsoncons::json payload{
        jsoncons::json_object_arg,
        {{"event_id", event.event_id},
         {"source_model_id", event.source_model_id},
         {"source_request_id", event.source_request_id},
         {"signal_id", event.signal_id},
         {"compartment_id", event.compartment_id},
         {"observed_fraction", event.observed_fraction}},
    };
    std::string content;
    jsoncons::encode_json(payload, content);
    if (content.size() > config.size_bytes) {
        invalid(core::ErrorCode::data_invalid,
                "Encoded detection content exceeds its declared logical message size");
    }

    LocalMessageRequest request{config.message_id_prefix + "." + event.event_id,
                                LocalMessageKind::detection,
                                config.target_device_id,
                                config.correlation_id,
                                event.event_id,
                                event.detected_at,
                                config.valid_for,
                                config.hop_limit,
                                config.size_bytes,
                                config.content_type,
                                std::move(content)};
    validate_local_message_request(request);
    return request;
}

} // namespace mehlissa::models::iot

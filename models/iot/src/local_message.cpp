// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/local_message.hpp>

#include <mehlissa/core/error.hpp>

#include <limits>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const char* message) {
    throw core::MehlissaError{code, message};
}

void validate_fields(const LocalMessage& message, const bool require_source) {
    if (message.message_id.empty() || message.target_device_id.empty() ||
        message.correlation_id.empty() || message.source_event_id.empty() ||
        message.created_at < core::SimulationClock::Duration::zero() ||
        message.valid_for <= core::SimulationClock::Duration::zero() || message.hop_limit == 0 ||
        message.size_bytes == 0 || message.content_type.empty() || message.content.empty() ||
        (require_source && message.source_device_id.empty())) {
        invalid(core::ErrorCode::data_invalid,
                "A local message requires complete identity, timing, routing, size, and content");
    }
    if (!message.source_device_id.empty() && message.source_device_id == message.target_device_id) {
        invalid(core::ErrorCode::data_invalid,
                "A local message cannot address its own source device");
    }
    const auto maximum = core::SimulationClock::Duration::max();
    if (message.created_at > maximum - message.valid_for) {
        invalid(core::ErrorCode::numeric_overflow, "Local-message validity time overflows");
    }
}

} // namespace

std::string_view to_string(const LocalMessageKind kind) noexcept {
    switch (kind) {
    case LocalMessageKind::detection:
        return "detection";
    case LocalMessageKind::fingerprint_tile:
        return "fingerprint_tile";
    case LocalMessageKind::measurement:
        return "measurement";
    case LocalMessageKind::control:
        return "control";
    case LocalMessageKind::acknowledgement:
        return "acknowledgement";
    }
    return "unknown";
}

void validate_local_message_request(const LocalMessageRequest& request) {
    const LocalMessage candidate{std::string{local_message_contract_version},
                                 request.message_id,
                                 request.kind,
                                 {},
                                 request.target_device_id,
                                 request.correlation_id,
                                 request.source_event_id,
                                 request.created_at,
                                 request.valid_for,
                                 request.hop_limit,
                                 request.size_bytes,
                                 request.content_type,
                                 request.content};
    validate_fields(candidate, false);
}

void validate_local_message(const LocalMessage& message) {
    if (message.contract_version != local_message_contract_version) {
        invalid(core::ErrorCode::data_invalid,
                "A local message requires the supported contract version");
    }
    validate_fields(message, true);
}

core::SimulationClock::Duration valid_until(const LocalMessage& message) {
    validate_local_message(message);
    return message.created_at + message.valid_for;
}

} // namespace mehlissa::models::iot

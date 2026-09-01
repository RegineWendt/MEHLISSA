// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_LOCAL_MESSAGE_HPP
#define MEHLISSA_MODELS_IOT_LOCAL_MESSAGE_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace mehlissa::models::iot {

inline constexpr std::string_view local_message_contract_version = "1.0.0";
inline constexpr std::string_view broadcast_device_id = "broadcast";

enum class LocalMessageKind : std::uint8_t {
    detection,
    fingerprint_tile,
    measurement,
    control,
    acknowledgement
};

[[nodiscard]] std::string_view to_string(LocalMessageKind kind) noexcept;

struct LocalMessageRequest final {
    std::string message_id;
    LocalMessageKind kind{LocalMessageKind::detection};
    std::string target_device_id;
    std::string correlation_id;
    std::string source_event_id;
    core::SimulationClock::Duration created_at{};
    core::SimulationClock::Duration valid_for{};
    std::uint32_t hop_limit{};
    std::uint64_t size_bytes{};
    std::string content_type;
    std::string content;
};

struct LocalMessage final {
    std::string contract_version;
    std::string message_id;
    LocalMessageKind kind{LocalMessageKind::detection};
    std::string source_device_id;
    std::string target_device_id;
    std::string correlation_id;
    std::string source_event_id;
    core::SimulationClock::Duration created_at{};
    core::SimulationClock::Duration valid_for{};
    std::uint32_t hop_limit{};
    std::uint64_t size_bytes{};
    std::string content_type;
    std::string content;

    [[nodiscard]] bool operator==(const LocalMessage&) const noexcept = default;
};

void validate_local_message_request(const LocalMessageRequest& request);
void validate_local_message(const LocalMessage& message);
[[nodiscard]] core::SimulationClock::Duration valid_until(const LocalMessage& message);

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_LOCAL_MESSAGE_HPP

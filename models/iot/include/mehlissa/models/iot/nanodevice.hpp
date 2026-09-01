// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_NANODEVICE_HPP
#define MEHLISSA_MODELS_IOT_NANODEVICE_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/iot/local_message.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::iot {

enum class NanodeviceCapability : std::uint8_t {
    sense,
    transmit,
    receive,
    relay,
    collect,
    actuate,
    release_payload
};

enum class NanodeviceLifecycleState : std::uint8_t { dormant, active, depleted, failed };

[[nodiscard]] std::string_view to_string(NanodeviceCapability capability) noexcept;
[[nodiscard]] std::string_view to_string(NanodeviceLifecycleState state) noexcept;

struct NanodeviceTarget final {
    std::string kind;
    std::string id;
};

struct NanodevicePayload final {
    std::string payload_id;
    std::string payload_type;
    std::optional<core::Amount> amount;
    std::optional<std::uint64_t> unit_count;
};

struct NanodeviceResourceConfig final {
    core::Energy initial_energy{};
    core::Energy transmit_energy_per_message{};
    core::Energy receive_energy_per_message{};
    std::uint64_t maximum_message_size_bytes{};
    std::uint64_t message_storage_capacity_bytes{};
    std::uint64_t maximum_transmissions{};
    std::uint64_t maximum_receptions{};
};

struct NanodeviceConfig final {
    std::string device_id;
    std::string device_type;
    NanodeviceLifecycleState initial_state{NanodeviceLifecycleState::dormant};
    NanodeviceTarget target;
    std::vector<NanodeviceCapability> capabilities;
    std::vector<NanodevicePayload> payloads;
    NanodeviceResourceConfig resources;
};

void validate_nanodevice_config(const NanodeviceConfig& config);

class Nanodevice final {
  public:
    explicit Nanodevice(NanodeviceConfig config);

    [[nodiscard]] std::string_view device_id() const noexcept;
    [[nodiscard]] std::string_view device_type() const noexcept;
    [[nodiscard]] NanodeviceLifecycleState state() const noexcept;
    [[nodiscard]] bool has_capability(NanodeviceCapability capability) const noexcept;
    [[nodiscard]] const NanodeviceTarget& target() const noexcept;
    [[nodiscard]] const std::vector<NanodevicePayload>& payloads() const noexcept;
    [[nodiscard]] core::Energy remaining_energy() const noexcept;
    [[nodiscard]] std::uint64_t used_message_storage_bytes() const noexcept;
    [[nodiscard]] std::uint64_t transmission_count() const noexcept;
    [[nodiscard]] std::uint64_t reception_count() const noexcept;
    [[nodiscard]] core::SimulationClock::Duration state_changed_at() const noexcept;
    [[nodiscard]] const std::vector<LocalMessage>& received_messages() const noexcept;

    void activate(core::SimulationClock::Duration time);
    void fail(core::SimulationClock::Duration time);
    [[nodiscard]] LocalMessage emit_local_message(const LocalMessageRequest& request);
    void receive_local_message(LocalMessage message, core::SimulationClock::Duration received_at);
    [[nodiscard]] std::vector<LocalMessage> take_received_messages();

  private:
    void require_active(std::string_view operation) const;
    void spend_energy(core::Energy amount, core::SimulationClock::Duration time);

    NanodeviceConfig config_;
    NanodeviceLifecycleState state_{};
    core::Energy remaining_energy_{};
    std::uint64_t used_message_storage_bytes_{};
    std::uint64_t transmission_count_{};
    std::uint64_t reception_count_{};
    core::SimulationClock::Duration state_changed_at_{};
    std::unordered_set<std::string> received_message_ids_;
    std::vector<LocalMessage> received_messages_;
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_NANODEVICE_HPP

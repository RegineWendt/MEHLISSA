// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/nanodevice.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <set>
#include <utility>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool nonnegative_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) >= 0.0;
}

[[nodiscard]] bool positive_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) > 0.0;
}

[[nodiscard]] bool contains(const std::vector<NanodeviceCapability>& capabilities,
                            const NanodeviceCapability capability) noexcept {
    for (const auto candidate : capabilities) {
        if (candidate == capability) {
            return true;
        }
    }
    return false;
}

} // namespace

std::string_view to_string(const NanodeviceCapability capability) noexcept {
    switch (capability) {
    case NanodeviceCapability::sense:
        return "sense";
    case NanodeviceCapability::transmit:
        return "transmit";
    case NanodeviceCapability::receive:
        return "receive";
    case NanodeviceCapability::relay:
        return "relay";
    case NanodeviceCapability::collect:
        return "collect";
    case NanodeviceCapability::actuate:
        return "actuate";
    case NanodeviceCapability::release_payload:
        return "release_payload";
    }
    return "unknown";
}

std::string_view to_string(const NanodeviceLifecycleState state) noexcept {
    switch (state) {
    case NanodeviceLifecycleState::dormant:
        return "dormant";
    case NanodeviceLifecycleState::active:
        return "active";
    case NanodeviceLifecycleState::depleted:
        return "depleted";
    case NanodeviceLifecycleState::failed:
        return "failed";
    }
    return "unknown";
}

void validate_nanodevice_config(const NanodeviceConfig& config) {
    if (config.device_id.empty() || config.device_type.empty() || config.target.kind.empty() ||
        config.target.id.empty() || config.capabilities.empty() ||
        (config.initial_state != NanodeviceLifecycleState::dormant &&
         config.initial_state != NanodeviceLifecycleState::active) ||
        !positive_finite(config.resources.initial_energy) ||
        !nonnegative_finite(config.resources.transmit_energy_per_message) ||
        !nonnegative_finite(config.resources.receive_energy_per_message)) {
        invalid(core::ErrorCode::data_invalid, "Nanodevice configuration is incomplete or invalid");
    }

    std::set<NanodeviceCapability> capabilities;
    for (const auto capability : config.capabilities) {
        if (!capabilities.insert(capability).second) {
            invalid(core::ErrorCode::data_invalid, "Nanodevice capabilities must be unique");
        }
    }

    const auto can_transmit = contains(config.capabilities, NanodeviceCapability::transmit);
    const auto can_receive = contains(config.capabilities, NanodeviceCapability::receive);
    if (can_transmit) {
        if (!positive_finite(config.resources.transmit_energy_per_message) ||
            config.resources.maximum_message_size_bytes == 0 ||
            config.resources.maximum_transmissions == 0) {
            invalid(core::ErrorCode::data_invalid,
                    "A transmitting nanodevice requires energy, size, and count budgets");
        }
    } else if (core::in_joules(config.resources.transmit_energy_per_message) != 0.0 ||
               config.resources.maximum_transmissions != 0) {
        invalid(core::ErrorCode::data_invalid,
                "Transmit resources require the transmit capability");
    }

    if (can_receive) {
        if (!positive_finite(config.resources.receive_energy_per_message) ||
            config.resources.maximum_message_size_bytes == 0 ||
            config.resources.message_storage_capacity_bytes == 0 ||
            config.resources.maximum_receptions == 0) {
            invalid(core::ErrorCode::data_invalid,
                    "A receiving nanodevice requires energy, size, storage, and count budgets");
        }
    } else if (core::in_joules(config.resources.receive_energy_per_message) != 0.0 ||
               config.resources.message_storage_capacity_bytes != 0 ||
               config.resources.maximum_receptions != 0) {
        invalid(core::ErrorCode::data_invalid, "Receive resources require the receive capability");
    }

    if (contains(config.capabilities, NanodeviceCapability::relay) &&
        (!can_transmit || !can_receive)) {
        invalid(core::ErrorCode::data_invalid,
                "Relay capability requires both transmit and receive capabilities");
    }
    if (contains(config.capabilities, NanodeviceCapability::collect) && !can_receive) {
        invalid(core::ErrorCode::data_invalid,
                "Collect capability requires the receive capability");
    }

    std::unordered_set<std::string> payload_ids;
    for (const auto& payload : config.payloads) {
        const auto has_amount = payload.amount.has_value();
        const auto has_units = payload.unit_count.has_value();
        if (payload.payload_id.empty() || payload.payload_type.empty() || has_amount == has_units ||
            !payload_ids.insert(payload.payload_id).second ||
            (has_amount && (!std::isfinite(core::in_moles(payload.amount.value())) ||
                            core::in_moles(payload.amount.value()) <= 0.0)) ||
            (has_units && payload.unit_count.value() == 0)) {
            invalid(
                core::ErrorCode::data_invalid,
                "Nanodevice payloads require unique identity and exactly one positive quantity");
        }
    }
}

Nanodevice::Nanodevice(NanodeviceConfig config)
    : config_{std::move(config)}, state_{config_.initial_state},
      remaining_energy_{config_.resources.initial_energy} {
    validate_nanodevice_config(config_);
}

std::string_view Nanodevice::device_id() const noexcept { return config_.device_id; }

std::string_view Nanodevice::device_type() const noexcept { return config_.device_type; }

NanodeviceLifecycleState Nanodevice::state() const noexcept { return state_; }

bool Nanodevice::has_capability(const NanodeviceCapability capability) const noexcept {
    return contains(config_.capabilities, capability);
}

const NanodeviceTarget& Nanodevice::target() const noexcept { return config_.target; }

const std::vector<NanodevicePayload>& Nanodevice::payloads() const noexcept {
    return config_.payloads;
}

core::Energy Nanodevice::remaining_energy() const noexcept { return remaining_energy_; }

std::uint64_t Nanodevice::used_message_storage_bytes() const noexcept {
    return used_message_storage_bytes_;
}

std::uint64_t Nanodevice::transmission_count() const noexcept { return transmission_count_; }

std::uint64_t Nanodevice::reception_count() const noexcept { return reception_count_; }

core::SimulationClock::Duration Nanodevice::state_changed_at() const noexcept {
    return state_changed_at_;
}

void Nanodevice::activate(const core::SimulationClock::Duration time) {
    if (state_ != NanodeviceLifecycleState::dormant ||
        time < core::SimulationClock::Duration::zero()) {
        invalid(core::ErrorCode::lifecycle_invalid,
                "Only a dormant nanodevice can be activated at a valid time");
    }
    state_ = NanodeviceLifecycleState::active;
    state_changed_at_ = time;
}

void Nanodevice::fail(const core::SimulationClock::Duration time) {
    if (state_ == NanodeviceLifecycleState::failed || time < state_changed_at_) {
        invalid(core::ErrorCode::lifecycle_invalid,
                "Nanodevice failure must be a forward, one-time lifecycle transition");
    }
    state_ = NanodeviceLifecycleState::failed;
    state_changed_at_ = time;
}

void Nanodevice::require_active(const std::string_view operation) const {
    if (state_ != NanodeviceLifecycleState::active) {
        invalid(core::ErrorCode::lifecycle_invalid,
                "Nanodevice must be active to " + std::string{operation});
    }
}

void Nanodevice::spend_energy(const core::Energy amount,
                              const core::SimulationClock::Duration time) {
    if (remaining_energy_ < amount) {
        invalid(core::ErrorCode::invariant_violated,
                "Nanodevice has insufficient energy for the requested operation");
    }
    remaining_energy_ -= amount;
    if (core::in_joules(remaining_energy_) == 0.0) {
        state_ = NanodeviceLifecycleState::depleted;
        state_changed_at_ = time;
    }
}

LocalMessage Nanodevice::emit_local_message(const LocalMessageRequest& request) {
    require_active("emit a local message");
    if (!has_capability(NanodeviceCapability::transmit)) {
        invalid(core::ErrorCode::invariant_violated, "Nanodevice lacks the transmit capability");
    }
    validate_local_message_request(request);
    if (request.size_bytes > config_.resources.maximum_message_size_bytes ||
        transmission_count_ >= config_.resources.maximum_transmissions) {
        invalid(core::ErrorCode::invariant_violated,
                "Local message exceeds the nanodevice transmission budget");
    }
    if (remaining_energy_ < config_.resources.transmit_energy_per_message) {
        invalid(core::ErrorCode::invariant_violated,
                "Nanodevice has insufficient energy to transmit");
    }

    LocalMessage message{std::string{local_message_contract_version},
                         request.message_id,
                         request.kind,
                         config_.device_id,
                         request.target_device_id,
                         request.correlation_id,
                         request.source_event_id,
                         request.created_at,
                         request.valid_for,
                         request.hop_limit,
                         request.size_bytes,
                         request.content_type,
                         request.content};
    validate_local_message(message);
    spend_energy(config_.resources.transmit_energy_per_message, request.created_at);
    ++transmission_count_;
    return message;
}

void Nanodevice::receive_local_message(LocalMessage message,
                                       const core::SimulationClock::Duration received_at) {
    require_active("receive a local message");
    if (!has_capability(NanodeviceCapability::receive)) {
        invalid(core::ErrorCode::invariant_violated, "Nanodevice lacks the receive capability");
    }
    validate_local_message(message);
    if ((message.target_device_id != config_.device_id &&
         message.target_device_id != broadcast_device_id) ||
        received_at < message.created_at || received_at > valid_until(message)) {
        invalid(core::ErrorCode::data_invalid, "Local message target or reception time is invalid");
    }
    if (message.size_bytes > config_.resources.maximum_message_size_bytes ||
        message.size_bytes >
            config_.resources.message_storage_capacity_bytes - used_message_storage_bytes_ ||
        reception_count_ >= config_.resources.maximum_receptions ||
        received_message_ids_.contains(message.message_id)) {
        invalid(core::ErrorCode::invariant_violated,
                "Local message exceeds a reception budget or duplicates an earlier message");
    }
    if (remaining_energy_ < config_.resources.receive_energy_per_message) {
        invalid(core::ErrorCode::invariant_violated,
                "Nanodevice has insufficient energy to receive");
    }

    received_messages_.push_back(std::move(message));
    const auto& stored = received_messages_.back();
    received_message_ids_.insert(stored.message_id);
    used_message_storage_bytes_ += stored.size_bytes;
    ++reception_count_;
    spend_energy(config_.resources.receive_energy_per_message, received_at);
}

std::vector<LocalMessage> Nanodevice::take_received_messages() {
    used_message_storage_bytes_ = 0;
    auto result = std::move(received_messages_);
    received_messages_.clear();
    return result;
}

} // namespace mehlissa::models::iot

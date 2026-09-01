// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/one_hop_link.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool nonnegative_finite(const core::Energy energy) noexcept {
    return std::isfinite(core::in_joules(energy)) && core::in_joules(energy) >= 0.0;
}

[[nodiscard]] double fraction(const std::uint64_t numerator,
                              const std::uint64_t denominator) noexcept {
    if (denominator == 0) {
        return 0.0;
    }
    return static_cast<double>(numerator) / static_cast<double>(denominator);
}

void add_count(std::uint64_t& total, const std::uint64_t increment) {
    if (increment > std::numeric_limits<std::uint64_t>::max() - total) {
        invalid(core::ErrorCode::numeric_overflow, "Communication metric counter overflows");
    }
    total += increment;
}

void add_duration(core::SimulationClock::Duration& total,
                  const core::SimulationClock::Duration increment) {
    if (increment > core::SimulationClock::Duration::max() - total) {
        invalid(core::ErrorCode::numeric_overflow, "Communication latency metric overflows");
    }
    total += increment;
}

void add_energy(core::Energy& total, const core::Energy increment) {
    const auto sum = core::in_joules(total) + core::in_joules(increment);
    if (!std::isfinite(sum) || sum < 0.0) {
        invalid(core::ErrorCode::numeric_overflow, "Communication energy metric overflows");
    }
    total = core::joules(sum);
}

void validate_transmission_result(const OneHopTransmissionResult& result,
                                  const LocalMessage& message) {
    const auto status_consistent = (result.status == OneHopDeliveryStatus::delivered &&
                                    result.drop_reason == OneHopDropReason::none) ||
                                   (result.status == OneHopDeliveryStatus::dropped &&
                                    result.drop_reason != OneHopDropReason::none);
    if (result.link_id.empty() || result.message_id != message.message_id ||
        result.departed_at != message.created_at || result.completed_at < result.departed_at ||
        result.latency != result.completed_at - result.departed_at ||
        result.size_bytes != message.size_bytes || !nonnegative_finite(result.link_energy) ||
        !status_consistent) {
        invalid(core::ErrorCode::invariant_violated,
                "One-hop implementation returned an inconsistent transmission result");
    }
}

} // namespace

std::string_view to_string(const ScheduledLinkOutcome outcome) noexcept {
    switch (outcome) {
    case ScheduledLinkOutcome::delivered:
        return "delivered";
    case ScheduledLinkOutcome::lost:
        return "lost";
    case ScheduledLinkOutcome::corrupted:
        return "corrupted";
    }
    return "unknown";
}

std::string_view to_string(const OneHopDeliveryStatus status) noexcept {
    switch (status) {
    case OneHopDeliveryStatus::delivered:
        return "delivered";
    case OneHopDeliveryStatus::dropped:
        return "dropped";
    }
    return "unknown";
}

std::string_view to_string(const OneHopDropReason reason) noexcept {
    switch (reason) {
    case OneHopDropReason::none:
        return "none";
    case OneHopDropReason::loss:
        return "loss";
    case OneHopDropReason::corruption:
        return "corruption";
    case OneHopDropReason::expired:
        return "expired";
    }
    return "unknown";
}

void validate_scheduled_one_hop_link_config(const ScheduledOneHopLinkConfig& config) {
    if (config.link_id.empty() || config.latency < core::SimulationClock::Duration::zero() ||
        !nonnegative_finite(config.energy_per_attempt) || config.repeating_outcomes.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Scheduled one-hop link requires identity, latency, energy, and outcomes");
    }
}

double delivery_fraction(const CommunicationMetrics& metrics) noexcept {
    return fraction(metrics.delivered_messages, metrics.attempted_messages);
}

double drop_fraction(const CommunicationMetrics& metrics) noexcept {
    return fraction(metrics.lost_messages + metrics.corrupted_messages + metrics.expired_messages,
                    metrics.attempted_messages);
}

double channel_loss_fraction(const CommunicationMetrics& metrics) noexcept {
    return fraction(metrics.lost_messages, metrics.attempted_messages);
}

double corruption_fraction(const CommunicationMetrics& metrics) noexcept {
    return fraction(metrics.corrupted_messages, metrics.attempted_messages);
}

core::SimulationClock::Duration
mean_delivery_latency(const CommunicationMetrics& metrics) noexcept {
    if (metrics.delivered_messages == 0) {
        return core::SimulationClock::Duration::zero();
    }
    using Rep = core::SimulationClock::Duration::rep;
    return core::SimulationClock::Duration{metrics.total_delivery_latency.count() /
                                           static_cast<Rep>(metrics.delivered_messages)};
}

ScheduledOneHopLink::ScheduledOneHopLink(ScheduledOneHopLinkConfig config)
    : config_{std::move(config)} {
    validate_scheduled_one_hop_link_config(config_);
}

std::string_view ScheduledOneHopLink::kind() const noexcept { return scheduled_one_hop_link_kind; }

std::string_view ScheduledOneHopLink::link_id() const noexcept { return config_.link_id; }

OneHopTransmissionResult ScheduledOneHopLink::transmit(const LocalMessage& message) {
    validate_local_message(message);
    if (message.created_at > core::SimulationClock::Duration::max() - config_.latency) {
        invalid(core::ErrorCode::numeric_overflow, "One-hop completion time overflows");
    }

    if (attempt_count_ == std::numeric_limits<std::uint64_t>::max()) {
        invalid(core::ErrorCode::numeric_overflow, "One-hop attempt counter overflows");
    }
    const auto completed_at = message.created_at + config_.latency;
    const auto index = attempt_count_ % config_.repeating_outcomes.size();
    const auto scheduled = config_.repeating_outcomes[index];
    ++attempt_count_;

    auto status = OneHopDeliveryStatus::dropped;
    auto reason = OneHopDropReason::none;
    if (completed_at > valid_until(message)) {
        reason = OneHopDropReason::expired;
    } else if (scheduled == ScheduledLinkOutcome::delivered) {
        status = OneHopDeliveryStatus::delivered;
    } else if (scheduled == ScheduledLinkOutcome::lost) {
        reason = OneHopDropReason::loss;
    } else {
        reason = OneHopDropReason::corruption;
    }

    return {config_.link_id, message.message_id, status,
            reason,          message.created_at, completed_at,
            config_.latency, message.size_bytes, config_.energy_per_attempt};
}

OneHopCommunicationSession::OneHopCommunicationSession(OneHopLinkModel& link) noexcept
    : link_{link} {}

OneHopExchangeResult OneHopCommunicationSession::exchange(Nanodevice& source, Nanodevice& target,
                                                          const LocalMessageRequest& request) {
    const auto source_energy_before = source.remaining_energy();
    auto message = source.emit_local_message(request);
    const auto transmitter_energy = source_energy_before - source.remaining_energy();

    auto transmission = link_.transmit(message);
    validate_transmission_result(transmission, message);
    if (transmission.link_id != link_.link_id()) {
        invalid(core::ErrorCode::invariant_violated,
                "One-hop result does not identify the selected link implementation");
    }
    auto receiver_energy = core::joules(0.0);
    if (transmission.status == OneHopDeliveryStatus::delivered) {
        const auto receiver_energy_before = target.remaining_energy();
        target.receive_local_message(message, transmission.completed_at);
        receiver_energy = receiver_energy_before - target.remaining_energy();
    }
    record(transmission, {transmitter_energy, receiver_energy});
    return {std::move(message), std::move(transmission)};
}

const CommunicationMetrics& OneHopCommunicationSession::metrics() const noexcept {
    return metrics_;
}

void OneHopCommunicationSession::record(const OneHopTransmissionResult& transmission,
                                        const EndpointEnergyUse& endpoint_energy) {
    add_count(metrics_.attempted_messages, 1);
    add_count(metrics_.attempted_bytes, transmission.size_bytes);
    add_energy(metrics_.transmitter_energy, endpoint_energy.transmitter);
    add_energy(metrics_.receiver_energy, endpoint_energy.receiver);
    add_energy(metrics_.link_energy, transmission.link_energy);

    if (transmission.status == OneHopDeliveryStatus::delivered) {
        add_count(metrics_.delivered_messages, 1);
        add_count(metrics_.delivered_bytes, transmission.size_bytes);
        add_duration(metrics_.total_delivery_latency, transmission.latency);
        metrics_.maximum_delivery_latency =
            std::max(metrics_.maximum_delivery_latency, transmission.latency);
        return;
    }
    switch (transmission.drop_reason) {
    case OneHopDropReason::loss:
        add_count(metrics_.lost_messages, 1);
        break;
    case OneHopDropReason::corruption:
        add_count(metrics_.corrupted_messages, 1);
        break;
    case OneHopDropReason::expired:
        add_count(metrics_.expired_messages, 1);
        break;
    case OneHopDropReason::none:
        invalid(core::ErrorCode::invariant_violated,
                "A dropped one-hop transmission requires a drop reason");
    }
}

} // namespace mehlissa::models::iot

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_ONE_HOP_LINK_HPP
#define MEHLISSA_MODELS_IOT_ONE_HOP_LINK_HPP

#include <mehlissa/models/iot/nanodevice.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::iot {

inline constexpr std::string_view scheduled_one_hop_link_kind = "scheduled_one_hop_link";

enum class ScheduledLinkOutcome : std::uint8_t { delivered, lost, corrupted };
enum class OneHopDeliveryStatus : std::uint8_t { delivered, dropped };
enum class OneHopDropReason : std::uint8_t { none, loss, corruption, expired };

[[nodiscard]] std::string_view to_string(ScheduledLinkOutcome outcome) noexcept;
[[nodiscard]] std::string_view to_string(OneHopDeliveryStatus status) noexcept;
[[nodiscard]] std::string_view to_string(OneHopDropReason reason) noexcept;

struct ScheduledOneHopLinkConfig final {
    std::string link_id;
    core::SimulationClock::Duration latency{};
    core::Energy energy_per_attempt{};
    std::vector<ScheduledLinkOutcome> repeating_outcomes;
};

void validate_scheduled_one_hop_link_config(const ScheduledOneHopLinkConfig& config);

struct OneHopTransmissionResult final {
    std::string link_id;
    std::string message_id;
    OneHopDeliveryStatus status{OneHopDeliveryStatus::dropped};
    OneHopDropReason drop_reason{OneHopDropReason::none};
    core::SimulationClock::Duration departed_at{};
    core::SimulationClock::Duration completed_at{};
    core::SimulationClock::Duration latency{};
    std::uint64_t size_bytes{};
    core::Energy link_energy{};
};

struct CommunicationMetrics final {
    std::uint64_t attempted_messages{};
    std::uint64_t delivered_messages{};
    std::uint64_t lost_messages{};
    std::uint64_t corrupted_messages{};
    std::uint64_t expired_messages{};
    std::uint64_t attempted_bytes{};
    std::uint64_t delivered_bytes{};
    core::SimulationClock::Duration total_delivery_latency{};
    core::SimulationClock::Duration maximum_delivery_latency{};
    core::Energy transmitter_energy{};
    core::Energy receiver_energy{};
    core::Energy link_energy{};
};

[[nodiscard]] double delivery_fraction(const CommunicationMetrics& metrics) noexcept;
[[nodiscard]] double drop_fraction(const CommunicationMetrics& metrics) noexcept;
[[nodiscard]] double channel_loss_fraction(const CommunicationMetrics& metrics) noexcept;
[[nodiscard]] double corruption_fraction(const CommunicationMetrics& metrics) noexcept;
[[nodiscard]] core::SimulationClock::Duration
mean_delivery_latency(const CommunicationMetrics& metrics) noexcept;

class OneHopLinkModel {
  public:
    virtual ~OneHopLinkModel() = default;

    [[nodiscard]] virtual std::string_view kind() const noexcept = 0;
    [[nodiscard]] virtual std::string_view link_id() const noexcept = 0;
    [[nodiscard]] virtual OneHopTransmissionResult transmit(const LocalMessage& message) = 0;
};

class ScheduledOneHopLink final : public OneHopLinkModel {
  public:
    explicit ScheduledOneHopLink(ScheduledOneHopLinkConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view link_id() const noexcept override;
    [[nodiscard]] OneHopTransmissionResult transmit(const LocalMessage& message) override;

  private:
    ScheduledOneHopLinkConfig config_;
    std::uint64_t attempt_count_{};
};

struct OneHopExchangeResult final {
    LocalMessage message;
    OneHopTransmissionResult transmission;
};

class OneHopCommunicationSession final {
  public:
    explicit OneHopCommunicationSession(OneHopLinkModel& link) noexcept;

    [[nodiscard]] OneHopExchangeResult exchange(Nanodevice& source, Nanodevice& target,
                                                const LocalMessageRequest& request);
    [[nodiscard]] const CommunicationMetrics& metrics() const noexcept;

  private:
    struct EndpointEnergyUse final {
        core::Energy transmitter{};
        core::Energy receiver{};
    };

    void record(const OneHopTransmissionResult& transmission,
                const EndpointEnergyUse& endpoint_energy);

    OneHopLinkModel& link_;
    CommunicationMetrics metrics_;
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_ONE_HOP_LINK_HPP

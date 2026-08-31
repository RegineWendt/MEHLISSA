// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_DRUG_DELIVERY_HPP
#define MEHLISSA_MODELS_CELL_DRUG_DELIVERY_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/models/cell/intracellular_response_network.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace mehlissa::models::cell {

inline constexpr std::string_view nanodevice_activation_contract_version = "1.0.0";
inline constexpr std::string_view analytical_conservative_drug_delivery_kind =
    "analytical_conservative_release_uptake";

struct NanodeviceActivationTarget final {
    std::string activation_id;
    std::string nanodevice_id;
    std::string payload_id;
};

struct NanodeviceActivationSignal final {
    std::string contract_version;
    std::string activation_id;
    std::string nanodevice_id;
    std::string payload_id;
    std::string source_request_id;
    std::string source_network_id;
    core::SimulationClock::Duration trigger_offset{};

    [[nodiscard]] bool operator==(const NanodeviceActivationSignal&) const noexcept = default;
};

void validate_nanodevice_activation_signal(const NanodeviceActivationSignal& signal);

[[nodiscard]] std::optional<NanodeviceActivationSignal>
make_nanodevice_activation_signal(const IntracellularOdeResponse& response,
                                  const NanodeviceActivationTarget& target);
[[nodiscard]] std::optional<NanodeviceActivationSignal>
make_nanodevice_activation_signal(const IntracellularSsaResponse& response,
                                  const NanodeviceActivationTarget& target);

struct DrugDeliveryConfig final {
    std::string model_id;
    std::string nanodevice_id;
    std::string payload_id;
    std::string drug_id;
    core::Amount loaded_amount{};
    core::FirstOrderRate release_rate{};
    core::FirstOrderRate uptake_rate{};
};

struct DrugDeliveryRequest final {
    std::string request_id;
    core::SimulationClock::Duration observation_after_activation{};
    std::optional<NanodeviceActivationSignal> activation;
};

struct DrugDeliveryResponse final {
    std::string request_id;
    std::string model_id;
    std::string drug_id;
    bool activated{};
    std::optional<core::SimulationClock::Duration> activation_offset;
    core::SimulationClock::Duration observation_after_activation{};
    core::Amount initial_payload_amount{};
    core::Amount device_payload_amount{};
    core::Amount extracellular_drug_amount{};
    core::Amount intracellular_drug_amount{};
    core::Amount released_drug_amount{};
    core::Amount balance_error{};
};

class AnalyticalDrugDeliveryModel final {
  public:
    explicit AnalyticalDrugDeliveryModel(DrugDeliveryConfig config);

    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] DrugDeliveryResponse evaluate(const DrugDeliveryRequest& request) const;

  private:
    DrugDeliveryConfig config_;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_DRUG_DELIVERY_HPP

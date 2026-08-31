// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_EXTRACELLULAR_SIGNAL_HPP
#define MEHLISSA_MODELS_COUPLING_EXTRACELLULAR_SIGNAL_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <string>
#include <string_view>

namespace mehlissa::models::coupling {

inline constexpr std::string_view extracellular_signal_contract_version = "1.0.0";
inline constexpr std::string_view non_consuming_uniform_inventory_snapshot =
    "non_consuming_uniform_inventory_snapshot";

struct ExtracellularSignalObservationRequest final {
    std::string contract_version;
    std::string sample_id;
    std::string signal_id;
    std::string source_compartment_id;
    core::Volume represented_volume{};
    core::SimulationClock::Duration observed_at{};
    core::SimulationClock::Duration valid_for{};
};

struct ExtracellularSignalSample final {
    std::string contract_version;
    std::string sample_id;
    std::string signal_id;
    std::string source_model_id;
    std::string source_compartment_id;
    std::string sampling_semantics;
    core::Amount represented_amount{};
    core::Volume represented_volume{};
    core::SimulationClock::Duration observed_at{};
    core::SimulationClock::Duration valid_for{};
};

void validate_extracellular_signal_request(const ExtracellularSignalObservationRequest& request);
void validate_extracellular_signal_sample(const ExtracellularSignalSample& sample);

[[nodiscard]] core::Concentration
extracellular_signal_concentration(const ExtracellularSignalSample& sample);
[[nodiscard]] core::SimulationClock::Duration
extracellular_signal_valid_until(const ExtracellularSignalSample& sample);

class ExtracellularSignalSource {
  public:
    virtual ~ExtracellularSignalSource() = default;

    [[nodiscard]] virtual std::string_view signal_source_model_id() const noexcept = 0;
    [[nodiscard]] virtual ExtracellularSignalSample
    observe_extracellular_signal(const ExtracellularSignalObservationRequest& request) const = 0;
};

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_EXTRACELLULAR_SIGNAL_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <string>

namespace mehlissa::models::capillary {

struct CapillaryExchangeRecord final {
    std::string transfer_id;
    std::string substance_id;
    std::string profile_id;
    core::SimulationClock::Duration reported_at{};
    core::Amount incoming_blood_amount{};
    core::Amount outgoing_blood_amount{};
    core::Amount endothelium_amount{};
    core::Amount interstitium_amount{};
    core::Amount cell_amount{};
};

struct CapillaryTissueInventory final {
    core::Amount endothelium_amount{};
    core::Amount interstitium_amount{};
    core::Amount cell_amount{};
};

[[nodiscard]] core::Amount total_accounted_amount(const CapillaryExchangeRecord& record) noexcept;
[[nodiscard]] double exchange_balance_error_moles(const CapillaryExchangeRecord& record) noexcept;
[[nodiscard]] bool is_balanced(const CapillaryExchangeRecord& record) noexcept;

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_EXCHANGE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_exchange.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace mehlissa::models::capillary {

core::Amount total_accounted_amount(const CapillaryExchangeRecord& record) noexcept {
    return record.outgoing_blood_amount + record.endothelium_amount + record.interstitium_amount +
           record.cell_amount;
}

double exchange_balance_error_moles(const CapillaryExchangeRecord& record) noexcept {
    return std::abs(core::in_moles(record.incoming_blood_amount) -
                    core::in_moles(total_accounted_amount(record)));
}

bool is_balanced(const CapillaryExchangeRecord& record) noexcept {
    constexpr double relative_tolerance = 1.0e-12;
    const auto scale = std::max(std::numeric_limits<double>::min(),
                                std::abs(core::in_moles(record.incoming_blood_amount)));
    return exchange_balance_error_moles(record) <= relative_tolerance * scale;
}

} // namespace mehlissa::models::capillary

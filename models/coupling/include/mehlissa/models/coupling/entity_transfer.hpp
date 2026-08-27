// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_ENTITY_TRANSFER_HPP
#define MEHLISSA_MODELS_COUPLING_ENTITY_TRANSFER_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace mehlissa::models::coupling {

inline constexpr std::string_view entity_transfer_contract_version = "1.0.0";

struct EntityTransfer final {
    std::string contract_version;
    std::uint64_t entity_id{};
    std::string entity_type;
    std::string source_model_id;
    std::string source_port_id;
    std::string target_model_id;
    std::string target_port_id;
    core::SimulationClock::Duration emitted_at{};

    [[nodiscard]] bool operator==(const EntityTransfer&) const noexcept = default;
};

void validate_entity_transfer(const EntityTransfer& transfer);

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_ENTITY_TRANSFER_HPP

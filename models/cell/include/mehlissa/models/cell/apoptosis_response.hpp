// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_HPP
#define MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_HPP

#include <mehlissa/models/cell/drug_delivery.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace mehlissa::models::cell {

inline constexpr std::string_view synthetic_hill_apoptosis_kind =
    "synthetic_hill_apoptosis_commitment";

enum class CellState : std::uint8_t { viable, apoptosis_committed };

[[nodiscard]] std::string_view to_string(CellState state) noexcept;

struct ApoptosisResponseConfig final {
    std::string model_id;
    std::string cell_id;
    std::string drug_id;
    core::Amount half_max_effect_amount{};
    double hill_coefficient{};
    double apoptosis_commitment_threshold{};
};

struct ApoptosisResponseRequest final {
    std::string request_id;
    DrugDeliveryResponse delivery;
};

struct ApoptosisResponse final {
    std::string request_id;
    std::string model_id;
    std::string cell_id;
    std::string drug_id;
    std::string source_delivery_request_id;
    std::string source_delivery_model_id;
    bool delivery_activated{};
    core::SimulationClock::Duration observed_at{};
    core::Amount intracellular_drug_amount{};
    double effect_fraction{};
    CellState state{CellState::viable};
};

void validate_apoptosis_response(const ApoptosisResponse& response);

class SyntheticHillApoptosisModel final {
  public:
    explicit SyntheticHillApoptosisModel(ApoptosisResponseConfig config);

    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] ApoptosisResponse evaluate(const ApoptosisResponseRequest& request) const;

  private:
    ApoptosisResponseConfig config_;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_HPP

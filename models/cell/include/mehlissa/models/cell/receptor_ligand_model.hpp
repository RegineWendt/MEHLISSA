// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_MODEL_HPP
#define MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_MODEL_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace mehlissa::models::cell {

struct ReceptorLigandModelConfig final {
    std::string model_id;
    std::string receptor_id;
    std::string ligand_id;
    std::string compartment_id;
    core::Volume cell_volume{};
    core::Concentration total_receptor_concentration{};
    core::SecondOrderAssociationRate association_rate{};
    core::FirstOrderRate dissociation_rate{};
    double detection_threshold_fraction{};
};

struct ReceptorLigandRequest final {
    std::string request_id;
    std::string ligand_id;
    std::string compartment_id;
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration observation_time{};
    double initial_bound_fraction{};
};

struct ReceptorLigandResponse final {
    std::string request_id;
    std::string receptor_id;
    std::string ligand_id;
    std::string cell_model_id;
    std::string compartment_id;
    core::SimulationClock::Duration observation_time{};
    core::Amount total_receptor_amount{};
    core::Amount free_receptor_amount{};
    core::Amount bound_receptor_amount{};
    double equilibrium_bound_fraction{};
    double final_bound_fraction{};
    bool detection_threshold_reached{};
    std::optional<core::SimulationClock::Duration> first_threshold_crossing_time;
};

class ReceptorLigandModel {
  public:
    virtual ~ReceptorLigandModel() = default;

    [[nodiscard]] virtual std::string_view kind() const noexcept = 0;
    [[nodiscard]] virtual std::string_view model_id() const noexcept = 0;
    [[nodiscard]] virtual ReceptorLigandResponse
    evaluate(const ReceptorLigandRequest& request) const = 0;
};

[[nodiscard]] core::Amount
accounted_receptor_amount(const ReceptorLigandResponse& response) noexcept;
[[nodiscard]] double receptor_balance_error_moles(const ReceptorLigandResponse& response) noexcept;

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_MODEL_HPP

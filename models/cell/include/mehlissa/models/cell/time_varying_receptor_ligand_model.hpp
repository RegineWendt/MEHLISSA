// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_MODEL_HPP
#define MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_MODEL_HPP

#include <mehlissa/models/cell/receptor_ligand_model.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr std::string_view time_varying_receptor_ligand_kind =
    "rk4_piecewise_constant_receptor_ligand";

struct LigandConcentrationKnot final {
    core::SimulationClock::Duration offset{};
    core::Concentration concentration{};
};

struct TimeVaryingReceptorLigandModelConfig final {
    ReceptorLigandModelConfig binding;
    core::SimulationClock::Duration integration_step{};
    std::size_t maximum_integration_steps{};
};

struct TimeVaryingReceptorLigandRequest final {
    std::string request_id;
    std::string ligand_id;
    std::string compartment_id;
    core::SimulationClock::Duration observation_time{};
    double initial_bound_fraction{};
    std::vector<LigandConcentrationKnot> ligand_trajectory;
};

struct ReceptorBindingSample final {
    core::SimulationClock::Duration offset{};
    core::Concentration ligand_concentration{};
    double bound_fraction{};
};

struct TimeVaryingReceptorLigandResponse final {
    std::string request_id;
    std::string receptor_id;
    std::string ligand_id;
    std::string cell_model_id;
    std::string compartment_id;
    core::SimulationClock::Duration observation_time{};
    core::Amount total_receptor_amount{};
    core::Amount free_receptor_amount{};
    core::Amount bound_receptor_amount{};
    double final_bound_fraction{};
    double peak_bound_fraction{};
    bool detection_threshold_reached{};
    std::optional<core::SimulationClock::Duration> first_threshold_crossing_time;
    std::size_t integration_steps{};
    std::vector<ReceptorBindingSample> samples;
};

void validate_time_varying_receptor_ligand_config(
    const TimeVaryingReceptorLigandModelConfig& config);

class TimeVaryingReceptorLigandModel final {
  public:
    explicit TimeVaryingReceptorLigandModel(TimeVaryingReceptorLigandModelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] std::string_view model_id() const noexcept;
    [[nodiscard]] TimeVaryingReceptorLigandResponse
    evaluate(const TimeVaryingReceptorLigandRequest& request) const;

  private:
    TimeVaryingReceptorLigandModelConfig config_;
};

[[nodiscard]] core::Amount
accounted_receptor_amount(const TimeVaryingReceptorLigandResponse& response) noexcept;
[[nodiscard]] double
receptor_balance_error_moles(const TimeVaryingReceptorLigandResponse& response) noexcept;

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_MODEL_HPP

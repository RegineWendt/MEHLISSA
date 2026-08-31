// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_MODEL_HPP
#define MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_MODEL_HPP

#include <mehlissa/core/random_stream.hpp>
#include <mehlissa/models/cell/receptor_ligand_model.hpp>
#include <mehlissa/models/cell/time_varying_receptor_ligand_model.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr std::string_view stochastic_receptor_ligand_kind =
    "gillespie_finite_receptor_ligand";

struct StochasticReceptorLigandModelConfig final {
    std::string model_id;
    std::string receptor_id;
    std::string ligand_id;
    std::string compartment_id;
    std::uint32_t receptor_count{};
    core::SecondOrderAssociationRate association_rate{};
    core::FirstOrderRate dissociation_rate{};
    double detection_threshold_fraction{};
    std::size_t maximum_reaction_events{};
    std::size_t maximum_recorded_samples{};
};

struct StochasticReceptorLigandRequest final {
    std::string request_id;
    std::string ligand_id;
    std::string compartment_id;
    core::SimulationClock::Duration observation_time{};
    std::uint32_t initial_bound_receptors{};
    std::vector<LigandConcentrationKnot> ligand_trajectory;
};

struct StochasticBindingSample final {
    core::SimulationClock::Duration offset{};
    std::uint32_t bound_receptors{};
};

struct StochasticReceptorLigandResponse final {
    std::string request_id;
    std::string receptor_id;
    std::string ligand_id;
    std::string cell_model_id;
    std::string compartment_id;
    core::SimulationClock::Duration observation_time{};
    std::uint32_t total_receptors{};
    std::uint32_t free_receptors{};
    std::uint32_t bound_receptors{};
    double final_bound_fraction{};
    double peak_bound_fraction{};
    bool detection_threshold_reached{};
    std::optional<core::SimulationClock::Duration> first_threshold_crossing_time;
    std::size_t reaction_events{};
    std::uint64_t random_draws{};
    std::size_t dropped_samples{};
    std::vector<StochasticBindingSample> samples;
};

void validate_stochastic_receptor_ligand_config(const StochasticReceptorLigandModelConfig& config);

class StochasticReceptorLigandModel final {
  public:
    explicit StochasticReceptorLigandModel(StochasticReceptorLigandModelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] std::string_view model_id() const noexcept;
    [[nodiscard]] StochasticReceptorLigandResponse
    evaluate(const StochasticReceptorLigandRequest& request, core::RandomStream& random) const;

  private:
    StochasticReceptorLigandModelConfig config_;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_MODEL_HPP

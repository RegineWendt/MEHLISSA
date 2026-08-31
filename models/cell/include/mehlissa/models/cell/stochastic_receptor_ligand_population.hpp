// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_POPULATION_HPP
#define MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_POPULATION_HPP

#include <mehlissa/models/cell/stochastic_receptor_ligand_model.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace mehlissa::models::cell {

struct StochasticPopulationConfig final {
    std::uint64_t master_seed{};
    std::string stream_prefix;
    std::size_t cells_per_cohort{};
};

struct StochasticPopulationRequest final {
    StochasticReceptorLigandRequest signal_positive;
    StochasticReceptorLigandRequest signal_negative;
};

struct PopulationDistribution final {
    std::size_t cells{};
    double mean_final_bound_fraction{};
    double variance_final_bound_fraction{};
    double minimum_final_bound_fraction{};
    double quantile_05{};
    double median{};
    double quantile_95{};
    double maximum_final_bound_fraction{};
    std::size_t detected_cells{};
    std::size_t total_reaction_events{};
    std::uint64_t total_random_draws{};
};

struct DetectionConfusionMatrix final {
    std::size_t true_positive{};
    std::size_t false_negative{};
    std::size_t false_positive{};
    std::size_t true_negative{};
    double sensitivity{};
    double specificity{};
    double false_negative_rate{};
    double false_positive_rate{};
};

struct StochasticPopulationResponse final {
    PopulationDistribution signal_positive;
    PopulationDistribution signal_negative;
    DetectionConfusionMatrix classification;
};

[[nodiscard]] StochasticPopulationResponse
evaluate_stochastic_population(const StochasticReceptorLigandModel& model,
                               const StochasticPopulationConfig& config,
                               const StochasticPopulationRequest& request);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_POPULATION_HPP

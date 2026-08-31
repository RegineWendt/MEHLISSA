// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/stochastic_receptor_ligand_population.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

namespace mehlissa::models::cell {
namespace {

struct CohortRun final {
    PopulationDistribution distribution;
    std::size_t detected{};
};

[[nodiscard]] double quantile(const std::vector<double>& sorted, const double probability) {
    const auto index =
        static_cast<std::size_t>(std::floor(probability * static_cast<double>(sorted.size() - 1)));
    return sorted[index];
}

[[nodiscard]] CohortRun run_cohort(const StochasticReceptorLigandModel& model,
                                   const StochasticPopulationConfig& config,
                                   const StochasticReceptorLigandRequest& request,
                                   const std::string& cohort_name) {
    std::vector<double> fractions;
    fractions.reserve(config.cells_per_cohort);
    std::size_t detected = 0;
    std::size_t events = 0;
    std::uint64_t draws = 0;
    for (std::size_t index = 0; index < config.cells_per_cohort; ++index) {
        core::RandomStream random{config.master_seed, config.stream_prefix + "." + cohort_name +
                                                          "." + std::to_string(index)};
        const auto response = model.evaluate(request, random);
        fractions.push_back(response.final_bound_fraction);
        detected += response.detection_threshold_reached ? 1U : 0U;
        events += response.reaction_events;
        draws += response.random_draws;
    }
    std::sort(fractions.begin(), fractions.end());
    const auto mean = std::accumulate(fractions.begin(), fractions.end(), 0.0) /
                      static_cast<double>(fractions.size());
    auto variance = 0.0;
    for (const auto value : fractions) {
        const auto difference = value - mean;
        variance += difference * difference;
    }
    variance /= static_cast<double>(fractions.size());
    return {{fractions.size(), mean, variance, fractions.front(), quantile(fractions, 0.05),
             quantile(fractions, 0.5), quantile(fractions, 0.95), fractions.back(), detected,
             events, draws},
            detected};
}

} // namespace

StochasticPopulationResponse
evaluate_stochastic_population(const StochasticReceptorLigandModel& model,
                               const StochasticPopulationConfig& config,
                               const StochasticPopulationRequest& request) {
    if (config.stream_prefix.empty() || config.cells_per_cohort < 2 ||
        config.cells_per_cohort > 1'000'000) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Stochastic population configuration is invalid"};
    }
    const auto positive = run_cohort(model, config, request.signal_positive, "positive");
    const auto negative = run_cohort(model, config, request.signal_negative, "negative");
    const auto positives = static_cast<double>(config.cells_per_cohort);
    const auto true_positive = positive.detected;
    const auto false_negative = config.cells_per_cohort - true_positive;
    const auto false_positive = negative.detected;
    const auto true_negative = config.cells_per_cohort - false_positive;
    return {positive.distribution,
            negative.distribution,
            {true_positive, false_negative, false_positive, true_negative,
             static_cast<double>(true_positive) / positives,
             static_cast<double>(true_negative) / positives,
             static_cast<double>(false_negative) / positives,
             static_cast<double>(false_positive) / positives}};
}

} // namespace mehlissa::models::cell

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_HPP
#define MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_HPP

#include <mehlissa/models/cell/apoptosis_response.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr std::string_view cohort_compressed_apoptosis_population_kind =
    "cohort_compressed_synthetic_hill_apoptosis_population";

struct ApoptosisPopulationConfig final {
    std::string model_id;
    std::string population_id;
    std::string drug_id;
    core::Amount half_max_effect_amount{};
    double hill_coefficient{};
    double apoptosis_commitment_threshold{};
};

struct ApoptosisAmountCohort final {
    std::string cohort_id;
    std::uint64_t cell_count{};
    core::Amount intracellular_drug_amount{};
};

struct ApoptosisPopulationRequest final {
    std::string request_id;
    core::SimulationClock::Duration observed_at{};
    std::size_t maximum_reported_cohorts{};
    std::vector<ApoptosisAmountCohort> cohorts;
};

struct ApoptosisCohortResult final {
    std::string cohort_id;
    std::uint64_t cell_count{};
    core::Amount intracellular_drug_amount{};
    double effect_fraction{};
    CellState state{CellState::viable};
};

struct ApoptosisPopulationResponse final {
    std::string request_id;
    std::string model_id;
    std::string population_id;
    std::string drug_id;
    core::SimulationClock::Duration observed_at{};
    std::size_t evaluated_cohorts{};
    std::size_t omitted_cohort_results{};
    std::uint64_t total_cells{};
    std::uint64_t viable_cells{};
    std::uint64_t apoptosis_committed_cells{};
    double apoptosis_committed_fraction{};
    double cell_weighted_mean_effect_fraction{};
    std::vector<ApoptosisCohortResult> cohort_results;
};

void validate_apoptosis_population_response(const ApoptosisPopulationResponse& response);

class CohortCompressedApoptosisPopulationModel final {
  public:
    explicit CohortCompressedApoptosisPopulationModel(ApoptosisPopulationConfig config);

    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] ApoptosisPopulationResponse
    evaluate(const ApoptosisPopulationRequest& request) const;

  private:
    ApoptosisPopulationConfig config_;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_HPP

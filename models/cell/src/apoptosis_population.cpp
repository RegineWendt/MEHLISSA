// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/apoptosis_population.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::cell {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] bool fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

void checked_add(std::uint64_t& total, const std::uint64_t value) {
    if (value > std::numeric_limits<std::uint64_t>::max() - total) {
        invalid("Apoptosis-population cell count overflows");
    }
    total += value;
}

} // namespace

void validate_apoptosis_population_response(const ApoptosisPopulationResponse& response) {
    if (response.request_id.empty() || response.model_id.empty() ||
        response.population_id.empty() || response.drug_id.empty() ||
        response.observed_at < core::SimulationClock::Duration::zero() ||
        response.evaluated_cohorts == 0 || response.total_cells == 0 ||
        response.viable_cells > response.total_cells ||
        response.apoptosis_committed_cells > response.total_cells ||
        response.viable_cells != response.total_cells - response.apoptosis_committed_cells ||
        !fraction(response.apoptosis_committed_fraction) ||
        !fraction(response.cell_weighted_mean_effect_fraction) ||
        response.cohort_results.size() > response.evaluated_cohorts ||
        response.omitted_cohort_results !=
            response.evaluated_cohorts - response.cohort_results.size()) {
        invalid("Apoptosis-population response is incomplete or inconsistent");
    }

    for (const auto& cohort : response.cohort_results) {
        const auto amount = core::in_moles(cohort.intracellular_drug_amount);
        const auto valid_state =
            cohort.state == CellState::viable || cohort.state == CellState::apoptosis_committed;
        if (cohort.cohort_id.empty() || cohort.cell_count == 0 || !std::isfinite(amount) ||
            amount < 0.0 || !fraction(cohort.effect_fraction) || !valid_state) {
            invalid("Reported apoptosis-population cohort is invalid");
        }
    }
}

CohortCompressedApoptosisPopulationModel::CohortCompressedApoptosisPopulationModel(
    ApoptosisPopulationConfig config)
    : config_{std::move(config)} {
    const auto half_max = core::in_moles(config_.half_max_effect_amount);
    if (config_.model_id.empty() || config_.population_id.empty() || config_.drug_id.empty() ||
        !std::isfinite(half_max) || half_max <= 0.0 || !std::isfinite(config_.hill_coefficient) ||
        config_.hill_coefficient <= 0.0 || !std::isfinite(config_.apoptosis_commitment_threshold) ||
        config_.apoptosis_commitment_threshold <= 0.0 ||
        config_.apoptosis_commitment_threshold >= 1.0) {
        invalid("Apoptosis-population configuration is incomplete or nonphysical");
    }
}

std::string_view CohortCompressedApoptosisPopulationModel::kind() const noexcept {
    return cohort_compressed_apoptosis_population_kind;
}

ApoptosisPopulationResponse CohortCompressedApoptosisPopulationModel::evaluate(
    const ApoptosisPopulationRequest& request) const {
    if (request.request_id.empty() ||
        request.observed_at < core::SimulationClock::Duration::zero() ||
        request.maximum_reported_cohorts == 0 || request.cohorts.empty()) {
        invalid("Apoptosis-population request is incomplete or invalid");
    }

    std::unordered_set<std::string> cohort_ids;
    std::uint64_t total_cells = 0;
    for (const auto& cohort : request.cohorts) {
        const auto amount = core::in_moles(cohort.intracellular_drug_amount);
        if (cohort.cohort_id.empty() || cohort.cell_count == 0 || !std::isfinite(amount) ||
            amount < 0.0 || !cohort_ids.insert(cohort.cohort_id).second) {
            invalid("Apoptosis-population cohorts must be physical, nonempty, and unique");
        }
        checked_add(total_cells, cohort.cell_count);
    }

    ApoptosisPopulationResponse response{request.request_id,
                                         config_.model_id,
                                         config_.population_id,
                                         config_.drug_id,
                                         request.observed_at,
                                         request.cohorts.size(),
                                         0,
                                         total_cells,
                                         0,
                                         0,
                                         0.0,
                                         0.0,
                                         {}};
    response.cohort_results.reserve(
        std::min(request.maximum_reported_cohorts, request.cohorts.size()));

    long double weighted_effect_sum = 0.0;
    for (const auto& cohort : request.cohorts) {
        const auto effect =
            synthetic_hill_effect(cohort.intracellular_drug_amount, config_.half_max_effect_amount,
                                  config_.hill_coefficient);
        const auto state = effect >= config_.apoptosis_commitment_threshold
                               ? CellState::apoptosis_committed
                               : CellState::viable;
        if (state == CellState::apoptosis_committed) {
            checked_add(response.apoptosis_committed_cells, cohort.cell_count);
        } else {
            checked_add(response.viable_cells, cohort.cell_count);
        }
        weighted_effect_sum +=
            static_cast<long double>(cohort.cell_count) * static_cast<long double>(effect);
        if (response.cohort_results.size() < request.maximum_reported_cohorts) {
            response.cohort_results.push_back({cohort.cohort_id, cohort.cell_count,
                                               cohort.intracellular_drug_amount, effect, state});
        }
    }

    response.omitted_cohort_results = response.evaluated_cohorts - response.cohort_results.size();
    response.apoptosis_committed_fraction =
        static_cast<double>(response.apoptosis_committed_cells) /
        static_cast<double>(response.total_cells);
    response.cell_weighted_mean_effect_fraction = std::clamp(
        static_cast<double>(weighted_effect_sum / static_cast<long double>(response.total_cells)),
        0.0, 1.0);
    validate_apoptosis_population_response(response);
    return response;
}

} // namespace mehlissa::models::cell

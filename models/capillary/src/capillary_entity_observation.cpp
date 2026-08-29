// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_entity_observation.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>

namespace mehlissa::models::capillary {

core::SimulationClock::Duration
total_residence_time(const CapillaryEntityObservationRecord& record) noexcept {
    return std::accumulate(record.region_residence_times.begin(),
                           record.region_residence_times.end(),
                           core::SimulationClock::Duration::zero());
}

double total_outcome_likelihood(const CapillaryEntityObservationRecord& record) noexcept {
    return record.pass_through_likelihood + record.retention_likelihood +
           record.adhesion_likelihood + record.extravasation_likelihood;
}

bool has_normalized_outcome_likelihoods(const CapillaryEntityObservationRecord& record) noexcept {
    constexpr double tolerance = 1.0e-12;
    const std::array likelihoods{record.pass_through_likelihood, record.retention_likelihood,
                                 record.adhesion_likelihood, record.extravasation_likelihood};
    return std::all_of(likelihoods.begin(), likelihoods.end(),
                       [](const double value) {
                           return std::isfinite(value) && value >= 0.0 && value <= 1.0;
                       }) &&
           std::abs(total_outcome_likelihood(record) - 1.0) <= tolerance;
}

CapillaryEntityObservationRecord
make_capillary_entity_observation(CapillaryEntityObservationInput input,
                                  const CapillaryEntityInteractionRule* interaction_rule) {
    CapillaryEntityObservationRecord result{input.entity_id, std::move(input.entity_type),
                                            std::move(input.profile_id), input.reported_at,
                                            input.region_residence_times};
    if (interaction_rule == nullptr) {
        return result;
    }

    constexpr double seconds_per_nanosecond = 1.0e-9;
    const auto capillary_seconds =
        static_cast<double>(input.region_residence_times.at(1).count()) * seconds_per_nanosecond;
    const auto total_rate = interaction_rule->retention_rate_per_second +
                            interaction_rule->adhesion_rate_per_second +
                            interaction_rule->extravasation_rate_per_second;
    const auto valid_rate = [](const double value) { return std::isfinite(value) && value >= 0.0; };
    if (!valid_rate(interaction_rule->retention_rate_per_second) ||
        !valid_rate(interaction_rule->adhesion_rate_per_second) ||
        !valid_rate(interaction_rule->extravasation_rate_per_second) ||
        !std::isfinite(total_rate) || total_rate <= 0.0) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Capillary entity observation requires a positive finite combined interaction rate"};
    }
    const auto pass_through = std::exp(-total_rate * capillary_seconds);
    const auto interaction = 1.0 - pass_through;
    result.interaction_rule_applied = true;
    result.pass_through_likelihood = pass_through;
    result.retention_likelihood =
        interaction * interaction_rule->retention_rate_per_second / total_rate;
    result.adhesion_likelihood =
        interaction * interaction_rule->adhesion_rate_per_second / total_rate;
    result.extravasation_likelihood =
        interaction * interaction_rule->extravasation_rate_per_second / total_rate;
    return result;
}

} // namespace mehlissa::models::capillary

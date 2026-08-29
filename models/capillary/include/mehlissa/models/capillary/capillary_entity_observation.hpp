// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace mehlissa::models::capillary {

inline constexpr std::size_t capillary_observed_region_count = 3;

struct CapillaryEntityPosition final {
    std::uint64_t entity_id{};
    std::string entity_type;
    std::string region_id;
    std::string region_kind;
    core::Length axial_position{};
    double axial_fraction{};
    core::SimulationClock::Duration accumulated_residence_time{};
};

struct CapillaryEntityObservationRecord final {
    std::uint64_t entity_id{};
    std::string entity_type;
    std::string profile_id;
    core::SimulationClock::Duration reported_at{};
    std::array<core::SimulationClock::Duration, capillary_observed_region_count>
        region_residence_times{};
    bool interaction_rule_applied{};
    double pass_through_likelihood{1.0};
    double retention_likelihood{};
    double adhesion_likelihood{};
    double extravasation_likelihood{};
};

struct CapillaryEntityObservationInput final {
    std::uint64_t entity_id{};
    std::string entity_type;
    std::string profile_id;
    core::SimulationClock::Duration reported_at{};
    std::array<core::SimulationClock::Duration, capillary_observed_region_count>
        region_residence_times{};
};

[[nodiscard]] core::SimulationClock::Duration
total_residence_time(const CapillaryEntityObservationRecord& record) noexcept;
[[nodiscard]] double
total_outcome_likelihood(const CapillaryEntityObservationRecord& record) noexcept;
[[nodiscard]] bool
has_normalized_outcome_likelihoods(const CapillaryEntityObservationRecord& record) noexcept;
[[nodiscard]] CapillaryEntityObservationRecord
make_capillary_entity_observation(CapillaryEntityObservationInput input,
                                  const CapillaryEntityInteractionRule* interaction_rule);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_OBSERVATION_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_CASE_HPP
#define MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_CASE_HPP

#include <mehlissa/core/quantity.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto axial_advection_reaction_case_kind = "axial_advection_diffusion_reaction_1d";

struct AxialAdvectionReactionConfig final {
    std::string model_id;
    core::Length domain_length{};
    core::Length source_position{};
    core::Length receiver_center{};
    core::Length receiver_width{};
    core::Length lumen_radius{};
    core::Speed mean_advection_speed{};
    core::Diffusivity diffusion_coefficient{};
    core::FirstOrderRate bulk_reaction_rate{};
    core::Speed wall_interaction_velocity{};
    core::Time observation_time{};
};

struct AxialParticleConfig final {
    std::uint64_t sample_count{};
    std::uint64_t experiment_seed{};
    std::string random_stream_name;
};

struct AxialFieldConfig final {
    std::uint64_t cell_count{};
    double cfl_safety_factor{};
};

struct AxialAnalyticalEvaluation final {
    double receiver_amount_fraction{};
    double active_amount_fraction{};
    double bulk_reacted_amount_fraction{};
    double wall_reacted_amount_fraction{};
    double effective_wall_reaction_rate_per_second{};
    core::Length mean_position{};
    core::Length standard_deviation{};
    double conservation_residual{};
};

struct AxialParticleEvaluation final {
    std::uint64_t sample_count{};
    std::uint64_t receiver_observation_count{};
    std::uint64_t active_in_domain_count{};
    std::uint64_t escaped_count{};
    std::uint64_t bulk_reacted_count{};
    std::uint64_t wall_reacted_count{};
    double receiver_amount_fraction{};
    double active_amount_fraction{};
    double escaped_amount_fraction{};
    double bulk_reacted_amount_fraction{};
    double wall_reacted_amount_fraction{};
    double receiver_fraction_standard_error{};
    core::Length mean_active_position{};
    double conservation_residual{};
};

struct AxialFieldCell final {
    std::uint64_t cell_index{};
    core::Length left_boundary{};
    core::Length right_boundary{};
    double active_amount_fraction{};
    double normalized_amount_density_per_meter{};
};

struct AxialFieldEvaluation final {
    std::uint64_t cell_count{};
    std::uint64_t time_step_count{};
    double receiver_amount_fraction{};
    double active_amount_fraction{};
    double escaped_left_amount_fraction{};
    double escaped_right_amount_fraction{};
    double bulk_reacted_amount_fraction{};
    double wall_reacted_amount_fraction{};
    core::Length mean_active_position{};
    double conservation_residual{};
    std::vector<AxialFieldCell> final_field;
};

struct AxialAdvectionReactionVerificationGate final {
    std::uint64_t minimum_particle_receiver_count{};
    double maximum_particle_standardized_error{};
    double maximum_coarse_relative_reference_error{};
    double maximum_refined_relative_reference_error{};
    double maximum_relative_refinement_difference{};
    double maximum_relative_active_fraction_error{};
    double maximum_absolute_conservation_residual{};
    double maximum_escaped_amount_fraction{};
};

struct AxialAdvectionReactionVerificationResult final {
    AxialAnalyticalEvaluation reference;
    AxialParticleEvaluation particle;
    AxialFieldEvaluation coarse_field;
    AxialFieldEvaluation refined_field;
    double particle_reference_standardized_error{};
    double particle_relative_active_fraction_error{};
    double coarse_relative_reference_error{};
    double refined_relative_reference_error{};
    double relative_refinement_difference{};
    bool passes{};
};

[[nodiscard]] AxialAnalyticalEvaluation
evaluate_axial_advection_reaction_reference(const AxialAdvectionReactionConfig& config);

[[nodiscard]] AxialParticleEvaluation
evaluate_axial_advection_reaction_particles(const AxialAdvectionReactionConfig& config,
                                            const AxialParticleConfig& particle_config);

[[nodiscard]] AxialFieldEvaluation
evaluate_axial_advection_reaction_field(const AxialAdvectionReactionConfig& config,
                                        const AxialFieldConfig& field_config);

[[nodiscard]] AxialAdvectionReactionVerificationResult verify_axial_advection_reaction_case(
    const AxialAdvectionReactionConfig& config, const AxialParticleConfig& particle_config,
    const AxialFieldConfig& coarse_field_config, const AxialFieldConfig& refined_field_config,
    const AxialAdvectionReactionVerificationGate& gate);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_AXIAL_ADVECTION_REACTION_CASE_HPP

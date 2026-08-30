// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/trajectory_brownian_channel_verification.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace mehlissa::models::capillary {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double reference_standardized_error(const double observed, const double expected,
                                                  const std::uint64_t sample_count) {
    const auto standard_error =
        std::sqrt(expected * (1.0 - expected) / static_cast<double>(sample_count));
    return (observed - expected) / standard_error;
}

[[nodiscard]] double relative_error(const double observed, const double expected) {
    return std::abs(observed - expected) / expected;
}

} // namespace

TrajectoryBrownianVerificationResult verify_trajectory_brownian_refinement(
    const MolecularChannel& reference, const TrajectoryBrownianChannel& coarse,
    const TrajectoryBrownianChannel& refined, const MolecularChannelRequest& request,
    const TrajectoryBrownianVerificationGate& gate) {
    if (reference.model_id() == coarse.model_id() || reference.model_id() == refined.model_id() ||
        coarse.model_id() == refined.model_id() || gate.minimum_receiver_observation_count == 0 ||
        coarse.boundary_kind() != BrownianBoundaryKind::unbounded ||
        refined.boundary_kind() != BrownianBoundaryKind::unbounded ||
        !std::isfinite(gate.maximum_absolute_reference_standardized_error) ||
        gate.maximum_absolute_reference_standardized_error <= 0.0 ||
        !std::isfinite(gate.maximum_relative_fraction_error) ||
        gate.maximum_relative_fraction_error <= 0.0 || gate.maximum_relative_fraction_error > 1.0 ||
        !std::isfinite(gate.maximum_refinement_standardized_difference) ||
        gate.maximum_refinement_standardized_difference <= 0.0 ||
        !std::isfinite(gate.maximum_relative_mean_squared_displacement_error) ||
        gate.maximum_relative_mean_squared_displacement_error <= 0.0 ||
        gate.maximum_relative_mean_squared_displacement_error > 1.0) {
        invalid("Brownian trajectory verification gate or model identity is invalid");
    }

    const auto reference_response = reference.evaluate(request);
    const auto coarse_evaluation = coarse.evaluate_with_diagnostics(request);
    const auto refined_evaluation = refined.evaluate_with_diagnostics(request);
    if (reference_response.request_id != request.request_id ||
        coarse_evaluation.response.request_id != request.request_id ||
        refined_evaluation.response.request_id != request.request_id ||
        reference_response.signal_id != request.signal_id ||
        coarse_evaluation.response.signal_id != request.signal_id ||
        refined_evaluation.response.signal_id != request.signal_id ||
        reference_response.observation_time != request.observation_time ||
        coarse_evaluation.response.observation_time != request.observation_time ||
        refined_evaluation.response.observation_time != request.observation_time ||
        coarse_evaluation.sample_count == 0 || refined_evaluation.sample_count == 0 ||
        coarse_evaluation.step_count >= refined_evaluation.step_count ||
        refined_evaluation.step_count % coarse_evaluation.step_count != 0 ||
        coarse_evaluation.free_diffusion_expected_mean_squared_displacement_m2 !=
            refined_evaluation.free_diffusion_expected_mean_squared_displacement_m2) {
        invalid("Brownian trajectory evaluations violate the shared refinement contract");
    }

    const auto expected_fraction = reference_response.expected_receiver_fraction;
    const auto coarse_fraction = coarse_evaluation.response.expected_receiver_fraction;
    const auto refined_fraction = refined_evaluation.response.expected_receiver_fraction;
    if (!std::isfinite(expected_fraction) || expected_fraction <= 0.0 || expected_fraction >= 1.0 ||
        !std::isfinite(coarse_fraction) || coarse_fraction < 0.0 || coarse_fraction > 1.0 ||
        !std::isfinite(refined_fraction) || refined_fraction < 0.0 || refined_fraction > 1.0) {
        invalid("Brownian trajectory verification fractions are invalid");
    }

    const auto coarse_reference_error = reference_standardized_error(
        coarse_fraction, expected_fraction, coarse_evaluation.sample_count);
    const auto refined_reference_error = reference_standardized_error(
        refined_fraction, expected_fraction, refined_evaluation.sample_count);
    const auto coarse_relative_error = relative_error(coarse_fraction, expected_fraction);
    const auto refined_relative_error = relative_error(refined_fraction, expected_fraction);
    const auto refinement_standard_error =
        std::sqrt(coarse_fraction * (1.0 - coarse_fraction) /
                      static_cast<double>(coarse_evaluation.sample_count) +
                  refined_fraction * (1.0 - refined_fraction) /
                      static_cast<double>(refined_evaluation.sample_count));
    if (!std::isfinite(refinement_standard_error) || refinement_standard_error <= 0.0) {
        invalid("Brownian trajectory refinement standard error is invalid");
    }
    const auto refinement_difference =
        (refined_fraction - coarse_fraction) / refinement_standard_error;
    const auto expected_msd =
        coarse_evaluation.free_diffusion_expected_mean_squared_displacement_m2;
    if (!std::isfinite(expected_msd) || expected_msd <= 0.0) {
        invalid("Brownian trajectory expected mean squared displacement is invalid");
    }
    const auto coarse_msd_error =
        relative_error(coarse_evaluation.mean_squared_displacement_m2, expected_msd);
    const auto refined_msd_error =
        relative_error(refined_evaluation.mean_squared_displacement_m2, expected_msd);

    const auto passes =
        std::min(coarse_evaluation.receiver_observation_count,
                 refined_evaluation.receiver_observation_count) >=
            gate.minimum_receiver_observation_count &&
        std::max(std::abs(coarse_reference_error), std::abs(refined_reference_error)) <=
            gate.maximum_absolute_reference_standardized_error &&
        std::max(coarse_relative_error, refined_relative_error) <=
            gate.maximum_relative_fraction_error &&
        std::abs(refinement_difference) <= gate.maximum_refinement_standardized_difference &&
        std::max(coarse_msd_error, refined_msd_error) <=
            gate.maximum_relative_mean_squared_displacement_error;

    return {reference_response,
            coarse_evaluation,
            refined_evaluation,
            coarse_reference_error,
            refined_reference_error,
            coarse_relative_error,
            refined_relative_error,
            refinement_difference,
            coarse_msd_error,
            refined_msd_error,
            passes};
}

} // namespace mehlissa::models::capillary

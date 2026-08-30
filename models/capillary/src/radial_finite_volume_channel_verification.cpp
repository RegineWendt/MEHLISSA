// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/radial_finite_volume_channel_verification.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace mehlissa::models::capillary {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] double relative_error(const double observed, const double expected) {
    return std::abs(observed - expected) / expected;
}

} // namespace

RadialFiniteVolumeVerificationResult verify_radial_finite_volume_refinement(
    const MolecularChannel& reference, const RadialFiniteVolumeChannel& coarse,
    const RadialFiniteVolumeChannel& refined, const MolecularChannelRequest& request,
    const RadialFiniteVolumeVerificationGate& gate) {
    if (reference.model_id() == coarse.model_id() || reference.model_id() == refined.model_id() ||
        coarse.model_id() == refined.model_id() ||
        !std::isfinite(gate.maximum_coarse_relative_reference_error) ||
        gate.maximum_coarse_relative_reference_error <= 0.0 ||
        gate.maximum_coarse_relative_reference_error > 1.0 ||
        !std::isfinite(gate.maximum_refined_relative_reference_error) ||
        gate.maximum_refined_relative_reference_error <= 0.0 ||
        gate.maximum_refined_relative_reference_error >
            gate.maximum_coarse_relative_reference_error ||
        !std::isfinite(gate.maximum_relative_refinement_difference) ||
        gate.maximum_relative_refinement_difference <= 0.0 ||
        gate.maximum_relative_refinement_difference > 1.0 ||
        !std::isfinite(gate.maximum_absolute_conservation_residual) ||
        gate.maximum_absolute_conservation_residual <= 0.0 ||
        !std::isfinite(gate.maximum_escaped_amount_fraction) ||
        gate.maximum_escaped_amount_fraction < 0.0 || gate.maximum_escaped_amount_fraction > 1.0) {
        invalid("Radial finite-volume verification gate or model identity is invalid");
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
        coarse_evaluation.radial_cell_count >= refined_evaluation.radial_cell_count ||
        refined_evaluation.radial_cell_count % coarse_evaluation.radial_cell_count != 0) {
        invalid("Radial finite-volume evaluations violate the shared refinement contract");
    }

    const auto expected_fraction = reference_response.expected_receiver_fraction;
    const auto coarse_fraction = coarse_evaluation.response.expected_receiver_fraction;
    const auto refined_fraction = refined_evaluation.response.expected_receiver_fraction;
    if (!std::isfinite(expected_fraction) || expected_fraction <= 0.0 || expected_fraction >= 1.0 ||
        !std::isfinite(coarse_fraction) || coarse_fraction <= 0.0 || coarse_fraction >= 1.0 ||
        !std::isfinite(refined_fraction) || refined_fraction <= 0.0 || refined_fraction >= 1.0) {
        invalid("Radial finite-volume verification fractions are invalid");
    }

    const auto coarse_reference_error = relative_error(coarse_fraction, expected_fraction);
    const auto refined_reference_error = relative_error(refined_fraction, expected_fraction);
    const auto refinement_difference = relative_error(coarse_fraction, refined_fraction);
    const auto maximum_residual = std::max(std::abs(coarse_evaluation.conservation_residual),
                                           std::abs(refined_evaluation.conservation_residual));
    const auto maximum_escaped = std::max(coarse_evaluation.escaped_amount_fraction,
                                          refined_evaluation.escaped_amount_fraction);
    const auto passes = coarse_reference_error <= gate.maximum_coarse_relative_reference_error &&
                        refined_reference_error <= gate.maximum_refined_relative_reference_error &&
                        refinement_difference <= gate.maximum_relative_refinement_difference &&
                        maximum_residual <= gate.maximum_absolute_conservation_residual &&
                        maximum_escaped <= gate.maximum_escaped_amount_fraction;

    return {reference_response,
            coarse_evaluation,
            refined_evaluation,
            coarse_reference_error,
            refined_reference_error,
            refinement_difference,
            passes};
}

} // namespace mehlissa::models::capillary

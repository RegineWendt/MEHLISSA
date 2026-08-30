// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/molecular_channel_comparison.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <string>

namespace mehlissa::models::capillary {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

} // namespace

MolecularChannelComparisonResult compare_molecular_channels(
    const MolecularChannel& reference, const BrownianParticleChannel& particle,
    const MolecularChannelRequest& request, const MolecularChannelComparisonGate& gate) {
    if (reference.model_id() == particle.model_id() ||
        gate.minimum_receiver_observation_count == 0 ||
        !std::isfinite(gate.maximum_absolute_standardized_error) ||
        gate.maximum_absolute_standardized_error <= 0.0 ||
        !std::isfinite(gate.maximum_relative_fraction_error) ||
        gate.maximum_relative_fraction_error <= 0.0 || gate.maximum_relative_fraction_error > 1.0) {
        invalid("Molecular-channel comparison configuration is invalid");
    }

    const auto reference_response = reference.evaluate(request);
    const auto particle_evaluation = particle.evaluate_with_diagnostics(request);
    if (reference_response.request_id != request.request_id ||
        particle_evaluation.response.request_id != request.request_id ||
        reference_response.signal_id != request.signal_id ||
        particle_evaluation.response.signal_id != request.signal_id ||
        reference_response.observation_time != request.observation_time ||
        particle_evaluation.response.observation_time != request.observation_time ||
        particle_evaluation.sample_count == 0 ||
        gate.minimum_receiver_observation_count > particle_evaluation.sample_count) {
        invalid("Molecular-channel comparison responses violate the shared request contract");
    }
    const auto expected_fraction = reference_response.expected_receiver_fraction;
    if (!std::isfinite(expected_fraction) || expected_fraction <= 0.0 || expected_fraction >= 1.0) {
        invalid("Molecular-channel reference fraction must lie strictly between zero and one");
    }
    const auto observed_fraction = particle_evaluation.response.expected_receiver_fraction;
    if (!std::isfinite(observed_fraction) || observed_fraction < 0.0 || observed_fraction > 1.0) {
        invalid("Molecular-channel particle fraction must lie between zero and one");
    }
    const auto absolute_error = std::abs(observed_fraction - expected_fraction);
    const auto relative_error = absolute_error / expected_fraction;
    const auto null_standard_error =
        std::sqrt(expected_fraction * (1.0 - expected_fraction) /
                  static_cast<double>(particle_evaluation.sample_count));
    const auto standardized_error = (observed_fraction - expected_fraction) / null_standard_error;
    const auto passes =
        particle_evaluation.receiver_observation_count >= gate.minimum_receiver_observation_count &&
        std::abs(standardized_error) <= gate.maximum_absolute_standardized_error &&
        relative_error <= gate.maximum_relative_fraction_error;

    return {reference_response, particle_evaluation, absolute_error,
            relative_error,     standardized_error,  passes};
}

} // namespace mehlissa::models::capillary

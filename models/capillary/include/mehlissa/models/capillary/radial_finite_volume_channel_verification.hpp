// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_VERIFICATION_HPP
#define MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_VERIFICATION_HPP

#include <mehlissa/models/capillary/radial_finite_volume_channel.hpp>

namespace mehlissa::models::capillary {

struct RadialFiniteVolumeVerificationGate final {
    double maximum_coarse_relative_reference_error{};
    double maximum_refined_relative_reference_error{};
    double maximum_relative_refinement_difference{};
    double maximum_absolute_conservation_residual{};
    double maximum_escaped_amount_fraction{};
};

struct RadialFiniteVolumeVerificationResult final {
    MolecularChannelResponse reference;
    RadialFiniteVolumeChannelEvaluation coarse;
    RadialFiniteVolumeChannelEvaluation refined;
    double coarse_relative_reference_error{};
    double refined_relative_reference_error{};
    double relative_refinement_difference{};
    bool passes{};
};

[[nodiscard]] RadialFiniteVolumeVerificationResult verify_radial_finite_volume_refinement(
    const MolecularChannel& reference, const RadialFiniteVolumeChannel& coarse,
    const RadialFiniteVolumeChannel& refined, const MolecularChannelRequest& request,
    const RadialFiniteVolumeVerificationGate& gate);

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_VERIFICATION_HPP

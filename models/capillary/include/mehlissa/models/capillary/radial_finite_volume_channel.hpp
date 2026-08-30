// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_HPP
#define MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_HPP

#include <mehlissa/models/capillary/molecular_channel.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto radial_finite_volume_diffusion_3d_kind = "radial_finite_volume_diffusion_3d";

struct RadialFiniteVolumeChannelConfig final {
    std::string model_id;
    core::Diffusivity diffusion_coefficient{};
    core::FirstOrderRate first_order_degradation_rate{};
    std::uint64_t radial_cell_count{};
    core::Length radial_domain_radius{};
    double cfl_safety_factor{};
};

struct RadialConcentrationCell final {
    std::uint64_t cell_index{};
    core::Length inner_radius{};
    core::Length outer_radius{};
    double active_amount_fraction{};
    double normalized_concentration_per_cubic_meter{};
};

struct RadialFiniteVolumeChannelEvaluation final {
    MolecularChannelResponse response;
    std::uint64_t radial_cell_count{};
    std::uint64_t time_step_count{};
    core::Length radial_cell_width{};
    double time_step_seconds{};
    std::uint64_t receiver_lower_cell_index{};
    std::uint64_t receiver_upper_cell_index{};
    double receiver_interpolation_weight{};
    double active_amount_fraction{};
    double degraded_amount_fraction{};
    double escaped_amount_fraction{};
    double conservation_residual{};
    std::vector<RadialConcentrationCell> final_field;
};

class RadialFiniteVolumeChannel final : public MolecularChannel {
  public:
    explicit RadialFiniteVolumeChannel(RadialFiniteVolumeChannelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] MolecularChannelResponse
    evaluate(const MolecularChannelRequest& request) const override;
    [[nodiscard]] RadialFiniteVolumeChannelEvaluation
    evaluate_with_diagnostics(const MolecularChannelRequest& request) const;

  private:
    RadialFiniteVolumeChannelConfig config_;
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_RADIAL_FINITE_VOLUME_CHANNEL_HPP

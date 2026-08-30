// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_ANALYTICAL_DIFFUSION_CHANNEL_HPP
#define MEHLISSA_MODELS_CAPILLARY_ANALYTICAL_DIFFUSION_CHANNEL_HPP

#include <mehlissa/models/capillary/molecular_channel.hpp>

#include <string>

namespace mehlissa::models::capillary {

inline constexpr auto analytical_free_diffusion_3d_kind = "analytical_free_diffusion_3d";

struct AnalyticalDiffusionChannelConfig final {
    std::string model_id;
    core::Diffusivity diffusion_coefficient{};
    core::FirstOrderRate first_order_degradation_rate{};
    double maximum_receiver_radius_to_separation_ratio{};
};

class AnalyticalDiffusionChannel final : public MolecularChannel {
  public:
    explicit AnalyticalDiffusionChannel(AnalyticalDiffusionChannelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] MolecularChannelResponse
    evaluate(const MolecularChannelRequest& request) const override;

    [[nodiscard]] core::SimulationClock::Duration
    peak_observation_time(core::Length separation) const;

  private:
    AnalyticalDiffusionChannelConfig config_;
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_ANALYTICAL_DIFFUSION_CHANNEL_HPP

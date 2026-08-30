// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_HPP
#define MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_HPP

#include <mehlissa/models/capillary/molecular_channel.hpp>

#include <cstdint>
#include <string>

namespace mehlissa::models::capillary {

inline constexpr auto brownian_particle_endpoint_3d_kind = "brownian_particle_endpoint_3d";

struct BrownianParticleChannelConfig final {
    std::string model_id;
    core::Diffusivity diffusion_coefficient{};
    core::FirstOrderRate first_order_degradation_rate{};
    std::uint64_t sample_count{};
    std::uint64_t experiment_seed{};
    std::string random_stream_name;
};

struct BrownianParticleChannelEvaluation final {
    MolecularChannelResponse response;
    std::uint64_t sample_count{};
    std::uint64_t receiver_observation_count{};
    double receiver_fraction_standard_error{};
};

class BrownianParticleChannel final : public MolecularChannel {
  public:
    explicit BrownianParticleChannel(BrownianParticleChannelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] MolecularChannelResponse
    evaluate(const MolecularChannelRequest& request) const override;
    [[nodiscard]] BrownianParticleChannelEvaluation
    evaluate_with_diagnostics(const MolecularChannelRequest& request) const;

  private:
    BrownianParticleChannelConfig config_;
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_BROWNIAN_PARTICLE_CHANNEL_HPP

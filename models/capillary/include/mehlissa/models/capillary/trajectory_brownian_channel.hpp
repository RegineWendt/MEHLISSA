// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_HPP
#define MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_HPP

#include <mehlissa/models/capillary/molecular_channel.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto brownian_trajectory_3d_kind = "brownian_trajectory_3d";

enum class BrownianBoundaryKind : std::uint8_t { unbounded, reflecting_box };

struct BrownianBoundaryConfig final {
    BrownianBoundaryKind kind{BrownianBoundaryKind::unbounded};
    std::array<core::Length, 3> reflecting_box_half_extents{};
};

struct TrajectoryBrownianChannelConfig final {
    std::string model_id;
    core::Diffusivity diffusion_coefficient{};
    core::FirstOrderRate first_order_degradation_rate{};
    std::uint64_t sample_count{};
    std::uint64_t step_count{};
    std::uint64_t experiment_seed{};
    std::string random_stream_name;
    BrownianBoundaryConfig boundary;
    std::uint64_t retained_trajectory_count{};
    std::size_t maximum_retained_points{};
};

struct BrownianTrajectoryPoint final {
    std::uint64_t particle_index{};
    std::uint64_t step_index{};
    double elapsed_seconds{};
    std::array<core::Length, 3> position{};
    bool signal_survives{};
};

struct TrajectoryBrownianChannelEvaluation final {
    MolecularChannelResponse response;
    std::uint64_t sample_count{};
    std::uint64_t step_count{};
    std::uint64_t receiver_observation_count{};
    std::uint64_t surviving_particle_count{};
    std::uint64_t boundary_reflection_count{};
    double receiver_fraction_standard_error{};
    double mean_squared_displacement_m2{};
    double free_diffusion_expected_mean_squared_displacement_m2{};
    std::vector<BrownianTrajectoryPoint> retained_points;
    std::uint64_t dropped_trajectory_point_count{};
};

class TrajectoryBrownianChannel final : public MolecularChannel {
  public:
    explicit TrajectoryBrownianChannel(TrajectoryBrownianChannelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] BrownianBoundaryKind boundary_kind() const noexcept;
    [[nodiscard]] MolecularChannelResponse
    evaluate(const MolecularChannelRequest& request) const override;
    [[nodiscard]] TrajectoryBrownianChannelEvaluation
    evaluate_with_diagnostics(const MolecularChannelRequest& request) const;

  private:
    TrajectoryBrownianChannelConfig config_;
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_TRAJECTORY_BROWNIAN_CHANNEL_HPP

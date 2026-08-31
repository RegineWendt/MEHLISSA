// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_NETWORK_HPP
#define MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_NETWORK_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/random_stream.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr std::string_view intracellular_ode_kind = "rk4_two_stage_intracellular";
inline constexpr std::string_view intracellular_ssa_kind = "gillespie_two_stage_intracellular";

struct ReceptorActivationKnot final {
    core::SimulationClock::Duration offset{};
    double bound_fraction{};
};

struct IntracellularNetworkKinetics final {
    std::string network_id;
    core::FirstOrderRate messenger_activation_rate{};
    core::FirstOrderRate messenger_deactivation_rate{};
    core::FirstOrderRate effector_activation_rate{};
    core::FirstOrderRate effector_deactivation_rate{};
    double response_threshold_fraction{};
};

struct IntracellularNetworkRequest final {
    std::string request_id;
    core::SimulationClock::Duration observation_time{};
    double initial_active_messenger_fraction{};
    double initial_active_effector_fraction{};
    std::vector<ReceptorActivationKnot> receptor_trajectory;
};

struct IntracellularStateSample final {
    core::SimulationClock::Duration offset{};
    double active_messenger_fraction{};
    double active_effector_fraction{};
};

struct IntracellularOdeConfig final {
    IntracellularNetworkKinetics kinetics;
    core::SimulationClock::Duration integration_step{};
    std::size_t maximum_integration_steps{};
    std::size_t maximum_recorded_samples{};
};

struct IntracellularOdeResponse final {
    std::string request_id;
    std::string network_id;
    double final_active_messenger_fraction{};
    double final_active_effector_fraction{};
    double peak_active_effector_fraction{};
    bool response_threshold_reached{};
    std::optional<core::SimulationClock::Duration> first_response_time;
    std::size_t integration_steps{};
    std::size_t dropped_samples{};
    std::vector<IntracellularStateSample> samples;
};

struct IntracellularSsaConfig final {
    IntracellularNetworkKinetics kinetics;
    std::uint32_t messenger_molecule_count{};
    std::uint32_t effector_molecule_count{};
    std::size_t maximum_reaction_events{};
    std::size_t maximum_recorded_samples{};
};

struct IntracellularSsaResponse final {
    std::string request_id;
    std::string network_id;
    std::uint32_t total_messenger_molecules{};
    std::uint32_t active_messenger_molecules{};
    std::uint32_t total_effector_molecules{};
    std::uint32_t active_effector_molecules{};
    double final_active_messenger_fraction{};
    double final_active_effector_fraction{};
    double peak_active_effector_fraction{};
    bool response_threshold_reached{};
    std::optional<core::SimulationClock::Duration> first_response_time;
    std::size_t reaction_events{};
    std::uint64_t random_draws{};
    std::size_t dropped_samples{};
    std::vector<IntracellularStateSample> samples;
};

class IntracellularOdeModel final {
  public:
    explicit IntracellularOdeModel(IntracellularOdeConfig config);
    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] IntracellularOdeResponse
    evaluate(const IntracellularNetworkRequest& request) const;

  private:
    IntracellularOdeConfig config_;
};

class IntracellularSsaModel final {
  public:
    explicit IntracellularSsaModel(IntracellularSsaConfig config);
    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] IntracellularSsaResponse evaluate(const IntracellularNetworkRequest& request,
                                                    core::RandomStream& random) const;

  private:
    IntracellularSsaConfig config_;
};

void validate_intracellular_kinetics(const IntracellularNetworkKinetics& kinetics);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_NETWORK_HPP

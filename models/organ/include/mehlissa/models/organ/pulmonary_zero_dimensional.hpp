// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_HPP
#define MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/organ/pulmonary_circulation.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::organ {

struct PulmonaryFlowAdaptationParameters final {
    core::FlowRate reference_cardiac_output;
    double resistance_exponent{};
    double compliance_exponent{};
    core::Dimensionless maximum_flow_ratio;
};

struct PulmonaryZeroDimensionalParameters final {
    core::FlowRate baseline_cardiac_output;
    core::Pressure left_atrial_pressure;
    core::VascularResistance pulmonary_vascular_resistance;
    core::VascularCompliance pulmonary_arterial_compliance;
    core::SimulationClock::Duration pulmonary_transit_time{};
    core::Dimensionless right_lung_perfusion_fraction;
    std::optional<PulmonaryFlowAdaptationParameters> flow_adaptation;
};

struct PulmonaryZeroDimensionalConfig final {
    std::string component_name;
    std::string model_id;
    std::string entry_port_id;
    std::string exit_port_id;
    std::string return_target_model_id;
    std::string return_target_port_id;
    PulmonaryZeroDimensionalParameters parameters;
};

struct PulmonaryZeroDimensionalState final {
    core::Pressure mean_pulmonary_arterial_pressure;
    core::FlowRate prescribed_inflow;
    core::FlowRate pulmonary_outflow;
    core::FlowRate right_lung_flow;
    core::FlowRate left_lung_flow;
    core::VascularResistance right_lung_resistance;
    core::VascularResistance left_lung_resistance;
    core::Time pressure_time_constant;
    core::VascularResistance effective_pulmonary_vascular_resistance;
    core::VascularCompliance effective_pulmonary_arterial_compliance;
    core::Dimensionless effective_flow_ratio;
};

class PulmonaryZeroDimensionalModel final : public coupling::ModelComponent {
  public:
    explicit PulmonaryZeroDimensionalModel(PulmonaryZeroDimensionalConfig config);

    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] bool accepts_entity_at(std::string_view port_id) const noexcept override;
    [[nodiscard]] bool emits_entity_at(std::string_view port_id) const noexcept override;
    void initialize(core::SimulationContext& context) override;
    void advance(core::SimulationContext& context, core::SimulationClock::Duration delta) override;
    void finalize(core::SimulationContext& context) noexcept override;
    void accept_entity(coupling::EntityTransfer transfer) override;
    [[nodiscard]] std::vector<coupling::EntityTransfer> take_outbound_entities() override;
    void accept_conserved_transfer(coupling::ConservedTransfer transfer) override;
    [[nodiscard]] std::vector<coupling::ConservedTransfer>
    take_outbound_conserved_transfers() override;
    [[nodiscard]] std::size_t resident_conserved_transfer_count() const noexcept override;

    [[nodiscard]] PulmonaryZeroDimensionalState state() const noexcept;

  private:
    PulmonaryZeroDimensionalConfig config_;
    PulmonaryCirculation transit_;
    core::FlowRate prescribed_inflow_;
    core::Pressure mean_pulmonary_arterial_pressure_;
    std::optional<core::FlowRate> pending_inflow_;
};

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_PULMONARY_ZERO_DIMENSIONAL_HPP

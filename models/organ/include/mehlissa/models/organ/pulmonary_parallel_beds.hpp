// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_PULMONARY_PARALLEL_BEDS_HPP
#define MEHLISSA_MODELS_ORGAN_PULMONARY_PARALLEL_BEDS_HPP

#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::organ {

struct PulmonaryParallelBedState final {
    std::string id;
    core::Dimensionless perfusion_fraction;
    core::FlowRate flow;
    core::VascularResistance resistance;
    core::VascularCompliance compliance;
    core::SimulationClock::Duration transit_time{};
    core::Volume blood_volume;
};

struct PulmonaryParallelBedsState final {
    PulmonaryZeroDimensionalState aggregate;
    std::vector<PulmonaryParallelBedState> beds;
};

class PulmonaryParallelBedsModel final : public coupling::ModelComponent {
  public:
    explicit PulmonaryParallelBedsModel(PulmonaryZeroDimensionalConfig config);

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

    [[nodiscard]] PulmonaryParallelBedsState state() const;
    [[nodiscard]] std::size_t bed_index_for_entity(std::uint64_t entity_id) const noexcept;
    [[nodiscard]] std::size_t resident_entity_count() const noexcept;

  private:
    PulmonaryZeroDimensionalConfig config_;
    PulmonaryZeroDimensionalModel aggregate_;
    std::vector<std::unique_ptr<PulmonaryCirculation>> bed_transits_;
};

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_PULMONARY_PARALLEL_BEDS_HPP

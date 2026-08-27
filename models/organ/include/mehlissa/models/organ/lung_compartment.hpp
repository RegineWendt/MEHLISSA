// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_LUNG_COMPARTMENT_HPP
#define MEHLISSA_MODELS_ORGAN_LUNG_COMPARTMENT_HPP

#include <mehlissa/models/coupling/model_component.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::organ {

struct LungCompartmentConfig final {
    std::string component_name;
    std::string model_id;
    std::string entry_port_id;
    std::string exit_port_id;
    std::string return_target_model_id;
    std::string return_target_port_id;
    core::SimulationClock::Duration transit_time{};
};

class LungCompartment final : public coupling::ModelComponent {
  public:
    explicit LungCompartment(LungCompartmentConfig config);

    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] bool accepts_entity_at(std::string_view port_id) const noexcept override;
    [[nodiscard]] bool emits_entity_at(std::string_view port_id) const noexcept override;

    void initialize(core::SimulationContext& context) override;
    void advance(core::SimulationContext& context, core::SimulationClock::Duration delta) override;
    void finalize(core::SimulationContext& context) noexcept override;

    void accept_entity(coupling::EntityTransfer transfer) override;
    [[nodiscard]] std::vector<coupling::EntityTransfer> take_outbound_entities() override;

    [[nodiscard]] std::size_t resident_count() const noexcept;
    [[nodiscard]] std::size_t outbound_count() const noexcept;
    [[nodiscard]] std::uint64_t accepted_count() const noexcept;
    [[nodiscard]] std::uint64_t completed_count() const noexcept;

  private:
    enum class State : std::uint8_t { building, initialized, finalized };

    struct ResidentEntity final {
        coupling::EntityTransfer transfer;
        core::SimulationClock::Duration residence_time{};
    };

    LungCompartmentConfig config_;
    std::vector<ResidentEntity> residents_;
    std::vector<coupling::EntityTransfer> outbound_;
    std::unordered_set<std::uint64_t> held_entity_ids_;
    core::SimulationClock::Duration synchronization_time_{};
    std::uint64_t accepted_count_{};
    std::uint64_t completed_count_{};
    State state_{State::building};
};

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_LUNG_COMPARTMENT_HPP

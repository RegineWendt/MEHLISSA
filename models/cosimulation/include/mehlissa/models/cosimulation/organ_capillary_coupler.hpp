// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_ORGAN_CAPILLARY_COUPLER_HPP
#define MEHLISSA_MODELS_COSIMULATION_ORGAN_CAPILLARY_COUPLER_HPP

#include <mehlissa/models/coupling/model_component.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::cosimulation {

struct OrganCapillaryEndpoints final {
    coupling::ModelComponent& organ;
    coupling::ModelComponent& capillary;
};

struct OrganCapillaryRoute final {
    std::string organ_departure_port_id;
    std::string capillary_entry_port_id;
    std::string capillary_exit_port_id;
    std::string organ_return_port_id;
};

struct CoupledTransferCounts final {
    std::size_t entities{};
    std::size_t conserved_transfers{};

    [[nodiscard]] bool operator==(const CoupledTransferCounts&) const noexcept = default;
};

class OrganCapillaryCoupler final {
  public:
    OrganCapillaryCoupler(OrganCapillaryEndpoints endpoints, OrganCapillaryRoute route);

    [[nodiscard]] CoupledTransferCounts
    transfer_to_capillary(core::SimulationClock::Duration synchronization_time);
    [[nodiscard]] CoupledTransferCounts
    transfer_to_organ(core::SimulationClock::Duration synchronization_time);

    [[nodiscard]] std::size_t outstanding_entity_count() const noexcept;
    [[nodiscard]] std::size_t outstanding_conserved_transfer_count() const noexcept;
    [[nodiscard]] std::size_t pending_departure_count() const noexcept;
    [[nodiscard]] std::size_t pending_return_count() const noexcept;
    [[nodiscard]] std::uint64_t completed_entity_round_trip_count() const noexcept;
    [[nodiscard]] std::uint64_t completed_conserved_round_trip_count() const noexcept;

  private:
    void validate_entity_departure(const coupling::EntityTransfer& transfer,
                                   core::SimulationClock::Duration synchronization_time) const;
    void validate_entity_return(const coupling::EntityTransfer& transfer,
                                core::SimulationClock::Duration synchronization_time) const;
    void validate_conserved_departure(const coupling::ConservedTransfer& transfer,
                                      core::SimulationClock::Duration synchronization_time) const;
    void validate_conserved_return(const coupling::ConservedTransfer& transfer,
                                   core::SimulationClock::Duration synchronization_time) const;

    coupling::ModelComponent& organ_;
    coupling::ModelComponent& capillary_;
    OrganCapillaryRoute route_;
    std::vector<coupling::EntityTransfer> pending_entity_departures_;
    std::vector<coupling::ConservedTransfer> pending_conserved_departures_;
    std::vector<coupling::EntityTransfer> pending_entity_returns_;
    std::vector<coupling::ConservedTransfer> pending_conserved_returns_;
    std::unordered_set<std::uint64_t> outstanding_entity_ids_;
    std::unordered_set<std::string> outstanding_conserved_transfer_ids_;
    std::uint64_t completed_entity_round_trips_{};
    std::uint64_t completed_conserved_round_trips_{};
};

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_ORGAN_CAPILLARY_COUPLER_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_BODY_ORGAN_COUPLER_HPP
#define MEHLISSA_MODELS_COSIMULATION_BODY_ORGAN_COUPLER_HPP

#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/coupling/model_component.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::cosimulation {

struct BodyOrganRoute final {
    std::string body_departure_segment_id;
    std::string body_departure_port_id;
    std::string organ_entry_port_id;
    std::string organ_exit_port_id;
    std::string body_return_port_id;
    std::string body_return_segment_id;
};

class BodyOrganCoupler final {
  public:
    BodyOrganCoupler(body::CompartmentTransport& body, coupling::ModelComponent& organ,
                     BodyOrganRoute route);

    void send_to_organ(std::uint64_t entity_id, core::SimulationClock::Duration time);
    [[nodiscard]] std::size_t
    receive_from_organ(core::SimulationClock::Duration synchronization_time);

    [[nodiscard]] std::size_t in_flight_count() const noexcept;
    [[nodiscard]] std::size_t pending_return_count() const noexcept;
    [[nodiscard]] std::uint64_t completed_round_trip_count() const noexcept;

  private:
    void validate_return(const coupling::EntityTransfer& transfer,
                         core::SimulationClock::Duration synchronization_time) const;

    body::CompartmentTransport& body_;
    coupling::ModelComponent& organ_;
    BodyOrganRoute route_;
    std::unordered_set<std::uint64_t> in_flight_entity_ids_;
    std::vector<coupling::EntityTransfer> pending_returns_;
    std::uint64_t completed_round_trip_count_{};
};

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_BODY_ORGAN_COUPLER_HPP

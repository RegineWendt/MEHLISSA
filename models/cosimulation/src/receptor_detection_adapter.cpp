// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/receptor_detection_adapter.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>

namespace mehlissa::models::cosimulation {

iot::MolecularDetectionEvent
make_receptor_detection_event(const cell::ReceptorLigandResponse& response,
                              const ReceptorDetectionMapping& mapping) {
    const auto crossing = response.first_threshold_crossing_time;
    if (mapping.event_id.empty() || mapping.detector_device_id.empty() ||
        response.request_id.empty() || response.receptor_id.empty() || response.ligand_id.empty() ||
        response.cell_model_id.empty() || response.compartment_id.empty() ||
        !response.detection_threshold_reached || !crossing.has_value() ||
        crossing.value_or(core::SimulationClock::Duration::min()) <
            core::SimulationClock::Duration::zero() ||
        crossing.value_or(core::SimulationClock::Duration::max()) > response.observation_time ||
        !std::isfinite(response.final_bound_fraction) || response.final_bound_fraction < 0.0 ||
        response.final_bound_fraction > 1.0) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Receptor response cannot form a complete causal molecular-detection event"};
    }

    iot::MolecularDetectionEvent event{std::string{iot::molecular_detection_contract_version},
                                       mapping.event_id,
                                       response.cell_model_id,
                                       response.request_id,
                                       mapping.detector_device_id,
                                       response.ligand_id,
                                       response.compartment_id,
                                       crossing.value(),
                                       response.final_bound_fraction};
    iot::validate_molecular_detection_event(event);
    return event;
}

} // namespace mehlissa::models::cosimulation

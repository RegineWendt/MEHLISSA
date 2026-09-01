// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_RECEPTOR_DETECTION_ADAPTER_HPP
#define MEHLISSA_MODELS_COSIMULATION_RECEPTOR_DETECTION_ADAPTER_HPP

#include <mehlissa/models/cell/receptor_ligand_model.hpp>
#include <mehlissa/models/iot/molecular_detection.hpp>

#include <string>

namespace mehlissa::models::cosimulation {

struct ReceptorDetectionMapping final {
    std::string event_id;
    std::string detector_device_id;
};

[[nodiscard]] iot::MolecularDetectionEvent
make_receptor_detection_event(const cell::ReceptorLigandResponse& response,
                              const ReceptorDetectionMapping& mapping);

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_RECEPTOR_DETECTION_ADAPTER_HPP

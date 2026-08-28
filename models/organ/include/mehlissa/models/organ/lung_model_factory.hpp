// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_LUNG_MODEL_FACTORY_HPP
#define MEHLISSA_MODELS_ORGAN_LUNG_MODEL_FACTORY_HPP

#include <mehlissa/models/coupling/model_component.hpp>
#include <mehlissa/models/organ/pulmonary_circulation.hpp>
#include <mehlissa/models/organ/pulmonary_parallel_beds.hpp>
#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::models::organ {

enum class LungModelVariant : std::uint8_t {
    effective_compartment,
    regional_circulation,
    pulmonary_zero_dimensional
};

struct LungModelConfig final {
    LungModelVariant variant{LungModelVariant::effective_compartment};
    std::string component_name;
    std::string model_id;
    std::string entry_port_id;
    std::string exit_port_id;
    std::string return_target_model_id;
    std::string return_target_port_id;
    core::SimulationClock::Duration compartment_transit_time{};
    std::vector<PulmonaryTransitRegion> regions;
    std::optional<PulmonaryZeroDimensionalParameters> zero_dimensional_parameters;
};

[[nodiscard]] std::unique_ptr<coupling::ModelComponent> make_lung_model(LungModelConfig config);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_LUNG_MODEL_FACTORY_HPP

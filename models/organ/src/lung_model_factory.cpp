// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_compartment.hpp>

#include <memory>
#include <utility>

namespace mehlissa::models::organ {

namespace {

[[noreturn]] void throw_invalid_config(const char* message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

void validate_exclusive_parameters(const LungModelConfig& config) {
    switch (config.variant) {
    case LungModelVariant::effective_compartment:
        if (!config.regions.empty()) {
            throw_invalid_config(
                "An effective lung compartment cannot also define pulmonary regions");
        }
        return;
    case LungModelVariant::regional_circulation:
        if (config.compartment_transit_time != core::SimulationClock::Duration::zero()) {
            throw_invalid_config(
                "A regional pulmonary circulation cannot also define a compartment transit time");
        }
        return;
    }
    throw_invalid_config("Unknown lung model variant");
}

} // namespace

std::unique_ptr<coupling::ModelComponent> make_lung_model(LungModelConfig config) {
    validate_exclusive_parameters(config);

    switch (config.variant) {
    case LungModelVariant::effective_compartment:
        return std::make_unique<LungCompartment>(LungCompartmentConfig{
            std::move(config.component_name),
            std::move(config.model_id),
            std::move(config.entry_port_id),
            std::move(config.exit_port_id),
            std::move(config.return_target_model_id),
            std::move(config.return_target_port_id),
            config.compartment_transit_time,
        });
    case LungModelVariant::regional_circulation:
        return std::make_unique<PulmonaryCirculation>(PulmonaryCirculationConfig{
            std::move(config.component_name),
            std::move(config.model_id),
            std::move(config.entry_port_id),
            std::move(config.exit_port_id),
            std::move(config.return_target_model_id),
            std::move(config.return_target_port_id),
            std::move(config.regions),
        });
    }
    throw_invalid_config("Unknown lung model variant");
}

} // namespace mehlissa::models::organ

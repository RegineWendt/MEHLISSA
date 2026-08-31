// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/capillary_cell_signal_coupler.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <string>
#include <utility>

namespace mehlissa::models::cosimulation {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

void validate_capillary_cell_signal_coupler_config(const CapillaryCellSignalCouplerConfig& config) {
    if (config.profile_id.empty() || config.source_model_id.empty() ||
        config.source_compartment_id.empty() || config.signal_id.empty() ||
        !std::isfinite(core::in_cubic_meters(config.represented_volume)) ||
        core::in_cubic_meters(config.represented_volume) <= 0.0 ||
        config.target_cell_model_id.empty() || config.ligand_id.empty() ||
        config.target_compartment_id.empty() ||
        config.exposure_duration <= core::SimulationClock::Duration::zero() ||
        !valid_fraction(config.initial_bound_fraction)) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-cell signal coupler configuration is incomplete or nonphysical");
    }
}

CapillaryCellSignalCoupler::CapillaryCellSignalCoupler(CapillaryCellSignalCouplerConfig config)
    : config_{std::move(config)} {
    validate_capillary_cell_signal_coupler_config(config_);
}

CapillaryCellSignalEvaluation CapillaryCellSignalCoupler::evaluate(
    const coupling::ExtracellularSignalSource& source, const cell::ReceptorLigandModel& cell_model,
    std::string sample_id, const core::SimulationClock::Duration observed_at) {
    if (sample_id.empty() || completed_sample_ids_.contains(sample_id)) {
        invalid(core::ErrorCode::invariant_violated,
                "Capillary-cell signal sample ID is empty or was already evaluated");
    }
    if (source.signal_source_model_id() != config_.source_model_id ||
        cell_model.model_id() != config_.target_cell_model_id) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-cell signal source or target model is incompatible");
    }

    const coupling::ExtracellularSignalObservationRequest observation{
        std::string{coupling::extracellular_signal_contract_version},
        sample_id,
        config_.signal_id,
        config_.source_compartment_id,
        config_.represented_volume,
        observed_at,
        config_.exposure_duration,
    };
    const auto sample = source.observe_extracellular_signal(observation);
    coupling::validate_extracellular_signal_sample(sample);
    if (sample.sample_id != sample_id || sample.signal_id != config_.signal_id ||
        sample.source_model_id != config_.source_model_id ||
        sample.source_compartment_id != config_.source_compartment_id ||
        sample.represented_volume != config_.represented_volume ||
        sample.observed_at != observed_at || sample.valid_for != config_.exposure_duration) {
        invalid(core::ErrorCode::invariant_violated,
                "Extracellular signal source changed the requested hand-off semantics");
    }

    const cell::ReceptorLigandRequest cell_request{
        sample.sample_id,
        config_.ligand_id,
        config_.target_compartment_id,
        coupling::extracellular_signal_concentration(sample),
        sample.valid_for,
        config_.initial_bound_fraction,
    };
    auto cell_response = cell_model.evaluate(cell_request);
    if (cell_response.request_id != sample.sample_id ||
        cell_response.cell_model_id != config_.target_cell_model_id ||
        cell_response.ligand_id != config_.ligand_id ||
        cell_response.compartment_id != config_.target_compartment_id ||
        cell_response.observation_time != sample.valid_for) {
        invalid(core::ErrorCode::invariant_violated,
                "Cell model changed the mapped signal hand-off identity or time window");
    }

    completed_sample_ids_.insert(sample.sample_id);
    return {config_.profile_id, sample, std::move(cell_response)};
}

std::size_t CapillaryCellSignalCoupler::completed_sample_count() const noexcept {
    return completed_sample_ids_.size();
}

} // namespace mehlissa::models::cosimulation

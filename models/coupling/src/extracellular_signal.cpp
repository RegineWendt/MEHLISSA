// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/coupling/extracellular_signal.hpp>

#include <mehlissa/core/error.hpp>

#include <cmath>
#include <string>

namespace mehlissa::models::coupling {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

void validate_time_window(const core::SimulationClock::Duration observed_at,
                          const core::SimulationClock::Duration valid_for) {
    if (observed_at < core::SimulationClock::Duration::zero() ||
        valid_for <= core::SimulationClock::Duration::zero() ||
        observed_at > core::SimulationClock::Duration::max() - valid_for) {
        invalid("Extracellular signal requires a non-negative, non-overflowing time window");
    }
}

} // namespace

void validate_extracellular_signal_request(const ExtracellularSignalObservationRequest& request) {
    if (request.contract_version != extracellular_signal_contract_version ||
        request.sample_id.empty() || request.signal_id.empty() ||
        request.source_compartment_id.empty() ||
        !std::isfinite(core::in_cubic_meters(request.represented_volume)) ||
        core::in_cubic_meters(request.represented_volume) <= 0.0) {
        invalid("Extracellular signal request is incomplete or nonphysical");
    }
    validate_time_window(request.observed_at, request.valid_for);
}

void validate_extracellular_signal_sample(const ExtracellularSignalSample& sample) {
    if (sample.contract_version != extracellular_signal_contract_version ||
        sample.sample_id.empty() || sample.signal_id.empty() || sample.source_model_id.empty() ||
        sample.source_compartment_id.empty() ||
        sample.sampling_semantics != non_consuming_uniform_inventory_snapshot ||
        !std::isfinite(core::in_moles(sample.represented_amount)) ||
        core::in_moles(sample.represented_amount) < 0.0 ||
        !std::isfinite(core::in_cubic_meters(sample.represented_volume)) ||
        core::in_cubic_meters(sample.represented_volume) <= 0.0) {
        invalid("Extracellular signal sample is incomplete or nonphysical");
    }
    validate_time_window(sample.observed_at, sample.valid_for);
}

core::Concentration extracellular_signal_concentration(const ExtracellularSignalSample& sample) {
    validate_extracellular_signal_sample(sample);
    return sample.represented_amount / sample.represented_volume;
}

core::SimulationClock::Duration
extracellular_signal_valid_until(const ExtracellularSignalSample& sample) {
    validate_extracellular_signal_sample(sample);
    return sample.observed_at + sample.valid_for;
}

} // namespace mehlissa::models::coupling

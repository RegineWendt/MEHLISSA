// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/analytical_receptor_ligand_model.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

namespace mehlissa::models::cell {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] bool finite_positive(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

void validate_config(const ReceptorLigandModelConfig& config) {
    if (config.model_id.empty() || config.receptor_id.empty() || config.ligand_id.empty() ||
        config.compartment_id.empty() ||
        !finite_positive(core::in_cubic_meters(config.cell_volume)) ||
        !finite_positive(core::in_moles_per_cubic_meter(config.total_receptor_concentration)) ||
        !finite_positive(core::in_cubic_meters_per_mole_second(config.association_rate)) ||
        !std::isfinite(core::in_per_second(config.dissociation_rate)) ||
        core::in_per_second(config.dissociation_rate) < 0.0 ||
        !valid_fraction(config.detection_threshold_fraction) ||
        config.detection_threshold_fraction <= 0.0) {
        invalid("Receptor-ligand model configuration is incomplete or nonphysical");
    }
}

[[nodiscard]] std::optional<core::SimulationClock::Duration>
threshold_crossing_time(const double initial_fraction, const double equilibrium_fraction,
                        const double combined_rate, const double threshold,
                        const core::SimulationClock::Duration observation_time) {
    if (initial_fraction >= threshold) {
        return core::SimulationClock::Duration::zero();
    }
    if (combined_rate <= 0.0 || equilibrium_fraction <= threshold) {
        return std::nullopt;
    }
    const auto ratio =
        (threshold - equilibrium_fraction) / (initial_fraction - equilibrium_fraction);
    if (!std::isfinite(ratio) || ratio <= 0.0 || ratio >= 1.0) {
        return std::nullopt;
    }
    const auto crossing_seconds = -std::log(ratio) / combined_rate;
    const auto crossing = std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{crossing_seconds});
    if (crossing > observation_time) {
        return std::nullopt;
    }
    return crossing;
}

} // namespace

AnalyticalReceptorLigandModel::AnalyticalReceptorLigandModel(ReceptorLigandModelConfig config)
    : config_{std::move(config)} {
    validate_config(config_);
}

std::string_view AnalyticalReceptorLigandModel::kind() const noexcept {
    return analytical_receptor_ligand_kind;
}

std::string_view AnalyticalReceptorLigandModel::model_id() const noexcept {
    return config_.model_id;
}

ReceptorLigandResponse
AnalyticalReceptorLigandModel::evaluate(const ReceptorLigandRequest& request) const {
    const auto ligand_concentration = core::in_moles_per_cubic_meter(request.ligand_concentration);
    if (request.request_id.empty() || request.ligand_id != config_.ligand_id ||
        request.compartment_id != config_.compartment_id || !std::isfinite(ligand_concentration) ||
        ligand_concentration < 0.0 ||
        request.observation_time < core::SimulationClock::Duration::zero() ||
        !valid_fraction(request.initial_bound_fraction)) {
        invalid("Receptor-ligand request is incompatible or nonphysical");
    }

    const auto association_rate =
        core::in_cubic_meters_per_mole_second(config_.association_rate) * ligand_concentration;
    const auto dissociation_rate = core::in_per_second(config_.dissociation_rate);
    const auto combined_rate = association_rate + dissociation_rate;
    const auto equilibrium_fraction =
        combined_rate > 0.0 ? association_rate / combined_rate : request.initial_bound_fraction;
    const auto elapsed_seconds = std::chrono::duration<double>{request.observation_time}.count();
    const auto final_fraction =
        std::clamp(equilibrium_fraction + (request.initial_bound_fraction - equilibrium_fraction) *
                                              std::exp(-combined_rate * elapsed_seconds),
                   0.0, 1.0);
    const auto total_amount = config_.total_receptor_concentration * config_.cell_volume;
    const auto bound_amount = total_amount * final_fraction;
    const auto free_amount = total_amount - bound_amount;
    const auto crossing =
        threshold_crossing_time(request.initial_bound_fraction, equilibrium_fraction, combined_rate,
                                config_.detection_threshold_fraction, request.observation_time);

    return {
        request.request_id,
        config_.receptor_id,
        request.ligand_id,
        config_.model_id,
        request.compartment_id,
        request.observation_time,
        total_amount,
        free_amount,
        bound_amount,
        equilibrium_fraction,
        final_fraction,
        crossing.has_value(),
        crossing,
    };
}

core::Amount accounted_receptor_amount(const ReceptorLigandResponse& response) noexcept {
    return response.free_receptor_amount + response.bound_receptor_amount;
}

double receptor_balance_error_moles(const ReceptorLigandResponse& response) noexcept {
    return std::abs(core::in_moles(response.total_receptor_amount) -
                    core::in_moles(accounted_receptor_amount(response)));
}

} // namespace mehlissa::models::cell

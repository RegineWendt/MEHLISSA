// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/dynamic_capillary_tissue_cell_model.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

namespace mehlissa::models::cosimulation {
namespace {

constexpr double avogadro_per_mole = 6.02214076e23;
constexpr double state_floor_moles = -1.0e-27;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool positive_finite(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

[[nodiscard]] bool nonnegative_finite(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0;
}

[[nodiscard]] bool fraction(const double value) noexcept {
    return nonnegative_finite(value) && value <= 1.0;
}

[[nodiscard]] double
sum_initial_owners(const DynamicCapillaryTissueCellInitialState& state) noexcept {
    return core::in_moles(state.blood_free) + core::in_moles(state.endothelium_free) +
           core::in_moles(state.interstitium_free) + core::in_moles(state.receptor_bound) +
           core::in_moles(state.internalized) + core::in_moles(state.cleared_or_degraded) +
           core::in_moles(state.cumulative_outlet);
}

} // namespace

core::Amount accounted_ligand_amount(const DynamicLigandLedger& ledger) noexcept {
    return ledger.blood_free + ledger.endothelium_free + ledger.interstitium_free +
           ledger.receptor_bound + ledger.internalized + ledger.cleared_or_degraded +
           ledger.cumulative_outlet;
}

double dynamic_balance_error_moles(const DynamicLigandLedger& ledger) noexcept {
    return std::abs(core::in_moles(ledger.initial_amount + ledger.cumulative_inlet) -
                    core::in_moles(accounted_ligand_amount(ledger)));
}

bool is_dynamically_balanced(const DynamicLigandLedger& ledger,
                             const double relative_tolerance) noexcept {
    if (!nonnegative_finite(relative_tolerance)) {
        return false;
    }
    const auto scale =
        std::max(std::numeric_limits<double>::min(),
                 std::abs(core::in_moles(ledger.initial_amount + ledger.cumulative_inlet)));
    return dynamic_balance_error_moles(ledger) <= relative_tolerance * scale;
}

void validate_dynamic_capillary_tissue_cell_parameters(
    const DynamicCapillaryTissueCellParameters& parameters) {
    if (parameters.contract_version != dynamic_capillary_tissue_cell_contract_version ||
        parameters.model_id.empty() || parameters.ligand_id != dccq1_vegfa165a_ligand_id ||
        parameters.receptor_id != dccq1_vegfr2_receptor_id ||
        parameters.cell_context != "primary-HUVEC" ||
        !positive_finite(core::in_cubic_meters(parameters.blood_volume)) ||
        !positive_finite(core::in_cubic_meters(parameters.endothelium_volume)) ||
        !positive_finite(core::in_cubic_meters(parameters.interstitium_volume)) ||
        !nonnegative_finite(core::in_per_second(parameters.blood_to_endothelium)) ||
        !nonnegative_finite(core::in_per_second(parameters.endothelium_to_blood)) ||
        !nonnegative_finite(core::in_per_second(parameters.endothelium_to_interstitium)) ||
        !nonnegative_finite(core::in_per_second(parameters.interstitium_to_endothelium)) ||
        !nonnegative_finite(core::in_per_second(parameters.blood_outlet)) ||
        !nonnegative_finite(core::in_per_second(parameters.interstitial_clearance)) ||
        !nonnegative_finite(core::in_cubic_meters_per_mole_second(parameters.association)) ||
        !nonnegative_finite(core::in_per_second(parameters.dissociation)) ||
        !nonnegative_finite(core::in_per_second(parameters.internalization)) ||
        !nonnegative_finite(core::in_per_second(parameters.degradation)) ||
        !positive_finite(core::in_moles(parameters.receptor_capacity)) ||
        !nonnegative_finite(in_moles_per_second(parameters.inlet_rate)) ||
        !positive_finite(core::in_seconds(parameters.internal_step)) ||
        !positive_finite(core::in_seconds(parameters.synchronization_interval)) ||
        core::in_seconds(parameters.internal_step) >
            core::in_seconds(parameters.synchronization_interval) ||
        !fraction(parameters.nrp1_site_fraction) ||
        !positive_finite(parameters.nrp1_association_multiplier) ||
        !fraction(parameters.feedback_occupancy_threshold) || !fraction(parameters.feedback_gain) ||
        !fraction(parameters.minimum_feedback_multiplier) ||
        parameters.minimum_feedback_multiplier > 1.0) {
        invalid(core::ErrorCode::data_invalid,
                "Dynamic capillary-tissue-cell parameters are incomplete, non-SI, or nonphysical");
    }
    if (parameters.nrp1_mode == Nrp1StructuralMode::excluded) {
        if (!parameters.coreceptor_id.empty() || parameters.nrp1_site_fraction != 0.0 ||
            parameters.nrp1_association_multiplier != 1.0) {
            invalid(core::ErrorCode::data_invalid,
                    "Excluded NRP1 must have no identifier, site fraction, or kinetic effect");
        }
    } else if (parameters.coreceptor_id != dccq1_nrp1_coreceptor_id) {
        invalid(core::ErrorCode::data_invalid,
                "Included NRP1 must use the frozen human NRP1 identifier");
    }
    if (parameters.nrp1_mode == Nrp1StructuralMode::tracked_neutral &&
        parameters.nrp1_association_multiplier != 1.0) {
        invalid(core::ErrorCode::data_invalid,
                "Tracked-neutral NRP1 cannot introduce an unsupported kinetic multiplier");
    }
    if (parameters.nrp1_mode == Nrp1StructuralMode::facilitated_binding_assumption &&
        parameters.nrp1_association_multiplier <= 1.0) {
        invalid(core::ErrorCode::data_invalid,
                "Exploratory NRP1 facilitation must increase the association multiplier");
    }
}

void validate_dynamic_capillary_tissue_cell_initial_state(
    const DynamicCapillaryTissueCellInitialState& initial_state) {
    const std::array owners{
        core::in_moles(initial_state.blood_free),
        core::in_moles(initial_state.endothelium_free),
        core::in_moles(initial_state.interstitium_free),
        core::in_moles(initial_state.receptor_bound),
        core::in_moles(initial_state.internalized),
        core::in_moles(initial_state.cleared_or_degraded),
        core::in_moles(initial_state.cumulative_outlet),
    };
    if (!positive_finite(core::in_moles(initial_state.declared_initial_amount)) ||
        !std::all_of(owners.begin(), owners.end(), nonnegative_finite)) {
        invalid(core::ErrorCode::data_invalid,
                "Dynamic coupling initial amounts must be finite and nonnegative");
    }
    const auto declared = core::in_moles(initial_state.declared_initial_amount);
    if (std::abs(sum_initial_owners(initial_state) - declared) >
        1.0e-12 * std::max(declared, std::numeric_limits<double>::min())) {
        invalid(core::ErrorCode::invariant_violated,
                "Declared initial amount does not equal the seven exclusive owners");
    }
}

DynamicCapillaryTissueCellModel::DynamicCapillaryTissueCellModel(
    DynamicCapillaryTissueCellParameters parameters,
    const DynamicCapillaryTissueCellInitialState initial_state)
    : parameters_{std::move(parameters)}, state_{core::in_moles(initial_state.blood_free),
                                                 core::in_moles(initial_state.endothelium_free),
                                                 core::in_moles(initial_state.interstitium_free),
                                                 core::in_moles(initial_state.receptor_bound),
                                                 core::in_moles(initial_state.internalized),
                                                 core::in_moles(initial_state.cleared_or_degraded),
                                                 core::in_moles(initial_state.cumulative_outlet),
                                                 0.0},
      initial_amount_moles_{core::in_moles(initial_state.declared_initial_amount)} {
    validate_dynamic_capillary_tissue_cell_parameters(parameters_);
    validate_dynamic_capillary_tissue_cell_initial_state(initial_state);
    if (state_[3] > core::in_moles(parameters_.receptor_capacity)) {
        invalid(core::ErrorCode::data_invalid,
                "Initial receptor-bound ligand exceeds the receptor capacity");
    }
}

const DynamicCapillaryTissueCellParameters&
DynamicCapillaryTissueCellModel::parameters() const noexcept {
    return parameters_;
}

DynamicLigandLedger DynamicCapillaryTissueCellModel::ledger() const noexcept {
    return {
        core::moles(initial_amount_moles_),
        core::moles(state_[7]),
        core::moles(state_[0]),
        core::moles(state_[1]),
        core::moles(state_[2]),
        core::moles(state_[3]),
        core::moles(state_[4]),
        core::moles(state_[5]),
        core::moles(state_[6]),
    };
}

DynamicCapillaryTissueCellSnapshot DynamicCapillaryTissueCellModel::snapshot() const noexcept {
    const auto capacity = core::in_moles(parameters_.receptor_capacity);
    return {core::seconds(time_seconds_), ledger(), state_[3] / capacity,
            applied_feedback_multiplier_, next_feedback_multiplier_};
}

DynamicCapillaryTissueCellModel::State
DynamicCapillaryTissueCellModel::derivative(const State& state,
                                            const double feedback_multiplier) const noexcept {
    const auto blood_to_endothelium =
        core::in_per_second(parameters_.blood_to_endothelium) * state[0];
    const auto endothelium_to_blood =
        core::in_per_second(parameters_.endothelium_to_blood) * state[1];
    const auto endothelium_to_interstitium =
        core::in_per_second(parameters_.endothelium_to_interstitium) * feedback_multiplier *
        state[1];
    const auto interstitium_to_endothelium =
        core::in_per_second(parameters_.interstitium_to_endothelium) * state[2];
    const auto outlet = core::in_per_second(parameters_.blood_outlet) * state[0];
    const auto clearance = core::in_per_second(parameters_.interstitial_clearance) * state[2];
    const auto free_receptor =
        std::max(0.0, core::in_moles(parameters_.receptor_capacity) - state[3]);
    const auto ligand_concentration =
        state[2] / core::in_cubic_meters(parameters_.interstitium_volume);
    const auto nrp1_multiplier =
        1.0 + parameters_.nrp1_site_fraction * (parameters_.nrp1_association_multiplier - 1.0);
    const auto association = core::in_cubic_meters_per_mole_second(parameters_.association) *
                             ligand_concentration * free_receptor * nrp1_multiplier;
    const auto dissociation = core::in_per_second(parameters_.dissociation) * state[3];
    const auto internalization = core::in_per_second(parameters_.internalization) * state[3];
    const auto degradation = core::in_per_second(parameters_.degradation) * state[4];
    const auto inlet = in_moles_per_second(parameters_.inlet_rate);

    return {
        inlet - blood_to_endothelium + endothelium_to_blood - outlet,
        blood_to_endothelium - endothelium_to_blood - endothelium_to_interstitium +
            interstitium_to_endothelium,
        endothelium_to_interstitium - interstitium_to_endothelium - association + dissociation -
            clearance,
        association - dissociation - internalization,
        internalization - degradation,
        clearance + degradation,
        outlet,
        inlet,
    };
}

void DynamicCapillaryTissueCellModel::rk4_step(const core::Time step,
                                               const double feedback_multiplier) {
    const auto step_seconds = core::in_seconds(step);
    const auto k1 = derivative(state_, feedback_multiplier);
    State intermediate{};
    for (std::size_t i = 0; i < state_.size(); ++i) {
        intermediate[i] = state_[i] + 0.5 * step_seconds * k1[i];
    }
    const auto k2 = derivative(intermediate, feedback_multiplier);
    for (std::size_t i = 0; i < state_.size(); ++i) {
        intermediate[i] = state_[i] + 0.5 * step_seconds * k2[i];
    }
    const auto k3 = derivative(intermediate, feedback_multiplier);
    for (std::size_t i = 0; i < state_.size(); ++i) {
        intermediate[i] = state_[i] + step_seconds * k3[i];
    }
    const auto k4 = derivative(intermediate, feedback_multiplier);
    for (std::size_t i = 0; i < state_.size(); ++i) {
        state_[i] += step_seconds * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) / 6.0;
        if (state_[i] < state_floor_moles) {
            invalid(core::ErrorCode::invariant_violated,
                    "Dynamic coupling integration produced a negative ligand owner");
        }
        if (state_[i] < 0.0) {
            state_[i] = 0.0;
        }
    }
    const auto current_ledger = ledger();
    if (!is_dynamically_balanced(current_ledger, 1.0e-9)) {
        invalid(core::ErrorCode::invariant_violated,
                "Dynamic coupling integration violated the complete ligand ledger");
    }
}

double DynamicCapillaryTissueCellModel::scheduled_feedback_multiplier() const noexcept {
    if (parameters_.feedback_gain == 0.0) {
        return 1.0;
    }
    const auto occupancy = state_[3] / core::in_moles(parameters_.receptor_capacity);
    if (occupancy < parameters_.feedback_occupancy_threshold) {
        return 1.0;
    }
    return std::max(parameters_.minimum_feedback_multiplier,
                    1.0 - parameters_.feedback_gain * occupancy);
}

DynamicCapillaryTissueCellSnapshot
DynamicCapillaryTissueCellModel::advance_one_synchronization_interval() {
    const auto interval = core::in_seconds(parameters_.synchronization_interval);
    const auto requested_step = core::in_seconds(parameters_.internal_step);
    const auto step_count = static_cast<std::size_t>(std::ceil(interval / requested_step));
    const auto step = interval / static_cast<double>(step_count);
    const auto multiplier_used = next_feedback_multiplier_;
    applied_feedback_multiplier_ = multiplier_used;
    for (std::size_t i = 0; i < step_count; ++i) {
        rk4_step(core::seconds(step), multiplier_used);
    }
    time_seconds_ += interval;
    next_feedback_multiplier_ = scheduled_feedback_multiplier();
    auto result = snapshot();
    result.applied_feedback_multiplier = multiplier_used;
    result.scheduled_feedback_multiplier = next_feedback_multiplier_;
    return result;
}

std::vector<DynamicCapillaryTissueCellSnapshot>
DynamicCapillaryTissueCellModel::run(const core::Time duration) {
    const auto duration_seconds = core::in_seconds(duration);
    const auto interval = core::in_seconds(parameters_.synchronization_interval);
    if (!positive_finite(duration_seconds)) {
        invalid(core::ErrorCode::data_invalid,
                "Dynamic coupling run duration must be positive and finite");
    }
    const auto interval_count = std::round(duration_seconds / interval);
    if (std::abs(duration_seconds - interval_count * interval) > 1.0e-9) {
        invalid(core::ErrorCode::data_invalid,
                "Dynamic coupling duration must be an exact synchronization multiple");
    }
    std::vector<DynamicCapillaryTissueCellSnapshot> result;
    result.reserve(static_cast<std::size_t>(interval_count) + 1);
    result.push_back(snapshot());
    for (std::size_t i = 0; i < static_cast<std::size_t>(interval_count); ++i) {
        result.push_back(advance_one_synchronization_interval());
    }
    return result;
}

DynamicCapillaryTissueCellParameters dccq1_reference_parameters() {
    return {
        std::string{dynamic_capillary_tissue_cell_contract_version},
        "cosimulation.dccq1.vegfa165a-vegfr2-huvec.v1",
        std::string{dccq1_vegfa165a_ligand_id},
        std::string{dccq1_vegfr2_receptor_id},
        std::string{dccq1_nrp1_coreceptor_id},
        "primary-HUVEC",
        core::cubic_meters(1.0e-11),
        core::cubic_meters(1.0e-12),
        core::cubic_meters(1.0e-12),
        core::per_second(1.0e-3),
        core::per_second(2.0e-4),
        core::per_second(5.0e-4),
        core::per_second(1.0e-4),
        core::per_second(2.0e-5),
        core::per_second(1.0e-5),
        core::cubic_meters_per_mole_second(1.0e4),
        core::per_second(1.0e-3),
        core::per_second(6.9e-4),
        core::per_second(2.3e-4),
        core::moles(4900.0 / avogadro_per_mole),
        Nrp1StructuralMode::tracked_neutral,
        1.0,
        1.0,
        moles_per_second(0.0),
        core::seconds(2.0),
        core::seconds(60.0),
        0.10,
        0.25,
        0.75,
    };
}

DynamicCapillaryTissueCellInitialState dccq1_reference_initial_state() {
    const auto amount = core::moles(6'843'182.0 / avogadro_per_mole);
    return {amount,           amount,           core::moles(0.0), core::moles(0.0),
            core::moles(0.0), core::moles(0.0), core::moles(0.0), core::moles(0.0)};
}

} // namespace mehlissa::models::cosimulation

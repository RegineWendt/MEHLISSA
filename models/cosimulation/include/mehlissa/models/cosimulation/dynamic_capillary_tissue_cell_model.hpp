// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_DYNAMIC_CAPILLARY_TISSUE_CELL_MODEL_HPP
#define MEHLISSA_MODELS_COSIMULATION_DYNAMIC_CAPILLARY_TISSUE_CELL_MODEL_HPP

#include <mehlissa/core/quantity.hpp>

#include <array>
#include <string>
#include <vector>

namespace mehlissa::models::cosimulation {

inline constexpr auto dynamic_capillary_tissue_cell_contract_version = "1.0.0";
inline constexpr auto dccq1_vegfa165a_ligand_id = "mehlissa.bio.human-vegfa165a-homodimer.v1";
inline constexpr auto dccq1_vegfr2_receptor_id = "UniProtKB:KDR_HUMAN";
inline constexpr auto dccq1_nrp1_coreceptor_id = "UniProtKB:NRP1_HUMAN";

using AmountRate = core::Quantity<core::Dimension<0, -1, 1>>;

[[nodiscard]] constexpr AmountRate moles_per_second(const double value) noexcept {
    return AmountRate::from_si(value);
}

[[nodiscard]] constexpr double in_moles_per_second(const AmountRate value) noexcept {
    return value.si_value();
}

enum class Nrp1StructuralMode {
    excluded,
    tracked_neutral,
    facilitated_binding_assumption,
};

struct DynamicLigandLedger final {
    core::Amount initial_amount{};
    core::Amount cumulative_inlet{};
    core::Amount blood_free{};
    core::Amount endothelium_free{};
    core::Amount interstitium_free{};
    core::Amount receptor_bound{};
    core::Amount internalized{};
    core::Amount cleared_or_degraded{};
    core::Amount cumulative_outlet{};
};

[[nodiscard]] core::Amount accounted_ligand_amount(const DynamicLigandLedger& ledger) noexcept;
[[nodiscard]] double dynamic_balance_error_moles(const DynamicLigandLedger& ledger) noexcept;
[[nodiscard]] bool is_dynamically_balanced(const DynamicLigandLedger& ledger,
                                           double relative_tolerance = 1.0e-10) noexcept;

struct DynamicCapillaryTissueCellParameters final {
    std::string contract_version;
    std::string model_id;
    std::string ligand_id;
    std::string receptor_id;
    std::string coreceptor_id;
    std::string cell_context;
    core::Volume blood_volume{};
    core::Volume endothelium_volume{};
    core::Volume interstitium_volume{};
    core::FirstOrderRate blood_to_endothelium{};
    core::FirstOrderRate endothelium_to_blood{};
    core::FirstOrderRate endothelium_to_interstitium{};
    core::FirstOrderRate interstitium_to_endothelium{};
    core::FirstOrderRate blood_outlet{};
    core::FirstOrderRate interstitial_clearance{};
    core::SecondOrderAssociationRate association{};
    core::FirstOrderRate dissociation{};
    core::FirstOrderRate internalization{};
    core::FirstOrderRate degradation{};
    core::Amount receptor_capacity{};
    Nrp1StructuralMode nrp1_mode{Nrp1StructuralMode::tracked_neutral};
    double nrp1_site_fraction{};
    double nrp1_association_multiplier{1.0};
    AmountRate inlet_rate{};
    core::Time internal_step{};
    core::Time synchronization_interval{};
    double feedback_occupancy_threshold{};
    double feedback_gain{};
    double minimum_feedback_multiplier{1.0};
};

struct DynamicCapillaryTissueCellInitialState final {
    core::Amount declared_initial_amount{};
    core::Amount blood_free{};
    core::Amount endothelium_free{};
    core::Amount interstitium_free{};
    core::Amount receptor_bound{};
    core::Amount internalized{};
    core::Amount cleared_or_degraded{};
    core::Amount cumulative_outlet{};
};

struct DynamicCapillaryTissueCellSnapshot final {
    core::Time time{};
    DynamicLigandLedger ledger;
    double receptor_occupancy_fraction{};
    double applied_feedback_multiplier{1.0};
    double scheduled_feedback_multiplier{1.0};
};

void validate_dynamic_capillary_tissue_cell_parameters(
    const DynamicCapillaryTissueCellParameters& parameters);
void validate_dynamic_capillary_tissue_cell_initial_state(
    const DynamicCapillaryTissueCellInitialState& initial_state);

class DynamicCapillaryTissueCellModel final {
  public:
    DynamicCapillaryTissueCellModel(DynamicCapillaryTissueCellParameters parameters,
                                    DynamicCapillaryTissueCellInitialState initial_state);

    [[nodiscard]] const DynamicCapillaryTissueCellParameters& parameters() const noexcept;
    [[nodiscard]] DynamicCapillaryTissueCellSnapshot snapshot() const noexcept;
    [[nodiscard]] DynamicCapillaryTissueCellSnapshot advance_one_synchronization_interval();
    [[nodiscard]] std::vector<DynamicCapillaryTissueCellSnapshot> run(core::Time duration);

  private:
    using State = std::array<double, 8>;

    [[nodiscard]] State derivative(const State& state, double feedback_multiplier) const noexcept;
    void rk4_step(double step_seconds, double feedback_multiplier);
    [[nodiscard]] double scheduled_feedback_multiplier() const noexcept;
    [[nodiscard]] DynamicLigandLedger ledger() const noexcept;

    DynamicCapillaryTissueCellParameters parameters_;
    State state_{};
    double initial_amount_moles_{};
    double time_seconds_{};
    double applied_feedback_multiplier_{1.0};
    double next_feedback_multiplier_{1.0};
};

[[nodiscard]] DynamicCapillaryTissueCellParameters dccq1_reference_parameters();
[[nodiscard]] DynamicCapillaryTissueCellInitialState dccq1_reference_initial_state();

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_DYNAMIC_CAPILLARY_TISSUE_CELL_MODEL_HPP

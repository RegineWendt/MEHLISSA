// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_zero_dimensional.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace mehlissa::models::organ {
namespace {

[[nodiscard]] bool positive_finite(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

void validate_parameters(const PulmonaryZeroDimensionalParameters& parameters) {
    const auto right_fraction = parameters.right_lung_perfusion_fraction.si_value();
    if (!positive_finite(core::in_cubic_meters_per_second(parameters.baseline_cardiac_output)) ||
        !std::isfinite(core::in_pascals(parameters.left_atrial_pressure)) ||
        core::in_pascals(parameters.left_atrial_pressure) < 0.0 ||
        !positive_finite(
            core::in_pascal_seconds_per_cubic_meter(parameters.pulmonary_vascular_resistance)) ||
        !positive_finite(
            core::in_cubic_meters_per_pascal(parameters.pulmonary_arterial_compliance)) ||
        parameters.pulmonary_transit_time <= core::SimulationClock::Duration::zero() ||
        !std::isfinite(right_fraction) || right_fraction <= 0.0 || right_fraction >= 1.0) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "Pulmonary 0D parameters require finite positive flow, resistance, compliance, "
            "transit time, non-negative left-atrial pressure, and a right-lung fraction in (0,1)"};
    }
    if (parameters.flow_adaptation.has_value()) {
        const auto& adaptation = *parameters.flow_adaptation;
        if (!positive_finite(
                core::in_cubic_meters_per_second(adaptation.reference_cardiac_output)) ||
            !std::isfinite(adaptation.resistance_exponent) ||
            adaptation.resistance_exponent > 0.0 ||
            !std::isfinite(adaptation.compliance_exponent) ||
            adaptation.compliance_exponent > 0.0 ||
            !std::isfinite(adaptation.maximum_flow_ratio.si_value()) ||
            adaptation.maximum_flow_ratio.si_value() <= 1.0) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Pulmonary flow adaptation requires finite non-positive exponents and a "
                "maximum flow ratio greater than one"};
        }
    }
    if (parameters.pressure_distensibility.has_value()) {
        const auto& distensibility = *parameters.pressure_distensibility;
        if (parameters.flow_adaptation.has_value() ||
            !positive_finite(
                core::in_cubic_meters_per_second(distensibility.reference_cardiac_output)) ||
            !std::isfinite(core::in_pascals(distensibility.reference_left_atrial_pressure)) ||
            core::in_pascals(distensibility.reference_left_atrial_pressure) < 0.0 ||
            !positive_finite(core::in_per_pascal(distensibility.coefficient)) ||
            (distensibility.older_coefficient.has_value() &&
             (!parameters.age_conditioning.has_value() ||
              !positive_finite(core::in_per_pascal(*distensibility.older_coefficient))))) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Pulmonary pressure distensibility requires positive finite reference flow and "
                "coefficient, non-negative finite reference left-atrial pressure, an age rule for "
                "any older coefficient, and cannot be combined with empirical flow adaptation"};
        }
    }
    if (parameters.age_conditioning.has_value()) {
        const auto& conditioning = *parameters.age_conditioning;
        const auto young_multiplier = conditioning.young_resistance_multiplier.si_value();
        const auto older_multiplier = conditioning.older_resistance_multiplier.si_value();
        if (!std::isfinite(conditioning.age_years) ||
            !std::isfinite(conditioning.minimum_supported_age_years) ||
            !std::isfinite(conditioning.young_upper_age_years) ||
            !std::isfinite(conditioning.older_lower_age_years) ||
            !std::isfinite(conditioning.maximum_supported_age_years) ||
            conditioning.minimum_supported_age_years > conditioning.age_years ||
            conditioning.age_years > conditioning.maximum_supported_age_years ||
            conditioning.minimum_supported_age_years >= conditioning.young_upper_age_years ||
            conditioning.young_upper_age_years >= conditioning.older_lower_age_years ||
            conditioning.older_lower_age_years >= conditioning.maximum_supported_age_years ||
            !positive_finite(young_multiplier) || !positive_finite(older_multiplier)) {
            throw core::MehlissaError{
                core::ErrorCode::data_invalid,
                "Pulmonary age conditioning requires an age inside ordered calibration bounds "
                "and positive finite resistance multipliers"};
        }
    }
}

[[nodiscard]] PulmonaryZeroDimensionalConfig
validated_config(PulmonaryZeroDimensionalConfig config) {
    validate_parameters(config.parameters);
    return config;
}

[[nodiscard]] PulmonaryCirculationConfig
make_transit_config(const PulmonaryZeroDimensionalConfig& config) {
    return {
        config.component_name,
        config.model_id,
        config.entry_port_id,
        config.exit_port_id,
        config.return_target_model_id,
        config.return_target_port_id,
        {{"whole-pulmonary-circulation", config.parameters.pulmonary_transit_time}},
    };
}

struct EffectiveHemodynamics final {
    core::VascularResistance resistance;
    core::VascularCompliance compliance;
    core::Dimensionless flow_ratio;
    core::Dimensionless age_resistance_multiplier;
    core::Pressure equilibrium_pressure;
    core::VascularResistance zero_pressure_resistance;
    core::InversePressure pressure_distensibility;
};

[[nodiscard]] EffectiveHemodynamics
effective_hemodynamics(const PulmonaryZeroDimensionalParameters& parameters,
                       const core::FlowRate inflow) noexcept {
    auto flow_ratio = 1.0;
    auto flow_resistance_multiplier = 1.0;
    auto flow_compliance_multiplier = 1.0;
    if (parameters.flow_adaptation.has_value()) {
        const auto& adaptation = *parameters.flow_adaptation;
        const auto raw_ratio =
            core::in_cubic_meters_per_second(inflow) /
            core::in_cubic_meters_per_second(adaptation.reference_cardiac_output);
        flow_ratio = std::clamp(raw_ratio, 1.0, adaptation.maximum_flow_ratio.si_value());
        flow_resistance_multiplier = std::pow(flow_ratio, adaptation.resistance_exponent);
        flow_compliance_multiplier = std::pow(flow_ratio, adaptation.compliance_exponent);
    }

    auto age_resistance_multiplier = 1.0;
    if (parameters.age_conditioning.has_value()) {
        const auto& conditioning = *parameters.age_conditioning;
        if (conditioning.age_years < conditioning.young_upper_age_years) {
            age_resistance_multiplier = conditioning.young_resistance_multiplier.si_value();
        } else if (conditioning.age_years >= conditioning.older_lower_age_years) {
            age_resistance_multiplier = conditioning.older_resistance_multiplier.si_value();
        }
    }
    const auto age_adjusted_reference_resistance =
        parameters.pulmonary_vascular_resistance * age_resistance_multiplier;
    if (parameters.pressure_distensibility.has_value()) {
        const auto& distensibility = *parameters.pressure_distensibility;
        auto effective_coefficient = distensibility.coefficient;
        if (distensibility.older_coefficient.has_value() &&
            parameters.age_conditioning.has_value() &&
            parameters.age_conditioning->age_years >=
                parameters.age_conditioning->older_lower_age_years) {
            effective_coefficient = *distensibility.older_coefficient;
        }
        const auto alpha = core::in_per_pascal(effective_coefficient);
        const auto reference_flow =
            core::in_cubic_meters_per_second(distensibility.reference_cardiac_output);
        const auto reference_left_atrial_pressure =
            core::in_pascals(distensibility.reference_left_atrial_pressure);
        const auto reference_target_pressure = core::in_pascals(
            distensibility.reference_left_atrial_pressure +
            age_adjusted_reference_resistance * distensibility.reference_cardiac_output);
        const auto zero_pressure_resistance = core::pascal_seconds_per_cubic_meter(
            (std::pow(1.0 + alpha * reference_target_pressure, 5.0) -
             std::pow(1.0 + alpha * reference_left_atrial_pressure, 5.0)) /
            (5.0 * alpha * reference_flow));
        const auto left_atrial_pressure = core::in_pascals(parameters.left_atrial_pressure);
        const auto pressure_term =
            std::pow(1.0 + alpha * left_atrial_pressure, 5.0) +
            5.0 * alpha * core::in_pascal_seconds_per_cubic_meter(zero_pressure_resistance) *
                core::in_cubic_meters_per_second(inflow);
        const auto equilibrium = core::pascals((std::pow(pressure_term, 0.2) - 1.0) / alpha);
        const auto effective_resistance = (equilibrium - parameters.left_atrial_pressure) / inflow;
        return {
            effective_resistance,
            parameters.pulmonary_arterial_compliance,
            core::Dimensionless::from_si(core::in_cubic_meters_per_second(inflow) / reference_flow),
            core::Dimensionless::from_si(age_resistance_multiplier),
            equilibrium,
            zero_pressure_resistance,
            effective_coefficient,
        };
    }

    const auto resistance = age_adjusted_reference_resistance * flow_resistance_multiplier;
    return {
        resistance,
        parameters.pulmonary_arterial_compliance * flow_compliance_multiplier,
        core::Dimensionless::from_si(flow_ratio),
        core::Dimensionless::from_si(age_resistance_multiplier),
        parameters.left_atrial_pressure + resistance * inflow,
        resistance,
        core::per_pascal(0.0),
    };
}

[[nodiscard]] core::FlowRate
outflow_from_pressure(const PulmonaryZeroDimensionalParameters& parameters,
                      const core::Pressure pressure,
                      const EffectiveHemodynamics& effective) noexcept {
    const auto alpha = core::in_per_pascal(effective.pressure_distensibility);
    if (alpha == 0.0) {
        return (pressure - parameters.left_atrial_pressure) / effective.resistance;
    }
    const auto upstream = 1.0 + alpha * core::in_pascals(pressure);
    const auto downstream = 1.0 + alpha * core::in_pascals(parameters.left_atrial_pressure);
    const auto flow =
        (std::pow(upstream, 5.0) - std::pow(downstream, 5.0)) /
        (5.0 * alpha * core::in_pascal_seconds_per_cubic_meter(effective.zero_pressure_resistance));
    return core::cubic_meters_per_second(flow);
}

} // namespace

PulmonaryZeroDimensionalModel::PulmonaryZeroDimensionalModel(PulmonaryZeroDimensionalConfig config)
    : config_{validated_config(std::move(config))}, transit_{make_transit_config(config_)},
      prescribed_inflow_{config_.parameters.baseline_cardiac_output},
      mean_pulmonary_arterial_pressure_{
          effective_hemodynamics(config_.parameters, prescribed_inflow_).equilibrium_pressure} {}

std::string_view PulmonaryZeroDimensionalModel::name() const noexcept { return transit_.name(); }
std::string_view PulmonaryZeroDimensionalModel::model_id() const noexcept {
    return transit_.model_id();
}
bool PulmonaryZeroDimensionalModel::accepts_entity_at(
    const std::string_view port_id) const noexcept {
    return transit_.accepts_entity_at(port_id);
}
bool PulmonaryZeroDimensionalModel::emits_entity_at(const std::string_view port_id) const noexcept {
    return transit_.emits_entity_at(port_id);
}

void PulmonaryZeroDimensionalModel::initialize(core::SimulationContext& context) {
    transit_.initialize(context);
    prescribed_inflow_ = config_.parameters.baseline_cardiac_output;
    mean_pulmonary_arterial_pressure_ =
        effective_hemodynamics(config_.parameters, prescribed_inflow_).equilibrium_pressure;
    pending_inflow_.reset();
}

void PulmonaryZeroDimensionalModel::advance(core::SimulationContext& context,
                                            const core::SimulationClock::Duration delta) {
    transit_.advance(context, delta);
    if (pending_inflow_.has_value()) {
        prescribed_inflow_ = *pending_inflow_;
        pending_inflow_.reset();
    }

    const auto effective = effective_hemodynamics(config_.parameters, prescribed_inflow_);
    const auto equilibrium = effective.equilibrium_pressure;
    const auto time_constant = effective.resistance * effective.compliance;
    const auto elapsed_seconds = static_cast<double>(delta.count()) /
                                 static_cast<double>(core::SimulationClock::Duration::period::den);
    const auto decay = std::exp(-elapsed_seconds / core::in_seconds(time_constant));
    mean_pulmonary_arterial_pressure_ =
        equilibrium + (mean_pulmonary_arterial_pressure_ - equilibrium) * decay;
}

void PulmonaryZeroDimensionalModel::finalize(core::SimulationContext& context) noexcept {
    transit_.finalize(context);
}

void PulmonaryZeroDimensionalModel::accept_entity(coupling::EntityTransfer transfer) {
    transit_.accept_entity(std::move(transfer));
}

std::vector<coupling::EntityTransfer> PulmonaryZeroDimensionalModel::take_outbound_entities() {
    return transit_.take_outbound_entities();
}

void PulmonaryZeroDimensionalModel::accept_conserved_transfer(
    coupling::ConservedTransfer transfer) {
    std::optional<core::FlowRate> accepted_inflow;
    if (const auto* flow = std::get_if<coupling::VolumeFlowTransfer>(&transfer); flow != nullptr) {
        if (pending_inflow_.has_value() && *pending_inflow_ != flow->flow_rate) {
            throw core::MehlissaError{
                core::ErrorCode::invariant_violated,
                "Pulmonary 0D model received conflicting flow prescriptions at one sync point"};
        }
        accepted_inflow = flow->flow_rate;
    }
    transit_.accept_conserved_transfer(std::move(transfer));
    if (accepted_inflow.has_value()) {
        pending_inflow_ = *accepted_inflow;
    }
}

std::vector<coupling::ConservedTransfer>
PulmonaryZeroDimensionalModel::take_outbound_conserved_transfers() {
    return transit_.take_outbound_conserved_transfers();
}

std::size_t PulmonaryZeroDimensionalModel::resident_conserved_transfer_count() const noexcept {
    return transit_.resident_conserved_transfer_count();
}

PulmonaryZeroDimensionalState PulmonaryZeroDimensionalModel::state() const noexcept {
    const auto effective = effective_hemodynamics(config_.parameters, prescribed_inflow_);
    const auto pulmonary_outflow =
        outflow_from_pressure(config_.parameters, mean_pulmonary_arterial_pressure_, effective);
    const auto right_fraction = config_.parameters.right_lung_perfusion_fraction.si_value();
    return {
        mean_pulmonary_arterial_pressure_,
        prescribed_inflow_,
        pulmonary_outflow,
        pulmonary_outflow * right_fraction,
        pulmonary_outflow * (1.0 - right_fraction),
        effective.resistance / right_fraction,
        effective.resistance / (1.0 - right_fraction),
        effective.resistance * effective.compliance,
        effective.resistance,
        effective.compliance,
        effective.flow_ratio,
        effective.age_resistance_multiplier,
        effective.zero_pressure_resistance,
        effective.pressure_distensibility,
    };
}

} // namespace mehlissa::models::organ

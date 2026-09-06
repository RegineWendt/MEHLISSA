// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/qualified_cd95_apoptosis_model.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>

namespace mehlissa::models::cell {
namespace {

constexpr auto index(const KallenbergerSpecies species) noexcept {
    return static_cast<std::size_t>(species);
}

constexpr std::array<std::string_view, kallenberger_species_count> species_ids{
    "CD95",  "FADD",    "DISC",        "p55free", "DISCp55", "p30",
    "p43",   "p18",     "p18inactive", "Bid",     "tBid",    "PrNES_mCherry",
    "PrNES", "mCherry", "PrER_mGFP",   "PrER",    "mGFP",    "CD95L"};

constexpr KallenbergerMinimalParameters source_parameters{
    0.000811711012144556, 0.00566528253772301, 0.000491828591049766, 0.0114186392006403,
    0.000446994772958953, 0.00343995957326369, 0.0949914492651531,   0.00052867403363568,
    0.00152252549827479,  8.98496674617627,    15.421878766215};

constexpr KallenbergerMinimalDefinition cd95_hela_definition{
    {"BIOMD0000000523", "calibration-like-CD95-HeLa-average-cell",
     "8605e43f8e2fd364f122d579341891c0058ef778",
     "2afe6758ab396038e71fcb1716fefcfec67656b8bd0bfb3da8d4e1eda9524ff4", "CC0-1.0"},
    source_parameters,
    {{116.0, 93.0, 0.0, 155.0, 0.0, 0.0, 0.0, 0.0, 0.0, 236.0, 0.0, 973.0, 0.0, 0.0, 5178.0, 0.0,
      0.0, 16.6}}};

constexpr KallenbergerMinimalDefinition wild_type_definition{
    {"BIOMD0000000524", "publication-wild-type-HeLa-average-cell",
     "d091308a14fb4301a4a2b1b567ea874484bb97e6",
     "4bf4a5bcda5b43a551bcdda09fca91a5e777d2c5db1eafcb17dcb6f1574221bc", "CC0-1.0"},
    source_parameters,
    {{12.0, 90.0, 0.0, 127.0, 0.0, 0.0, 0.0, 0.0, 0.0, 224.0, 0.0, 1909.0, 0.0, 0.0, 3316.0, 0.0,
      0.0, 16.6}}};

[[noreturn]] void invalid(const std::string& message) {
    throw core::MehlissaError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] bool positive_finite(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

void validate_parameters(const KallenbergerMinimalParameters& p) {
    const std::array values{
        p.kon_fadd, p.koff_fadd,  p.kdisc, p.kd216, p.kd374trans_p55, p.kd374trans_p43, p.kdiss_p18,
        p.kbid,     p.kd374probe, p.kdr,   p.kdl};
    if (!std::ranges::all_of(values, positive_finite)) {
        invalid("Kallenberger minimal mechanism parameters must be positive and finite");
    }
}

void validate_state(const KallenbergerState& state, const std::string_view stage) {
    constexpr auto negative_roundoff_limit = -1.0e-10;
    for (const auto value : state.values) {
        if (!std::isfinite(value) || value < negative_roundoff_limit) {
            invalid("Kallenberger state is nonfinite or materially negative during " +
                    std::string{stage});
        }
    }
}

[[nodiscard]] KallenbergerState add_scaled(const KallenbergerState& state,
                                           const KallenbergerState& slope,
                                           const double scale) noexcept {
    KallenbergerState result{};
    for (std::size_t i = 0; i < result.values.size(); ++i) {
        result.values[i] = state.values[i] + scale * slope.values[i];
    }
    return result;
}

[[nodiscard]] KallenbergerState rk4_step(const KallenbergerMinimalMechanism& mechanism,
                                         const KallenbergerState& state, const double step) {
    const auto k1 = mechanism.derivative(state);
    const auto k2 = mechanism.derivative(add_scaled(state, k1, 0.5 * step));
    const auto k3 = mechanism.derivative(add_scaled(state, k2, 0.5 * step));
    const auto k4 = mechanism.derivative(add_scaled(state, k3, step));
    KallenbergerState result{};
    for (std::size_t i = 0; i < result.values.size(); ++i) {
        result.values[i] =
            state.values[i] +
            step * (k1.values[i] + 2.0 * k2.values[i] + 2.0 * k3.values[i] + k4.values[i]) / 6.0;
        if (result.values[i] < 0.0 && result.values[i] >= -1.0e-10) {
            result.values[i] = 0.0;
        }
    }
    validate_state(result, "integration");
    return result;
}

[[nodiscard]] KallenbergerObservables observables(const KallenbergerState& state) noexcept {
    return {state.at(KallenbergerSpecies::prer_mgfp), state.at(KallenbergerSpecies::prnes_mcherry),
            state.at(KallenbergerSpecies::p43), state.at(KallenbergerSpecies::p18)};
}

} // namespace

ModelNativeStateValue KallenbergerState::at(const KallenbergerSpecies species) const noexcept {
    return {values[index(species)]};
}

std::string_view species_id(const KallenbergerSpecies species) noexcept {
    const auto position = index(species);
    return position < species_ids.size() ? species_ids[position] : std::string_view{};
}

const KallenbergerMinimalDefinition&
kallenberger_minimal_definition(const KallenbergerCase source_case) {
    switch (source_case) {
    case KallenbergerCase::cd95_hela:
        return cd95_hela_definition;
    case KallenbergerCase::wild_type_hela:
        return wild_type_definition;
    }
    invalid("Unknown Kallenberger source case");
}

KallenbergerMinimalMechanism::KallenbergerMinimalMechanism(KallenbergerMinimalParameters parameters)
    : parameters_{parameters} {
    validate_parameters(parameters_);
}

double KallenbergerMinimalMechanism::assigned_cd95_activity(const KallenbergerState& state) const {
    const auto cd95 = state.at(KallenbergerSpecies::cd95).value;
    const auto ligand = state.at(KallenbergerSpecies::cd95l).value;
    const auto kdl = parameters_.kdl;
    const auto kdr = parameters_.kdr;
    const auto denominator = (ligand + kdl) * (cd95 * cd95 * kdl * kdl + kdr * ligand * ligand +
                                               2.0 * kdr * kdl * ligand + kdr * kdl * kdl);
    if (!(denominator > 0.0) || !std::isfinite(denominator)) {
        invalid("Kallenberger CD95 activity assignment has an invalid denominator");
    }
    return cd95 * cd95 * cd95 * kdl * kdl * ligand / denominator;
}

KallenbergerState KallenbergerMinimalMechanism::derivative(const KallenbergerState& state) const {
    validate_state(state, "derivative evaluation");
    const auto value = [&state](const KallenbergerSpecies species) {
        return state.at(species).value;
    };
    const auto active_cd95 = assigned_cd95_activity(state);
    const auto v1 = parameters_.kon_fadd * active_cd95 * value(KallenbergerSpecies::fadd);
    const auto v2 = parameters_.koff_fadd * value(KallenbergerSpecies::disc);
    const auto v3 =
        parameters_.kdisc * value(KallenbergerSpecies::p55free) * value(KallenbergerSpecies::disc);
    const auto v4 = parameters_.kd216 * value(KallenbergerSpecies::discp55);
    const auto v5 = parameters_.kd216 * value(KallenbergerSpecies::p43);
    const auto trans_p55 = value(KallenbergerSpecies::discp55) + value(KallenbergerSpecies::p30);
    const auto v6 = parameters_.kd374trans_p55 * value(KallenbergerSpecies::discp55) * trans_p55;
    const auto v7 = parameters_.kd374trans_p43 * value(KallenbergerSpecies::discp55) *
                    value(KallenbergerSpecies::p43);
    const auto v8 = parameters_.kd374trans_p55 * value(KallenbergerSpecies::p30) * trans_p55;
    const auto v9 = parameters_.kd374trans_p43 * value(KallenbergerSpecies::p30) *
                    value(KallenbergerSpecies::p43);
    const auto v10 = parameters_.kdiss_p18 * value(KallenbergerSpecies::p18);
    const auto active_caspase = value(KallenbergerSpecies::p43) + value(KallenbergerSpecies::p18);
    const auto v11 = parameters_.kbid * value(KallenbergerSpecies::bid) * active_caspase;
    const auto v12 =
        parameters_.kd374probe * value(KallenbergerSpecies::prnes_mcherry) * active_caspase;
    const auto v13 = parameters_.kd374probe * value(KallenbergerSpecies::prer_mgfp) *
                     value(KallenbergerSpecies::p18);

    KallenbergerState result{};
    result.values[index(KallenbergerSpecies::fadd)] = -v1 + v2;
    result.values[index(KallenbergerSpecies::disc)] = v1 - v2 - v3 + v5 + v8 + v9;
    result.values[index(KallenbergerSpecies::p55free)] = -v3;
    result.values[index(KallenbergerSpecies::discp55)] = v3 - v4 - v6 - v7;
    result.values[index(KallenbergerSpecies::p30)] = v4 - v8 - v9;
    result.values[index(KallenbergerSpecies::p43)] = v6 + v7 - v5;
    result.values[index(KallenbergerSpecies::p18)] = v5 + v8 + v9 - v10;
    result.values[index(KallenbergerSpecies::p18inactive)] = v10;
    result.values[index(KallenbergerSpecies::bid)] = -v11;
    result.values[index(KallenbergerSpecies::tbid)] = v11;
    result.values[index(KallenbergerSpecies::prnes_mcherry)] = -v12;
    result.values[index(KallenbergerSpecies::prnes)] = v12;
    result.values[index(KallenbergerSpecies::mcherry)] = v12;
    result.values[index(KallenbergerSpecies::prer_mgfp)] = -v13;
    result.values[index(KallenbergerSpecies::prer)] = v13;
    result.values[index(KallenbergerSpecies::mgfp)] = v13;
    return result;
}

std::vector<QualifiedCd95Sample> KallenbergerMinimalMechanism::integrate(
    const KallenbergerState& initial_state, const ModelNativeTime end_time,
    const ModelNativeTime output_interval, const ModelNativeTime maximum_internal_step,
    std::size_t& integration_steps) const {
    if (!positive_finite(end_time.value) || !positive_finite(output_interval.value) ||
        !positive_finite(maximum_internal_step.value) || output_interval.value > end_time.value ||
        maximum_internal_step.value > output_interval.value) {
        invalid("Kallenberger integration request is incomplete or nonphysical");
    }
    const auto interval_count = end_time.value / output_interval.value;
    const auto rounded_intervals = std::round(interval_count);
    if (std::abs(interval_count - rounded_intervals) > 1.0e-12 || rounded_intervals > 1'000'000.0) {
        invalid("Kallenberger integration requires an exact bounded output grid");
    }
    auto state = initial_state;
    validate_state(state, "initialization");
    integration_steps = 0;
    const auto point_count = static_cast<std::size_t>(rounded_intervals) + 1;
    const auto step_ratio = output_interval.value / maximum_internal_step.value;
    const auto steps_per_interval = static_cast<std::size_t>(std::ceil(step_ratio - 1.0e-12));
    if (steps_per_interval == 0 ||
        steps_per_interval > 100'000'000 / static_cast<std::size_t>(rounded_intervals)) {
        invalid("Kallenberger integration exceeds its integration-step bound");
    }
    const auto fixed_step = output_interval.value / static_cast<double>(steps_per_interval);
    std::vector<QualifiedCd95Sample> samples;
    samples.reserve(point_count);
    samples.push_back({{0.0}, state, observables(state)});
    for (std::size_t point = 1; point < point_count; ++point) {
        const auto target_time = static_cast<double>(point) * output_interval.value;
        for (std::size_t substep = 0; substep < steps_per_interval; ++substep) {
            state = rk4_step(*this, state, fixed_step);
            ++integration_steps;
        }
        samples.push_back({{target_time}, state, observables(state)});
    }
    return samples;
}

std::string_view QualifiedCd95ApoptosisAdapter::kind() const noexcept {
    return qualified_cd95_apoptosis_kind;
}

QualifiedCd95Response
QualifiedCd95ApoptosisAdapter::evaluate(const QualifiedCd95Request& request) const {
    const auto& definition = kallenberger_minimal_definition(request.source_case);
    const auto expected_stimulus = definition.initial_state.at(KallenbergerSpecies::cd95l).value;
    if (request.request_id.empty() || request.stimulus.species_id != "CD95L" ||
        request.stimulus.unit_semantics != unresolved_model_native_unit ||
        request.stimulus.initial_value.value != expected_stimulus ||
        !positive_finite(request.end_time.value)) {
        invalid("Qualified CD95 adapter request violates its typed no-refit boundary");
    }

    KallenbergerMinimalMechanism mechanism{definition.parameters};
    QualifiedCd95Response response{request.request_id,
                                   qualified_cd95_apoptosis_kind,
                                   definition.source,
                                   unresolved_model_native_unit,
                                   unresolved_model_native_unit,
                                   0,
                                   {}};
    response.samples =
        mechanism.integrate(definition.initial_state, request.end_time, request.output_interval,
                            request.maximum_internal_step, response.integration_steps);
    return response;
}

} // namespace mehlissa::models::cell

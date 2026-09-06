// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_SCENARIOS_FDG_PET_FDG_PET_MODEL_HPP
#define MEHLISSA_SCENARIOS_FDG_PET_FDG_PET_MODEL_HPP

#include "mehlissa/core/quantity.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace mehlissa::scenarios::fdg_pet {

using Activity = core::Quantity<core::Dimension<0, -1, 0>>;
using ActivityConcentration = core::Quantity<core::Dimension<-3, -1, 0>>;

[[nodiscard]] constexpr Activity becquerels(double value) noexcept {
    return Activity::from_si(value);
}
[[nodiscard]] constexpr Activity megabecquerels(double value) noexcept {
    return becquerels(value * 1.0e6);
}
[[nodiscard]] constexpr ActivityConcentration becquerels_per_milliliter(double value) noexcept {
    return ActivityConcentration::from_si(value * 1.0e6);
}
[[nodiscard]] constexpr double in_becquerels(Activity value) noexcept { return value.si_value(); }
[[nodiscard]] constexpr double in_becquerels_per_milliliter(ActivityConcentration value) noexcept {
    return value.si_value() / 1.0e6;
}

enum class DecayReference { injection_time_corrected, scan_time_uncorrected };

struct Administration final {
    Activity injected_activity{megabecquerels(210.0)};
    core::Mass body_mass{core::Mass::from_si(70.0)};
    core::Time injection_duration{core::seconds(35.0)};
};

struct TissueKinetics final {
    core::FirstOrderRate k1{};
    core::FirstOrderRate k2{};
    core::FirstOrderRate k3{};
    core::FirstOrderRate k4{};
    double vascular_fraction{};
    core::Volume volume{};
};

struct CandidateParameters final {
    Administration administration{};
    core::FlowRate cardiac_output{};
    std::array<double, 4> blood_amplitudes_per_minute_squared{};
    std::array<double, 4> blood_decay_per_minute{};
    TissueKinetics lung{};
    TissueKinetics liver{};
    TissueKinetics kidney{};
    core::FirstOrderRate renal_excretion{};
    core::Time fluorine18_half_life{};
    DecayReference decay_reference{DecayReference::injection_time_corrected};
};

struct Frame final {
    core::Time start{};
    core::Time duration{};
};

struct FramePrediction final {
    Frame frame{};
    ActivityConcentration aortic_input{};
    ActivityConcentration lung{};
    ActivityConcentration liver{};
    ActivityConcentration kidney{};
    Activity urinary_bladder{};
};

[[nodiscard]] CandidateParameters
source_disjoint_reference_candidate(const Administration& administration = {});
[[nodiscard]] std::vector<FramePrediction> simulate(const CandidateParameters& parameters,
                                                    const std::vector<Frame>& frames,
                                                    core::Time maximum_step = core::seconds(0.1));

} // namespace mehlissa::scenarios::fdg_pet

#endif

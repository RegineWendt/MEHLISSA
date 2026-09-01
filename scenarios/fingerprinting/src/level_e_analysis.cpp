// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_e_analysis.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

namespace mehlissa::scenarios::fingerprinting {
namespace {

constexpr double z_95 = 1.959963984540054;

[[noreturn]] void invalid(const std::string& message) {
    throw ScenarioProfileError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] ClassificationKind classify(const bool present, const bool detected) noexcept {
    if (present && detected) {
        return ClassificationKind::true_positive;
    }
    if (!present && !detected) {
        return ClassificationKind::true_negative;
    }
    return present ? ClassificationKind::false_negative : ClassificationKind::false_positive;
}

[[nodiscard]] ProportionInterval wilson_interval(const std::uint64_t successes,
                                                 const std::uint64_t total) {
    const auto count = static_cast<double>(total);
    const auto estimate = static_cast<double>(successes) / count;
    const auto z_squared = z_95 * z_95;
    const auto denominator = 1.0 + z_squared / count;
    const auto center = (estimate + z_squared / (2.0 * count)) / denominator;
    const auto margin =
        z_95 *
        std::sqrt((estimate * (1.0 - estimate) / count) + (z_squared / (4.0 * count * count))) /
        denominator;
    return {estimate, std::max(0.0, center - margin), std::min(1.0, center + margin)};
}

[[nodiscard]] std::optional<ProportionInterval> proportion(const std::uint64_t successes,
                                                           const std::uint64_t total) {
    return total == 0 ? std::nullopt
                      : std::optional<ProportionInterval>{wilson_interval(successes, total)};
}

} // namespace

std::string_view to_string(const ClassificationKind classification) noexcept {
    switch (classification) {
    case ClassificationKind::true_positive:
        return "true_positive";
    case ClassificationKind::true_negative:
        return "true_negative";
    case ClassificationKind::false_positive:
        return "false_positive";
    case ClassificationKind::false_negative:
        return "false_negative";
    }
    return "unknown";
}

std::vector<LevelECase> default_level_e_cases(const LevelAPlan& plan) {
    const auto reference = default_level_b_detection_input(plan);
    return {
        {"present-reference", reference.ligand_concentration, reference.exposure_duration, true},
        {"present-below-threshold", core::moles_per_cubic_meter(1.0e-9),
         reference.exposure_duration, true},
        {"absent-cross-reactive-reference", reference.ligand_concentration,
         reference.exposure_duration, false},
        {"absent-below-threshold", core::moles_per_cubic_meter(1.0e-9), reference.exposure_duration,
         false},
    };
}

LevelEAnalysisResult run_level_e_analysis(const LevelAPlan& plan,
                                          const std::vector<LevelECase>& cases) {
    if (cases.empty()) {
        invalid("M7.7 analysis requires at least one labelled case");
    }

    std::unordered_set<std::string> case_ids;
    std::vector<LevelECaseResult> results;
    results.reserve(cases.size());
    LevelEClassificationSummary summary;
    for (const auto& item : cases) {
        const auto concentration = core::in_moles_per_cubic_meter(item.ligand_concentration);
        if (item.case_id.empty() || !case_ids.insert(item.case_id).second ||
            !std::isfinite(concentration) || concentration < 0.0 ||
            item.exposure_duration <= core::SimulationClock::Duration::zero()) {
            invalid("M7.7 labelled cases require unique identities and valid inputs");
        }
        const auto detection =
            run_level_b_detection(plan, {item.ligand_concentration, item.exposure_duration, 0.0});
        const auto classification = classify(item.target_present, detection.detected);
        switch (classification) {
        case ClassificationKind::true_positive:
            ++summary.true_positive;
            break;
        case ClassificationKind::true_negative:
            ++summary.true_negative;
            break;
        case ClassificationKind::false_positive:
            ++summary.false_positive;
            break;
        case ClassificationKind::false_negative:
            ++summary.false_negative;
            break;
        }
        results.push_back({item.case_id, item.ligand_concentration, item.exposure_duration,
                           item.target_present, detection.detected, detection.final_bound_fraction,
                           classification});
    }

    const auto positive_total = summary.true_positive + summary.false_negative;
    const auto negative_total = summary.true_negative + summary.false_positive;
    summary.sensitivity = proportion(summary.true_positive, positive_total);
    summary.specificity = proportion(summary.true_negative, negative_total);
    summary.false_positive_rate = proportion(summary.false_positive, negative_total);
    summary.false_negative_rate = proportion(summary.false_negative, positive_total);

    return {plan.profile.run.id + ":level-e-analysis",
            std::move(results),
            std::move(summary),
            {"ligand_concentration", "exposure_duration", "analyst_supplied_target_label"},
            {"Truth labels are analyst-supplied scenario inputs, not clinical diagnoses.",
             "The default four-case matrix is a software demonstration and is far too small "
             "for empirical performance estimation.",
             "False-positive and false-negative cases deliberately expose concentration/label "
             "mismatch; they do not estimate prevalence, cross-reactivity, or assay accuracy.",
             "Binding affinity, perfusion, injection site, device counts, stochastic transport, "
             "and patient variability are not propagated by this Level-E increment."}};
}

} // namespace mehlissa::scenarios::fingerprinting

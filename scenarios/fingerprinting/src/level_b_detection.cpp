// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_b_detection.hpp>

#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cosimulation/receptor_detection_adapter.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <algorithm>
#include <cmath>
#include <ranges>
#include <string>

namespace mehlissa::scenarios::fingerprinting {
namespace {

[[noreturn]] void invalid(const std::string& message) {
    throw ScenarioProfileError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] const ResolvedArtifact& artifact(const LevelAPlan& plan, const ArtifactRole role) {
    const auto found = std::ranges::find(plan.artifacts, role, &ResolvedArtifact::role);
    if (found == plan.artifacts.end()) {
        invalid("M7.4 runtime artifact is missing: " + std::string{to_string(role)});
    }
    return *found;
}

[[nodiscard]] core::SimulationClock::Duration localization_time(const LevelAPlan& plan) {
    const auto found = std::ranges::find(plan.timer_run.events,
                                         experiment::FingerprintTimerEventKind::first_localization,
                                         &experiment::FingerprintTimerEvent::kind);
    if (found == plan.timer_run.events.end()) {
        invalid("M7.4 selected timer run has no localization event");
    }
    return found->time;
}

} // namespace

LevelBDetectionInput default_level_b_detection_input(const LevelAPlan& plan) {
    const auto& receptor_file = artifact(plan, ArtifactRole::receptor_model);
    const auto profile = models::cell::load_receptor_ligand_profile(
        {receptor_file.definition_path, receptor_file.schema_path});
    return {profile.reference_case.ligand_concentration, profile.reference_case.observation_time,
            profile.reference_case.initial_bound_fraction};
}

LevelBDetectionResult run_level_b_detection(const LevelAPlan& plan,
                                            const LevelBDetectionInput& input) {
    const auto concentration = core::in_moles_per_cubic_meter(input.ligand_concentration);
    if (!std::isfinite(concentration) || concentration < 0.0 ||
        input.exposure_duration <= core::SimulationClock::Duration::zero() ||
        !std::isfinite(input.initial_bound_fraction) || input.initial_bound_fraction < 0.0 ||
        input.initial_bound_fraction > 1.0) {
        invalid("M7.4 detection input is outside the receptor-model contract");
    }

    const auto& receptor_file = artifact(plan, ArtifactRole::receptor_model);
    const auto profile = models::cell::load_receptor_ligand_profile(
        {receptor_file.definition_path, receptor_file.schema_path});
    const auto model = models::cell::make_receptor_ligand_model(profile);
    const models::cell::ReceptorLigandRequest request{
        plan.profile.run.id + ":binding:FP9", profile.model.ligand_id, profile.model.compartment_id,
        input.ligand_concentration,           input.exposure_duration, input.initial_bound_fraction,
    };
    const auto response = model->evaluate(request);

    const auto& locator_file = artifact(plan, ArtifactRole::locator_device);
    const auto locator = models::iot::load_nanodevice_profile(
        {locator_file.definition_path, locator_file.schema_path});
    const auto event_id = plan.profile.run.id + ":detection:FP9";
    std::optional<core::SimulationClock::Duration> absolute;
    if (response.detection_threshold_reached) {
        auto event = models::cosimulation::make_receptor_detection_event(
            response, {event_id, locator.device.device_id});
        event.detected_at += localization_time(plan);
        models::iot::validate_molecular_detection_event(event);
        absolute = event.detected_at;
    }

    return {profile.profile_id,
            std::string{model->model_id()},
            locator.device.device_id,
            event_id,
            response.ligand_id,
            input.ligand_concentration,
            input.exposure_duration,
            response.equilibrium_bound_fraction,
            response.final_bound_fraction,
            response.detection_threshold_reached,
            response.first_threshold_crossing_time,
            absolute};
}

void apply_level_b_detection(LevelARuntimeResult& runtime, const LevelBDetectionResult& detection) {
    const auto recognition = std::ranges::find(runtime.stages, StageKind::molecular_recognition,
                                               &StageTraceEntry::stage);
    const auto assembly =
        std::ranges::find(runtime.stages, StageKind::fingerprint_assembly, &StageTraceEntry::stage);
    if (recognition == runtime.stages.end() || assembly == runtime.stages.end()) {
        invalid("M7.4 cannot find the recognition-to-assembly edge in the runtime trace");
    }
    if (!detection.detected || !detection.absolute_detection_time.has_value()) {
        invalid("M7.4 cannot continue a positive fingerprinting trace without detection");
    }
    recognition->time = detection.absolute_detection_time.value();
    recognition->basis = ExecutionBasis::model_execution;
    recognition->component_id = detection.receptor_model_id;
    recognition->output_identity = detection.event_id;
    recognition->qualification =
        "Level B evaluates the selected concentration with the selected analytical 1:1 "
        "receptor model. Parameters remain a synthetic software surrogate, not an FP9-specific "
        "proteomic calibration.";
    assembly->input_identity = detection.event_id;
    if (recognition->time > assembly->time) {
        invalid("M7.4 binding detection occurs after the Level-A message-active milestone");
    }

    const auto receptor = std::ranges::find(runtime.components, ArtifactRole::receptor_model,
                                            &RuntimeComponentIdentity::role);
    if (receptor != runtime.components.end()) {
        receptor->advanced = true;
    }
}

} // namespace mehlissa::scenarios::fingerprinting

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_c_assembly.hpp>

#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <algorithm>
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
        invalid("M7.5 runtime artifact is missing: " + std::string{to_string(role)});
    }
    return *found;
}

} // namespace

LevelCAssemblyInput default_level_c_assembly_input(const LevelAPlan& plan) {
    return {plan.timer_baseline.target_locator_count, plan.timer_baseline.configured_target_count,
            plan.timer_baseline.assembly_duration};
}

LevelCAssemblyResult run_level_c_assembly(const LevelAPlan& plan,
                                          const LevelBDetectionResult& detection,
                                          const LevelCAssemblyInput& input) {
    if (!detection.detected || !detection.absolute_detection_time.has_value()) {
        invalid("M7.5 tile release requires a positive Level-B detection");
    }
    if (input.available_detected_locators == 0 || input.required_unique_tiles == 0 ||
        input.required_unique_tiles > plan.timer_baseline.configured_target_count ||
        input.assembly_duration <= core::SimulationClock::Duration::zero()) {
        invalid("M7.5 assembly input is outside the FP9 surrogate contract");
    }

    const auto& locator_file = artifact(plan, ArtifactRole::locator_device);
    const auto locator = models::iot::load_nanodevice_profile(
        {locator_file.definition_path, locator_file.schema_path});
    if (locator.device.payloads.size() != 1) {
        invalid("M7.5 selected locator must carry exactly one fingerprint-tile payload");
    }
    const auto& tile_payload = locator.device.payloads.front();
    if (tile_payload.unit_count.value_or(0) == 0) {
        invalid("M7.5 selected locator must carry one unit-count fingerprint-tile payload");
    }

    const auto released_count =
        std::min(input.available_detected_locators, input.required_unique_tiles);
    std::vector<FingerprintTileRelease> releases;
    releases.reserve(static_cast<std::size_t>(released_count));
    const auto started_at = detection.absolute_detection_time.value();
    for (std::uint64_t index = 0; index < released_count; ++index) {
        const auto ordinal = std::to_string(index + 1);
        releases.push_back({plan.profile.run.id + ":release:" + ordinal, tile_payload.payload_id,
                            plan.profile.target.fingerprint_id + ":tile:" + ordinal,
                            detection.event_id, started_at});
    }
    const auto complete = released_count == input.required_unique_tiles;
    std::optional<core::SimulationClock::Duration> completed_at;
    if (complete) {
        completed_at = started_at + input.assembly_duration;
    }

    return {plan.profile.run.id + ":assembly:" + plan.profile.target.fingerprint_id,
            plan.profile.target.fingerprint_id,
            std::move(releases),
            input.required_unique_tiles,
            complete,
            started_at,
            completed_at,
            "Level C executes explicit tile identities and an all-required-tiles rule. The "
            "assembly duration remains the historical FP9 timer and is not a NetTAS or "
            "diffusion calculation."};
}

void apply_level_c_assembly(LevelARuntimeResult& runtime, const LevelCAssemblyResult& assembly) {
    const auto stage =
        std::ranges::find(runtime.stages, StageKind::fingerprint_assembly, &StageTraceEntry::stage);
    const auto collection =
        std::ranges::find(runtime.stages, StageKind::local_collection, &StageTraceEntry::stage);
    if (stage == runtime.stages.end() || collection == runtime.stages.end()) {
        invalid("M7.5 cannot find the assembly-to-collection edge in the runtime trace");
    }
    if (!assembly.complete || !assembly.completed_at.has_value()) {
        invalid("M7.5 cannot continue a positive fingerprinting trace with incomplete assembly");
    }
    stage->time = assembly.completed_at.value();
    stage->basis = ExecutionBasis::software_surrogate;
    stage->component_id = "fp9.explicit-tile-assembly.v1";
    stage->output_identity = assembly.assembly_id;
    stage->qualification = assembly.qualification;
    collection->input_identity = assembly.assembly_id;
    collection->time = std::max(collection->time, stage->time);
}

} // namespace mehlissa::scenarios::fingerprinting

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/runtime_coordinator.hpp>

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/body/body_state_profile.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cosimulation/capillary_cell_signal_profile.hpp>
#include <mehlissa/models/iot/active_gateway_profile.hpp>
#include <mehlissa/models/iot/ban_station_profile.hpp>
#include <mehlissa/models/iot/cluster_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>
#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <algorithm>
#include <chrono>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

namespace mehlissa::scenarios::fingerprinting {
namespace {

using namespace std::chrono_literals;

[[noreturn]] void invalid(const std::string& message) {
    throw ScenarioProfileError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] const ResolvedArtifact& artifact(const LevelAPlan& plan, const ArtifactRole role) {
    const auto found = std::ranges::find(plan.artifacts, role, &ResolvedArtifact::role);
    if (found == plan.artifacts.end()) {
        invalid("M7.2 runtime artifact is missing: " + std::string{to_string(role)});
    }
    return *found;
}

[[nodiscard]] RuntimeComponentIdentity identity(const ArtifactRole role, std::string profile_id,
                                                std::string model_id, const bool advanced) {
    return {role, std::move(profile_id), std::move(model_id), true, advanced};
}

[[nodiscard]] core::SimulationClock::Duration
timer_time(const LevelAPlan& plan, const experiment::FingerprintTimerEventKind kind) {
    const auto found =
        std::ranges::find(plan.timer_run.events, kind, &experiment::FingerprintTimerEvent::kind);
    if (found == plan.timer_run.events.end()) {
        invalid("M7.2 selected timer run is incomplete");
    }
    return found->time;
}

[[nodiscard]] std::vector<StageTraceEntry> make_level_a_trace(const LevelAPlan& plan) {
    const auto injection = timer_time(plan, experiment::FingerprintTimerEventKind::injection);
    const auto localization =
        timer_time(plan, experiment::FingerprintTimerEventKind::first_localization);
    const auto message = timer_time(plan, experiment::FingerprintTimerEventKind::message_active);
    const auto external = timer_time(plan, experiment::FingerprintTimerEventKind::external_report);
    const auto body = injection + (localization - injection) / 2;
    const auto organ = injection + ((localization - injection) * 4) / 5;
    const auto local_collection = message;
    const auto collector_return = external - 30ms;
    const auto gateway = external - 10ms;

    const auto& run_id = plan.profile.run.id;
    const auto entity = run_id + ":entity:1";
    const auto localized = run_id + ":localized:FP9";
    const auto detection = run_id + ":detection:FP9";
    const auto assembled = run_id + ":message:FP9";
    const auto collected = run_id + ":collector:FP9";
    const auto measurement = run_id + ":measurement:FP9";
    const auto report = run_id + ":report:FP9";

    return {
        {0, StageKind::injection, injection, ExecutionBasis::model_execution,
         "bvs95-dissertation-rest", run_id, entity,
         "The selected body component is initialized with a deterministic representative "
         "particle; the population-scale FP9 timing remains historical."},
        {1, StageKind::body_transport, body, ExecutionBasis::historical_timer,
         "bvs95-dissertation-rest", entity, entity,
         "Intermediate body-stage time is a declared partition of the historical "
         "first-localization interval, not an independently calibrated transit prediction."},
        {2, StageKind::organ_transfer, organ, ExecutionBasis::historical_timer,
         "lung.pulmonary-0d.healthy-adult-lobar-parallel.v7", entity, entity,
         "The selected five-lobe organ model is instantiated and advanced; this Level-A "
         "timestamp remains timer-derived."},
        {3, StageKind::capillary_localization, localization, ExecutionBasis::historical_timer,
         "capillary.lung.healthy-adult-rest-supine.v1", entity, localized,
         "The pulmonary capillary component is instantiated and advanced; localization time "
         "is the published FP9 regression target."},
        {4, StageKind::molecular_recognition, localization, ExecutionBasis::historical_timer,
         "fp9-lung-historical-timer-v1", localized, detection,
         "Level A treats localization as immediate recognition; Level B replaces this decision."},
        {5, StageKind::fingerprint_assembly, message, ExecutionBasis::historical_timer,
         "fp9-lung-historical-timer-v1", detection, assembled,
         "Message-active time is the published first-localization plus assembly duration."},
        {6, StageKind::local_collection, local_collection, ExecutionBasis::historical_timer,
         "cluster.fp9.lung.level-a.1", assembled, collected,
         "Level A has no separately published collection delay; Level C/D replace this edge."},
        {7, StageKind::collector_return, collector_return, ExecutionBasis::historical_timer,
         "bvs95-dissertation-rest", collected, collected,
         "The dissertation reports collection and return only as an aggregate interval; this "
         "placeholder preserves ordering without claiming a measured split."},
        {8, StageKind::gateway_measurement, gateway, ExecutionBasis::software_surrogate,
         "gateway.synthetic.wrist", collected, measurement,
         "Ten milliseconds are reserved for the selected software-contract gateway/BAN path."},
        {9, StageKind::external_report, external, ExecutionBasis::historical_timer,
         "station.synthetic.analysis-control", measurement, report,
         "External-report time is the published 1,000-collector FP9 regression target."},
    };
}

} // namespace

std::string_view to_string(const ExecutionBasis basis) noexcept {
    switch (basis) {
    case ExecutionBasis::model_execution:
        return "model_execution";
    case ExecutionBasis::historical_timer:
        return "historical_timer";
    case ExecutionBasis::software_surrogate:
        return "software_surrogate";
    }
    return "unknown";
}

LevelARuntimeResult run_level_a_runtime(const LevelAPlan& plan) {
    validate_scenario_profile(plan.profile);
    if (plan.profile.acceptance.required_stage_order.size() != 10) {
        invalid("M7.2 requires the complete ten-stage fingerprinting contract");
    }

    const auto& body_file = artifact(plan, ArtifactRole::body_model);
    const auto& state_file = artifact(plan, ArtifactRole::body_state);
    auto graph =
        models::body::load_vascular_graph({body_file.definition_path, body_file.schema_path});
    const auto body_model_id = graph.model_id;
    const auto body_model_version = graph.model_version;
    const auto state =
        models::body::load_body_state_profile({state_file.definition_path, state_file.schema_path});
    graph = models::body::apply_body_state_profile(graph, state);
    if (plan.timer_baseline.historical_injection_segment_index >= graph.segments.size()) {
        invalid("M7.2 historical injection segment index is outside the selected body model");
    }
    const auto injection_segment =
        graph.segments[plan.timer_baseline.historical_injection_segment_index].id;

    const auto& organ_file = artifact(plan, ArtifactRole::organ_model);
    const auto organ_definition = models::organ::load_lung_model_definition(
        {organ_file.definition_path, organ_file.schema_path});
    const auto organ_model_id = organ_definition.model.model_id;

    const auto& capillary_file = artifact(plan, ArtifactRole::capillary_model);
    const auto capillary_definition = models::capillary::load_capillary_bed_definition(
        {capillary_file.definition_path, capillary_file.schema_path});
    const auto capillary_model_id = capillary_definition.model.model_id;

    auto body = std::make_unique<models::body::CompartmentTransport>(
        std::move(graph), std::vector<models::body::InjectionEvent>{{0ns, injection_segment, 1}});
    auto organ = models::organ::make_lung_model(organ_definition.model);
    auto capillary = std::make_unique<models::capillary::CapillaryBed>(capillary_definition.model);
    auto* body_observer = body.get();
    core::ComponentHost host{plan.profile.run.master_seed};
    host.add(std::move(body));
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();
    host.advance(1ms);
    if (host.component_count() != 3 || body_observer->injected_particle_count() != 1) {
        invalid("M7.2 failed to initialize and advance the selected physiological stack");
    }
    host.finalize();

    const auto& signal_file = artifact(plan, ArtifactRole::capillary_cell_signal);
    const auto signal = models::cosimulation::load_capillary_cell_signal_profile(
        {signal_file.definition_path, signal_file.schema_path});
    const auto& receptor_file = artifact(plan, ArtifactRole::receptor_model);
    const auto receptor = models::cell::load_receptor_ligand_profile(
        {receptor_file.definition_path, receptor_file.schema_path});
    const auto receptor_model = models::cell::make_receptor_ligand_model(receptor);

    const auto& locator_file = artifact(plan, ArtifactRole::locator_device);
    const auto locator = models::iot::load_nanodevice_profile(
        {locator_file.definition_path, locator_file.schema_path});
    const auto& collector_file = artifact(plan, ArtifactRole::collector_device);
    const auto collector = models::iot::load_nanodevice_profile(
        {collector_file.definition_path, collector_file.schema_path});
    const auto& cluster_file = artifact(plan, ArtifactRole::communication_cluster);
    const auto cluster = models::iot::load_cluster_communication_profile(
        {cluster_file.definition_path, cluster_file.schema_path});
    models::iot::NanodeviceCluster cluster_model{cluster.cluster};
    static_cast<void>(cluster_model.select_route(cluster.reference_case.source_device_id,
                                                 cluster.reference_case.target_device_id,
                                                 cluster.reference_case.strategy));
    const auto& gateway_file = artifact(plan, ArtifactRole::active_gateway);
    const auto gateway = models::iot::load_active_gateway_profile(
        {gateway_file.definition_path, gateway_file.schema_path});
    const auto& ban_file = artifact(plan, ArtifactRole::ban_station);
    const auto ban =
        models::iot::load_ban_station_profile({ban_file.definition_path, ban_file.schema_path});

    std::vector<RuntimeComponentIdentity> components;
    components.reserve(11);
    components.push_back(
        identity(ArtifactRole::body_model, body_model_version, body_model_id, true));
    components.push_back(identity(ArtifactRole::body_state, state.profile_id, body_model_id, true));
    components.push_back(
        identity(ArtifactRole::organ_model, organ_definition.definition_id, organ_model_id, true));
    components.push_back(identity(ArtifactRole::capillary_model, capillary_definition.definition_id,
                                  capillary_model_id, true));
    components.push_back(identity(ArtifactRole::capillary_cell_signal, signal.profile_id,
                                  signal.coupler.target_cell_model_id, false));
    components.push_back(identity(ArtifactRole::receptor_model, receptor.profile_id,
                                  std::string{receptor_model->model_id()}, false));
    components.push_back(identity(ArtifactRole::locator_device, locator.profile_id,
                                  locator.device.device_id, false));
    components.push_back(identity(ArtifactRole::collector_device, collector.profile_id,
                                  collector.device.device_id, false));
    components.push_back(identity(ArtifactRole::communication_cluster, cluster.profile_id,
                                  cluster.cluster.cluster_id, false));
    components.push_back(identity(ArtifactRole::active_gateway, gateway.profile_id,
                                  gateway.gateway.gateway_id, false));
    components.push_back(
        identity(ArtifactRole::ban_station, ban.profile_id, ban.station.station_id, false));

    auto stages = make_level_a_trace(plan);
    if (!std::ranges::is_sorted(stages, {}, &StageTraceEntry::time)) {
        invalid("M7.2 produced an acausal stage trace");
    }
    for (std::size_t index = 0; index < stages.size(); ++index) {
        if (stages[index].ordinal != index ||
            stages[index].stage != plan.profile.acceptance.required_stage_order[index] ||
            (index > 0 && stages[index - 1].output_identity != stages[index].input_identity)) {
            invalid("M7.2 stage trace does not preserve order and identity");
        }
    }

    return {std::string{fingerprinting_runtime_contract_version},
            plan.profile.scenario.id,
            plan.profile.run.id,
            plan.profile.run.master_seed,
            std::move(components),
            std::move(stages),
            1ms};
}

} // namespace mehlissa::scenarios::fingerprinting

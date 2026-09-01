// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/scenarios/fingerprinting/level_d_communication.hpp>

#include <mehlissa/models/iot/active_gateway_profile.hpp>
#include <mehlissa/models/iot/ban_station_profile.hpp>
#include <mehlissa/models/iot/cluster_communication_profile.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <ranges>
#include <string>

namespace mehlissa::scenarios::fingerprinting {
namespace {

using namespace std::chrono_literals;

[[noreturn]] void invalid(const std::string& message) {
    throw ScenarioProfileError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] const ResolvedArtifact& artifact(const LevelAPlan& plan, const ArtifactRole role) {
    const auto found = std::ranges::find(plan.artifacts, role, &ResolvedArtifact::role);
    if (found == plan.artifacts.end()) {
        invalid("M7.6 runtime artifact is missing: " + std::string{to_string(role)});
    }
    return *found;
}

[[nodiscard]] core::SimulationClock::Duration external_timer_time(const LevelAPlan& plan) {
    const auto found = std::ranges::find(plan.timer_run.events,
                                         experiment::FingerprintTimerEventKind::external_report,
                                         &experiment::FingerprintTimerEvent::kind);
    if (found == plan.timer_run.events.end()) {
        invalid("M7.6 selected timer run has no external-report event");
    }
    return found->time;
}

[[nodiscard]] models::iot::NanodeviceProfile load_device(const LevelAPlan& plan,
                                                         const ArtifactRole role) {
    const auto& file = artifact(plan, role);
    return models::iot::load_nanodevice_profile({file.definition_path, file.schema_path});
}

} // namespace

LevelDCommunicationResult run_level_d_communication(const LevelAPlan& plan,
                                                    const LevelCAssemblyResult& assembly) {
    if (!assembly.complete || !assembly.completed_at.has_value()) {
        invalid("M7.6 communication requires a complete Level-C assembly");
    }

    const auto locator_profile = load_device(plan, ArtifactRole::locator_device);
    const auto collector_profile = load_device(plan, ArtifactRole::collector_device);
    const auto endpoint_profile = load_device(plan, ArtifactRole::gateway_endpoint);
    const auto& cluster_file = artifact(plan, ArtifactRole::communication_cluster);
    const auto cluster_profile = models::iot::load_cluster_communication_profile(
        {cluster_file.definition_path, cluster_file.schema_path});
    const auto& gateway_file = artifact(plan, ArtifactRole::active_gateway);
    const auto gateway_profile = models::iot::load_active_gateway_profile(
        {gateway_file.definition_path, gateway_file.schema_path});
    const auto& ban_file = artifact(plan, ArtifactRole::ban_station);
    const auto ban_profile =
        models::iot::load_ban_station_profile({ban_file.definition_path, ban_file.schema_path});

    models::iot::Nanodevice locator{locator_profile.device};
    models::iot::Nanodevice collector{collector_profile.device};
    models::iot::ActiveGateway gateway{gateway_profile.gateway, endpoint_profile.device,
                                       endpoint_profile.profile_id};
    models::iot::NanodeviceCluster cluster{cluster_profile.cluster};
    models::iot::BoundedMultiHopSession local_session{cluster};
    models::iot::ClusterDeviceMap devices{
        {std::string{locator.device_id()}, locator},
        {std::string{collector.device_id()}, collector},
        {std::string{gateway.gateway_id()}, gateway.endpoint()},
    };

    const auto collection_message_id = plan.profile.run.id + ":message:assembled";
    const models::iot::LocalMessageRequest collection_request{
        collection_message_id,
        models::iot::LocalMessageKind::fingerprint_tile,
        std::string{collector.device_id()},
        plan.profile.run.id,
        assembly.assembly_id,
        assembly.completed_at.value(),
        1s,
        1,
        256,
        "application/vnd.mehlissa.fingerprint+json;version=1.0.0",
        "{\"fingerprint_id\":\"" + plan.profile.target.fingerprint_id + "\",\"assembled\":true}",
    };
    const auto collection =
        local_session.exchange(devices, locator.device_id(), collector.device_id(),
                               collection_request, models::iot::ClusterRouteStrategy::fewest_hops);
    if (collection.status != models::iot::OneHopDeliveryStatus::delivered ||
        collector.received_messages().size() != 1) {
        invalid("M7.6 selected locator-to-collector path did not deliver");
    }
    static_cast<void>(collector.take_received_messages());

    const auto historical_return_departure = external_timer_time(plan) - 50ms;
    const auto uplink_created_at =
        std::max(collection.metrics.total_delivery_latency + assembly.completed_at.value(),
                 historical_return_departure);
    const auto uplink_message_id = plan.profile.run.id + ":message:collector-uplink";
    const models::iot::LocalMessageRequest uplink_request{
        uplink_message_id,
        models::iot::LocalMessageKind::measurement,
        std::string{gateway.gateway_id()},
        plan.profile.run.id,
        assembly.assembly_id,
        uplink_created_at,
        1s,
        1,
        256,
        "application/vnd.mehlissa.measurement+json;version=1.0.0",
        "{\"fingerprint_id\":\"" + plan.profile.target.fingerprint_id + "\",\"tissue\":\"" +
            plan.profile.target.tissue + "\",\"detected\":true}",
    };
    const auto uplink =
        local_session.exchange(devices, collector.device_id(), gateway.gateway_id(), uplink_request,
                               models::iot::ClusterRouteStrategy::fewest_hops);
    if (uplink.status != models::iot::OneHopDeliveryStatus::delivered) {
        invalid("M7.6 selected collector-to-gateway path did not deliver");
    }
    const auto gateway_time = uplink_created_at + uplink.metrics.total_delivery_latency;
    const auto measurement = gateway.publish_next_measurement(gateway_time);

    models::iot::GatewayBanAdapter gateway_adapter{ban_profile.gateway_adapter};
    models::iot::ExternalAnalysisControlStation station{ban_profile.station};
    models::iot::ScheduledBanTransportAdapter transport{ban_profile.uplink_transport};
    models::iot::BanCommunicationSession ban_session{transport};
    const auto frame = gateway_adapter.publish_measurement(measurement, 1s);
    const auto ban_result = ban_session.exchange(frame);
    if (ban_result.status != models::iot::OneHopDeliveryStatus::delivered) {
        invalid("M7.6 selected gateway-to-station BAN path did not deliver");
    }
    station.receive_measurement(frame, ban_result.completed_at);

    const auto add_energy = [](const core::Energy left, const core::Energy right) {
        return core::joules(core::in_joules(left) + core::in_joules(right));
    };
    return {collection_message_id,
            uplink_message_id,
            measurement.measurement_id,
            frame.frame_id,
            plan.profile.run.id + ":report:FP9",
            collection.route_device_ids,
            uplink.route_device_ids,
            assembly.completed_at.value() + collection.metrics.total_delivery_latency,
            gateway_time,
            gateway_time,
            ban_result.completed_at,
            collection.metrics.attempted_messages + uplink.metrics.attempted_messages,
            collection.metrics.delivered_messages + uplink.metrics.delivered_messages,
            ban_session.metrics().attempted_messages,
            ban_session.metrics().delivered_messages,
            add_energy(collection.metrics.transmitter_energy, uplink.metrics.transmitter_energy),
            add_energy(collection.metrics.receiver_energy, uplink.metrics.receiver_energy),
            add_energy(collection.metrics.link_energy, uplink.metrics.link_energy),
            ban_session.metrics().transmitter_energy,
            ban_session.metrics().receiver_energy,
            ban_session.metrics().link_energy,
            "Level D executes the selected deterministic locator-collector and "
            "collector-gateway links, active-gateway publication, BAN transport, and external "
            "station reception. All channel timing and energy values remain synthetic "
            "software-contract parameters."};
}

void apply_level_d_communication(LevelARuntimeResult& runtime,
                                 const LevelDCommunicationResult& communication) {
    const auto collection =
        std::ranges::find(runtime.stages, StageKind::local_collection, &StageTraceEntry::stage);
    const auto collector_return =
        std::ranges::find(runtime.stages, StageKind::collector_return, &StageTraceEntry::stage);
    const auto gateway =
        std::ranges::find(runtime.stages, StageKind::gateway_measurement, &StageTraceEntry::stage);
    const auto external =
        std::ranges::find(runtime.stages, StageKind::external_report, &StageTraceEntry::stage);
    if (collection == runtime.stages.end() || collector_return == runtime.stages.end() ||
        gateway == runtime.stages.end() || external == runtime.stages.end() ||
        communication.delivered_local_messages != 2 || communication.delivered_ban_frames != 1) {
        invalid("M7.6 cannot apply an incomplete communication result");
    }

    collection->time = communication.local_collection_completed_at;
    collection->basis = ExecutionBasis::software_surrogate;
    collection->component_id = "cluster.fp9.lung.level-a.1";
    collection->output_identity = communication.local_collection_message_id;
    collection->qualification = communication.qualification;

    collector_return->time = communication.collector_uplink_completed_at;
    collector_return->basis = ExecutionBasis::software_surrogate;
    collector_return->component_id = "cluster.fp9.lung.level-a.1";
    collector_return->input_identity = communication.local_collection_message_id;
    collector_return->output_identity = communication.collector_uplink_message_id;
    collector_return->qualification = communication.qualification;

    gateway->time = communication.gateway_measurement_at;
    gateway->basis = ExecutionBasis::software_surrogate;
    gateway->component_id = "gateway.synthetic.wrist";
    gateway->input_identity = communication.collector_uplink_message_id;
    gateway->output_identity = communication.gateway_measurement_id;
    gateway->qualification = communication.qualification;

    external->time = communication.external_report_at;
    external->basis = ExecutionBasis::software_surrogate;
    external->component_id = "station.synthetic.analysis-control";
    external->input_identity = communication.gateway_measurement_id;
    external->output_identity = communication.external_report_id;
    external->qualification = communication.qualification;

    constexpr std::array advanced_roles{
        ArtifactRole::locator_device,        ArtifactRole::collector_device,
        ArtifactRole::communication_cluster, ArtifactRole::gateway_endpoint,
        ArtifactRole::active_gateway,        ArtifactRole::ban_station};
    for (auto& component : runtime.components) {
        if (std::ranges::find(advanced_roles, component.role) != advanced_roles.end()) {
            component.advanced = true;
        }
    }

    if (!std::ranges::is_sorted(runtime.stages, {}, &StageTraceEntry::time)) {
        invalid("M7.6 communication result makes the stage trace acausal");
    }
}

} // namespace mehlissa::scenarios::fingerprinting

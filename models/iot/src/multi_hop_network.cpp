// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/iot/multi_hop_network.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <ranges>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::iot {
namespace {

[[noreturn]] void invalid(const core::ErrorCode code, const std::string_view message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] bool has_member(const NanodeviceClusterConfig& config,
                              const std::string_view device_id) {
    return std::ranges::any_of(
        config.members, [device_id](const auto& member) { return member.device_id == device_id; });
}

void add_count(std::uint64_t& target, const std::uint64_t value) {
    if (value > std::numeric_limits<std::uint64_t>::max() - target) {
        invalid(core::ErrorCode::numeric_overflow, "Multi-hop metric counter overflows");
    }
    target += value;
}

void add_duration(core::SimulationClock::Duration& target,
                  const core::SimulationClock::Duration value) {
    if (value > core::SimulationClock::Duration::max() - target) {
        invalid(core::ErrorCode::numeric_overflow, "Multi-hop latency metric overflows");
    }
    target += value;
}

void add_energy(core::Energy& target, const core::Energy value) {
    const auto sum = core::in_joules(target) + core::in_joules(value);
    if (!std::isfinite(sum)) {
        invalid(core::ErrorCode::numeric_overflow, "Multi-hop energy metric overflows");
    }
    target = core::joules(sum);
}

void merge_metrics(CommunicationMetrics& target, const CommunicationMetrics& source) {
    add_count(target.attempted_messages, source.attempted_messages);
    add_count(target.delivered_messages, source.delivered_messages);
    add_count(target.lost_messages, source.lost_messages);
    add_count(target.corrupted_messages, source.corrupted_messages);
    add_count(target.expired_messages, source.expired_messages);
    add_count(target.attempted_bytes, source.attempted_bytes);
    add_count(target.delivered_bytes, source.delivered_bytes);
    add_duration(target.total_delivery_latency, source.total_delivery_latency);
    target.maximum_delivery_latency =
        std::max(target.maximum_delivery_latency, source.maximum_delivery_latency);
    add_energy(target.transmitter_energy, source.transmitter_energy);
    add_energy(target.receiver_energy, source.receiver_energy);
    add_energy(target.link_energy, source.link_energy);
}

[[nodiscard]] bool better_route(const ClusterRoute& candidate, const ClusterRoute& current,
                                const ClusterRouteStrategy strategy) {
    if (strategy == ClusterRouteStrategy::fewest_hops) {
        if (candidate.link_indices.size() != current.link_indices.size()) {
            return candidate.link_indices.size() < current.link_indices.size();
        }
        if (candidate.total_configured_latency != current.total_configured_latency) {
            return candidate.total_configured_latency < current.total_configured_latency;
        }
    } else {
        if (candidate.total_configured_latency != current.total_configured_latency) {
            return candidate.total_configured_latency < current.total_configured_latency;
        }
        if (candidate.link_indices.size() != current.link_indices.size()) {
            return candidate.link_indices.size() < current.link_indices.size();
        }
    }
    return candidate.device_ids < current.device_ids;
}

void search_routes(const NanodeviceClusterConfig& config, const std::string_view target_device_id,
                   const ClusterRouteStrategy strategy, ClusterRoute& path,
                   std::unordered_set<std::string>& visited, std::optional<ClusterRoute>& best) {
    if (path.device_ids.back() == target_device_id) {
        if (!best.has_value() || better_route(path, best.value(), strategy)) {
            best = path;
        }
        return;
    }
    if (path.link_indices.size() >= config.maximum_hops) {
        return;
    }

    const auto current = path.device_ids.back();
    for (std::size_t index = 0; index < config.links.size(); ++index) {
        const auto& edge = config.links[index];
        if (edge.source_device_id != current || visited.contains(edge.target_device_id)) {
            continue;
        }
        if (edge.link.latency >
            core::SimulationClock::Duration::max() - path.total_configured_latency) {
            invalid(core::ErrorCode::numeric_overflow, "Cluster route latency overflows");
        }

        path.link_indices.push_back(index);
        path.device_ids.push_back(edge.target_device_id);
        path.total_configured_latency += edge.link.latency;
        visited.insert(edge.target_device_id);
        search_routes(config, target_device_id, strategy, path, visited, best);
        visited.erase(edge.target_device_id);
        path.total_configured_latency -= edge.link.latency;
        path.device_ids.pop_back();
        path.link_indices.pop_back();
    }
}

[[nodiscard]] Nanodevice& require_device(const ClusterDeviceMap& devices,
                                         const std::string& device_id) {
    const auto iterator = devices.find(device_id);
    if (iterator == devices.end() || iterator->second.get().device_id() != device_id) {
        invalid(core::ErrorCode::data_invalid,
                "Multi-hop exchange lacks a correctly keyed route device");
    }
    return iterator->second.get();
}

} // namespace

std::string_view to_string(const ClusterMemberRole role) noexcept {
    switch (role) {
    case ClusterMemberRole::endpoint:
        return "endpoint";
    case ClusterMemberRole::relay:
        return "relay";
    case ClusterMemberRole::collector:
        return "collector";
    case ClusterMemberRole::gateway:
        return "gateway";
    }
    return "unknown";
}

std::string_view to_string(const ClusterRouteStrategy strategy) noexcept {
    switch (strategy) {
    case ClusterRouteStrategy::fewest_hops:
        return "fewest_hops";
    case ClusterRouteStrategy::lowest_total_latency:
        return "lowest_total_latency";
    }
    return "unknown";
}

void validate_nanodevice_cluster_config(const NanodeviceClusterConfig& config) {
    if (config.cluster_id.empty() || config.maximum_hops == 0 || config.maximum_hops > 64 ||
        config.members.size() < 2 || config.links.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "A cluster requires identity, members, links, and a bounded hop count");
    }

    std::unordered_set<std::string> member_ids;
    for (const auto& member : config.members) {
        if (member.device_id.empty() || !member_ids.insert(member.device_id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Cluster members require unique non-empty device identities");
        }
    }

    std::unordered_set<std::string> link_ids;
    std::unordered_set<std::string> directed_pairs;
    for (const auto& edge : config.links) {
        validate_scheduled_one_hop_link_config(edge.link);
        const auto pair = edge.source_device_id + "\n" + edge.target_device_id;
        if (!has_member(config, edge.source_device_id) ||
            !has_member(config, edge.target_device_id) ||
            edge.source_device_id == edge.target_device_id ||
            !link_ids.insert(edge.link.link_id).second || !directed_pairs.insert(pair).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Cluster links require unique identities and valid directed member pairs");
        }
    }
}

NanodeviceCluster::NanodeviceCluster(NanodeviceClusterConfig config) : config_{std::move(config)} {
    validate_nanodevice_cluster_config(config_);
    links_.reserve(config_.links.size());
    for (const auto& edge : config_.links) {
        links_.push_back(std::make_unique<ScheduledOneHopLink>(edge.link));
    }
}

std::string_view NanodeviceCluster::cluster_id() const noexcept { return config_.cluster_id; }

const NanodeviceClusterConfig& NanodeviceCluster::config() const noexcept { return config_; }

ClusterRoute NanodeviceCluster::select_route(const std::string_view source_device_id,
                                             const std::string_view target_device_id,
                                             const ClusterRouteStrategy strategy) const {
    if (source_device_id == target_device_id || !has_member(config_, source_device_id) ||
        !has_member(config_, target_device_id)) {
        invalid(core::ErrorCode::data_invalid,
                "Cluster route requires two distinct known endpoint identities");
    }

    ClusterRoute path{config_.cluster_id, strategy, {std::string{source_device_id}}, {}, {}};
    std::unordered_set<std::string> visited{std::string{source_device_id}};
    std::optional<ClusterRoute> best;
    search_routes(config_, target_device_id, strategy, path, visited, best);
    if (!best.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "No loop-free cluster route satisfies the configured hop bound");
    }
    return std::move(best.value());
}

OneHopLinkModel& NanodeviceCluster::link(const std::size_t index) {
    if (index >= links_.size()) {
        invalid(core::ErrorCode::internal_failure, "Cluster link index is outside the topology");
    }
    return *links_[index];
}

BoundedMultiHopSession::BoundedMultiHopSession(NanodeviceCluster& cluster) noexcept
    : cluster_{cluster} {}

MultiHopExchangeResult BoundedMultiHopSession::exchange(const ClusterDeviceMap& devices,
                                                        const std::string_view source_device_id,
                                                        const std::string_view target_device_id,
                                                        const LocalMessageRequest& request,
                                                        const ClusterRouteStrategy strategy) {
    validate_local_message_request(request);
    if (request.target_device_id != target_device_id) {
        invalid(core::ErrorCode::data_invalid,
                "Multi-hop request target must equal the requested route target");
    }
    const auto route = cluster_.select_route(source_device_id, target_device_id, strategy);
    if (route.link_indices.size() > request.hop_limit) {
        invalid(core::ErrorCode::data_invalid,
                "Selected cluster route exceeds the message hop limit");
    }

    for (std::size_t index = 0; index < route.device_ids.size(); ++index) {
        auto& device = require_device(devices, route.device_ids[index]);
        if (index > 0 && index + 1 < route.device_ids.size() &&
            (!device.has_capability(NanodeviceCapability::relay) ||
             device.used_message_storage_bytes() != 0)) {
            invalid(core::ErrorCode::invariant_violated,
                    "Intermediate route devices must be idle relay-capable nanodevices");
        }
    }

    MultiHopExchangeResult result{cluster_.config().cluster_id,
                                  strategy,
                                  OneHopDeliveryStatus::dropped,
                                  OneHopDropReason::none,
                                  route.device_ids,
                                  {},
                                  {}};
    const auto absolute_expiry = request.created_at + request.valid_for;
    auto departed_at = request.created_at;

    for (std::size_t index = 0; index < route.link_indices.size(); ++index) {
        const auto remaining_validity = absolute_expiry - departed_at;
        auto hop_request = request;
        hop_request.message_id = index == 0
                                     ? request.message_id
                                     : request.message_id + ".relay." + std::to_string(index);
        hop_request.target_device_id = route.device_ids[index + 1];
        hop_request.created_at = departed_at;
        hop_request.valid_for = remaining_validity;
        hop_request.hop_limit = request.hop_limit - static_cast<std::uint32_t>(index);

        auto& source = require_device(devices, route.device_ids[index]);
        auto& target = require_device(devices, route.device_ids[index + 1]);
        OneHopCommunicationSession session{cluster_.link(route.link_indices[index])};
        auto hop = session.exchange(source, target, hop_request);
        merge_metrics(result.metrics, session.metrics());
        result.hops.push_back(std::move(hop));
        const auto& recorded = result.hops.back();
        if (recorded.transmission.status == OneHopDeliveryStatus::dropped) {
            result.drop_reason = recorded.transmission.drop_reason;
            return result;
        }

        departed_at = recorded.transmission.completed_at;
        if (index + 2 < route.device_ids.size()) {
            auto relayed = target.take_received_messages();
            if (relayed.size() != 1 || relayed.front() != recorded.message) {
                invalid(core::ErrorCode::invariant_violated,
                        "Relay receive buffer does not contain exactly the delivered hop message");
            }
        }
    }

    result.status = OneHopDeliveryStatus::delivered;
    return result;
}

} // namespace mehlissa::models::iot

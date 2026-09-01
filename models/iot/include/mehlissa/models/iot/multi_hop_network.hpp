// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_IOT_MULTI_HOP_NETWORK_HPP
#define MEHLISSA_MODELS_IOT_MULTI_HOP_NETWORK_HPP

#include <mehlissa/models/iot/one_hop_link.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mehlissa::models::iot {

enum class ClusterMemberRole : std::uint8_t { endpoint, relay, collector, gateway };
enum class ClusterRouteStrategy : std::uint8_t { fewest_hops, lowest_total_latency };

[[nodiscard]] std::string_view to_string(ClusterMemberRole role) noexcept;
[[nodiscard]] std::string_view to_string(ClusterRouteStrategy strategy) noexcept;

struct ClusterMemberConfig final {
    std::string device_id;
    ClusterMemberRole role{ClusterMemberRole::endpoint};
};

struct ClusterDirectedLinkConfig final {
    std::string source_device_id;
    std::string target_device_id;
    ScheduledOneHopLinkConfig link;
};

struct NanodeviceClusterConfig final {
    std::string cluster_id;
    std::uint32_t maximum_hops{};
    std::vector<ClusterMemberConfig> members;
    std::vector<ClusterDirectedLinkConfig> links;
};

void validate_nanodevice_cluster_config(const NanodeviceClusterConfig& config);

struct ClusterRoute final {
    std::string cluster_id;
    ClusterRouteStrategy strategy{ClusterRouteStrategy::fewest_hops};
    std::vector<std::string> device_ids;
    std::vector<std::size_t> link_indices;
    core::SimulationClock::Duration total_configured_latency{};
};

class NanodeviceCluster final {
  public:
    explicit NanodeviceCluster(NanodeviceClusterConfig config);

    [[nodiscard]] std::string_view cluster_id() const noexcept;
    [[nodiscard]] const NanodeviceClusterConfig& config() const noexcept;
    [[nodiscard]] ClusterRoute select_route(std::string_view source_device_id,
                                            std::string_view target_device_id,
                                            ClusterRouteStrategy strategy) const;
    [[nodiscard]] OneHopLinkModel& link(std::size_t index);

  private:
    NanodeviceClusterConfig config_;
    std::vector<std::unique_ptr<OneHopLinkModel>> links_;
};

using ClusterDeviceMap = std::unordered_map<std::string, std::reference_wrapper<Nanodevice>>;

struct MultiHopExchangeResult final {
    std::string cluster_id;
    ClusterRouteStrategy strategy{ClusterRouteStrategy::fewest_hops};
    OneHopDeliveryStatus status{OneHopDeliveryStatus::dropped};
    OneHopDropReason drop_reason{OneHopDropReason::none};
    std::vector<std::string> route_device_ids;
    std::vector<OneHopExchangeResult> hops;
    CommunicationMetrics metrics;
};

class BoundedMultiHopSession final {
  public:
    explicit BoundedMultiHopSession(NanodeviceCluster& cluster) noexcept;

    [[nodiscard]] MultiHopExchangeResult exchange(const ClusterDeviceMap& devices,
                                                  std::string_view source_device_id,
                                                  std::string_view target_device_id,
                                                  const LocalMessageRequest& request,
                                                  ClusterRouteStrategy strategy);

  private:
    NanodeviceCluster& cluster_;
};

} // namespace mehlissa::models::iot

#endif // MEHLISSA_MODELS_IOT_MULTI_HOP_NETWORK_HPP

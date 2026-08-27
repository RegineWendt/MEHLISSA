// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_CONSERVED_TRANSFER_HPP
#define MEHLISSA_MODELS_COUPLING_CONSERVED_TRANSFER_HPP

#include <mehlissa/core/quantity.hpp>
#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace mehlissa::models::coupling {

inline constexpr std::string_view conserved_transfer_contract_version = "1.0.0";

struct TransferHeader final {
    std::string contract_version;
    std::string transfer_id;
    std::string source_model_id;
    std::string source_port_id;
    std::string target_model_id;
    std::string target_port_id;
    core::SimulationClock::Duration emitted_at{};

    [[nodiscard]] bool operator==(const TransferHeader&) const noexcept = default;
};

struct PopulationTransfer final {
    TransferHeader header;
    std::string population_type;
    std::uint64_t count{};

    [[nodiscard]] bool operator==(const PopulationTransfer&) const noexcept = default;
};

struct SubstanceAmountTransfer final {
    TransferHeader header;
    std::string substance_id;
    core::Amount amount;

    [[nodiscard]] bool operator==(const SubstanceAmountTransfer&) const noexcept = default;
};

struct VolumeFlowTransfer final {
    TransferHeader header;
    core::FlowRate flow_rate;
    core::SimulationClock::Duration interval{};

    [[nodiscard]] bool operator==(const VolumeFlowTransfer&) const noexcept = default;
};

using ConservedTransfer =
    std::variant<PopulationTransfer, SubstanceAmountTransfer, VolumeFlowTransfer>;

void validate_transfer(const PopulationTransfer& transfer);
void validate_transfer(const SubstanceAmountTransfer& transfer);
void validate_transfer(const VolumeFlowTransfer& transfer);
void validate_transfer(const ConservedTransfer& transfer);
[[nodiscard]] const TransferHeader& transfer_header(const ConservedTransfer& transfer);
[[nodiscard]] TransferHeader& transfer_header(ConservedTransfer& transfer);
[[nodiscard]] core::Volume integrated_volume(const VolumeFlowTransfer& transfer);

class ConservationLedger final {
  public:
    void record_sent(const PopulationTransfer& transfer);
    void record_received(const PopulationTransfer& transfer);
    void record_sent(const SubstanceAmountTransfer& transfer);
    void record_received(const SubstanceAmountTransfer& transfer);
    void record_sent(const VolumeFlowTransfer& transfer);
    void record_received(const VolumeFlowTransfer& transfer);

    void verify_balanced() const;
    [[nodiscard]] std::size_t outstanding_transfer_count() const noexcept;

  private:
    template <typename Transfer>
    static void record(const Transfer& transfer, std::unordered_map<std::string, Transfer>& records,
                       std::string_view role);

    std::unordered_map<std::string, PopulationTransfer> populations_sent_;
    std::unordered_map<std::string, PopulationTransfer> populations_received_;
    std::unordered_map<std::string, SubstanceAmountTransfer> substances_sent_;
    std::unordered_map<std::string, SubstanceAmountTransfer> substances_received_;
    std::unordered_map<std::string, VolumeFlowTransfer> flows_sent_;
    std::unordered_map<std::string, VolumeFlowTransfer> flows_received_;
};

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_CONSERVED_TRANSFER_HPP

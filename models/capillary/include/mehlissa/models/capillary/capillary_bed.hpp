// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_HPP

#include <mehlissa/models/capillary/capillary_entity_disposition_profile.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>
#include <mehlissa/models/capillary/capillary_exchange.hpp>
#include <mehlissa/models/capillary/capillary_exchange_profile.hpp>
#include <mehlissa/models/capillary/capillary_recruitment_profile.hpp>
#include <mehlissa/models/coupling/extracellular_signal.hpp>
#include <mehlissa/models/coupling/model_component.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::capillary {

enum class CapillaryRegionKind : std::uint8_t { arteriole, capillary, venule };

[[nodiscard]] std::string_view to_string(CapillaryRegionKind kind) noexcept;

struct CapillaryRegion final {
    std::string id;
    CapillaryRegionKind kind{};
    core::Length length{};
    core::Length diameter{};
    std::uint64_t parallel_vessel_count{};
};

struct CapillaryRegionMetrics final {
    core::Area single_vessel_cross_section{};
    core::Area total_cross_section{};
    core::Volume blood_volume{};
    core::Speed mean_velocity{};
    core::SimulationClock::Duration transit_time{};
};

struct CapillaryBedConfig final {
    std::string component_name;
    std::string model_id;
    std::string entry_port_id;
    std::string exit_port_id;
    std::string return_target_model_id;
    std::string return_target_port_id;
    std::uint64_t total_parallel_path_count{};
    std::uint64_t perfused_path_count{};
    core::FlowRate volume_flow_rate{};
    std::vector<CapillaryRegion> regions;
};

struct CapillaryBedProfiles final {
    std::optional<CapillaryRecruitmentProfile> recruitment;
    std::optional<CapillaryExchangeProfile> exchange;
    std::optional<CapillaryEntityObservationProfile> entity_observation;
    std::optional<CapillaryEntityDispositionProfile> entity_disposition;
};

class CapillaryBed final : public coupling::ModelComponent,
                           public coupling::EntityDispositionSource,
                           public coupling::ExtracellularSignalSource {
  public:
    explicit CapillaryBed(CapillaryBedConfig config);
    CapillaryBed(CapillaryBedConfig config, CapillaryBedProfiles profiles);
    CapillaryBed(CapillaryBedConfig config, CapillaryRecruitmentProfile recruitment_profile);
    CapillaryBed(CapillaryBedConfig config, CapillaryExchangeProfile exchange_profile);
    CapillaryBed(CapillaryBedConfig config, CapillaryRecruitmentProfile recruitment_profile,
                 CapillaryExchangeProfile exchange_profile);

    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] bool accepts_entity_at(std::string_view port_id) const noexcept override;
    [[nodiscard]] bool emits_entity_at(std::string_view port_id) const noexcept override;

    void initialize(core::SimulationContext& context) override;
    void advance(core::SimulationContext& context, core::SimulationClock::Duration delta) override;
    void finalize(core::SimulationContext& context) noexcept override;

    void accept_entity(coupling::EntityTransfer transfer) override;
    [[nodiscard]] std::vector<coupling::EntityTransfer> take_outbound_entities() override;
    void accept_conserved_transfer(coupling::ConservedTransfer transfer) override;
    [[nodiscard]] std::vector<coupling::ConservedTransfer>
    take_outbound_conserved_transfers() override;
    [[nodiscard]] std::size_t resident_conserved_transfer_count() const noexcept override;

    [[nodiscard]] std::size_t region_count() const noexcept;
    [[nodiscard]] std::uint64_t total_parallel_path_count() const noexcept;
    [[nodiscard]] std::uint64_t perfused_path_count() const noexcept;
    [[nodiscard]] core::FlowRate volume_flow_rate() const noexcept;
    [[nodiscard]] bool has_recruitment_profile() const noexcept;
    [[nodiscard]] std::string_view recruitment_state_id() const noexcept;
    [[nodiscard]] std::size_t open_sphincter_group_count() const noexcept;
    [[nodiscard]] std::optional<CapillaryBoundaryCondition> boundary_condition() const noexcept;
    [[nodiscard]] bool has_exchange_profile() const noexcept;
    [[nodiscard]] std::string_view exchange_profile_id() const noexcept;
    [[nodiscard]] CapillaryTissueInventory tissue_inventory(std::string_view substance_id) const;
    [[nodiscard]] std::size_t exchange_record_count() const noexcept;
    [[nodiscard]] std::vector<CapillaryExchangeRecord> take_exchange_records();
    [[nodiscard]] bool has_entity_observation_profile() const noexcept;
    [[nodiscard]] std::string_view entity_observation_profile_id() const noexcept;
    [[nodiscard]] std::vector<CapillaryEntityPosition> entity_positions() const;
    [[nodiscard]] std::size_t entity_observation_record_count() const noexcept;
    [[nodiscard]] std::uint64_t dropped_entity_observation_record_count() const noexcept;
    [[nodiscard]] std::vector<CapillaryEntityObservationRecord> take_entity_observation_records();
    [[nodiscard]] bool has_entity_disposition_profile() const noexcept;
    [[nodiscard]] std::string_view entity_disposition_profile_id() const noexcept;
    [[nodiscard]] std::size_t pending_entity_disposition_count() const noexcept;
    [[nodiscard]] std::vector<coupling::EntityDispositionTransfer>
    take_outbound_entity_dispositions() override;
    [[nodiscard]] std::string_view signal_source_model_id() const noexcept override;
    [[nodiscard]] coupling::ExtracellularSignalSample observe_extracellular_signal(
        const coupling::ExtracellularSignalObservationRequest& request) const override;
    [[nodiscard]] const CapillaryRegionMetrics& region_metrics(CapillaryRegionKind kind) const;
    [[nodiscard]] std::size_t resident_entity_count() const noexcept;
    [[nodiscard]] std::size_t resident_entity_count_in(CapillaryRegionKind kind) const noexcept;

  private:
    enum class State : std::uint8_t { building, initialized, finalized };

    struct ResidentEntity final {
        coupling::EntityTransfer transfer;
        std::size_t region_index{};
        core::Length region_distance{};
        std::array<core::SimulationClock::Duration, capillary_observed_region_count>
            region_residence_times{};
    };

    struct ResidentConservedTransfer final {
        coupling::ConservedTransfer transfer;
        std::size_t region_index{};
        core::Length region_distance{};
        std::array<core::SimulationClock::Duration, capillary_observed_region_count>
            region_residence_times{};
    };

    struct AdvanceInterval final {
        core::SimulationClock::Duration delta{};
        core::SimulationClock::Duration emitted_at{};
    };

    void apply_recruitment_state(std::size_t state_index);
    void apply_substance_exchange(coupling::ConservedTransfer& transfer,
                                  core::SimulationClock::Duration reported_at);
    [[nodiscard]] std::optional<CapillaryEntityObservationRecord>
    record_entity_observation(const ResidentEntity& resident,
                              core::SimulationClock::Duration reported_at);
    [[nodiscard]] std::optional<coupling::EntityDispositionTransfer> sample_entity_disposition(
        const ResidentEntity& resident, const CapillaryEntityObservationRecord& observation,
        core::SimulationClock::Duration decided_at, core::RandomStream& random) const;
    void advance_residents(AdvanceInterval interval, core::RandomStream* disposition_random);

    CapillaryBedConfig config_;
    std::optional<CapillaryRecruitmentProfile> recruitment_profile_;
    std::optional<CapillaryExchangeProfile> exchange_profile_;
    std::optional<CapillaryEntityObservationProfile> entity_observation_profile_;
    std::optional<CapillaryEntityDispositionProfile> entity_disposition_profile_;
    std::vector<CapillaryRegionMetrics> region_metrics_;
    std::vector<ResidentEntity> resident_entities_;
    std::vector<coupling::EntityTransfer> outbound_entities_;
    std::vector<ResidentConservedTransfer> resident_conserved_transfers_;
    std::vector<coupling::ConservedTransfer> outbound_conserved_transfers_;
    std::vector<CapillaryExchangeRecord> exchange_records_;
    std::vector<CapillaryEntityObservationRecord> entity_observation_records_;
    std::vector<coupling::EntityDispositionTransfer> outbound_entity_dispositions_;
    std::unordered_map<std::string, CapillaryTissueInventory> tissue_inventories_;
    std::unordered_set<std::uint64_t> held_entity_ids_;
    std::unordered_set<std::string> held_transfer_ids_;
    std::uint64_t dropped_entity_observation_records_{};
    core::SimulationClock::Duration synchronization_time_{};
    core::SimulationClock::Duration recruitment_origin_time_{};
    core::FlowRate current_volume_flow_rate_{};
    std::uint64_t current_perfused_path_count_{};
    std::size_t current_recruitment_state_index_{};
    std::size_t next_recruitment_state_index_{};
    State state_{State::building};
};

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_BED_HPP

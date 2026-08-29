// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>
#include <mehlissa/models/capillary/capillary_exchange_profile.hpp>
#include <mehlissa/models/cosimulation/organ_capillary_coupler.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numbers>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::coupling::ConservedTransfer;
using mehlissa::models::coupling::EntityTransfer;
using mehlissa::models::coupling::PopulationTransfer;
using mehlissa::models::coupling::SubstanceAmountTransfer;
using mehlissa::models::coupling::TransferHeader;
using mehlissa::models::coupling::VolumeFlowTransfer;

constexpr double synthetic_continuity_flow_m3_s = std::numbers::pi * 1.0e-13;

class ScriptedOrgan final : public mehlissa::models::coupling::ModelComponent {
  public:
    [[nodiscard]] std::string_view name() const noexcept override { return "organ.synthetic"; }
    [[nodiscard]] std::string_view model_id() const noexcept override { return "organ.synthetic"; }

    [[nodiscard]] bool accepts_entity_at(const std::string_view port_id) const noexcept override {
        return port_id == "capillary-return";
    }

    [[nodiscard]] bool emits_entity_at(const std::string_view port_id) const noexcept override {
        return port_id == "capillary-departure";
    }

    void initialize(mehlissa::core::SimulationContext& context) override {
        if (state_ != State::building) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::lifecycle_invalid,
                                                "Scripted organ can only initialize once"};
        }
        synchronization_time_ = context.clock().now();
        state_ = State::initialized;
    }

    void advance(mehlissa::core::SimulationContext& context,
                 const mehlissa::core::SimulationClock::Duration delta) override {
        if (state_ != State::initialized || context.clock().now() != synchronization_time_) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::lifecycle_invalid,
                                                "Scripted organ is not synchronized"};
        }
        mehlissa::core::SimulationClock next{synchronization_time_};
        next.advance(delta);
        synchronization_time_ = next.now();
    }

    void finalize(mehlissa::core::SimulationContext&) noexcept override {
        state_ = State::finalized;
    }

    void accept_entity(EntityTransfer transfer) override {
        if (state_ != State::initialized) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::lifecycle_invalid,
                                                "Scripted organ is not initialized"};
        }
        mehlissa::models::coupling::validate_entity_transfer(transfer);
        if (transfer.target_model_id != model_id() || !accepts_entity_at(transfer.target_port_id) ||
            transfer.emitted_at != synchronization_time_ ||
            !returned_entity_ids_.insert(transfer.entity_id).second) {
            throw mehlissa::core::MehlissaError{
                mehlissa::core::ErrorCode::invariant_violated,
                "Scripted organ return does not match its active boundary"};
        }
        returned_entities_.push_back(std::move(transfer));
    }

    [[nodiscard]] std::vector<EntityTransfer> take_outbound_entities() override {
        auto result = std::move(outbound_entities_);
        outbound_entities_.clear();
        return result;
    }

    void accept_conserved_transfer(ConservedTransfer transfer) override {
        if (state_ != State::initialized) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::lifecycle_invalid,
                                                "Scripted organ is not initialized"};
        }
        mehlissa::models::coupling::validate_transfer(transfer);
        const auto& header = mehlissa::models::coupling::transfer_header(transfer);
        if (header.target_model_id != model_id() || !accepts_entity_at(header.target_port_id) ||
            header.emitted_at != synchronization_time_ ||
            !returned_transfer_ids_.insert(header.transfer_id).second) {
            throw mehlissa::core::MehlissaError{
                mehlissa::core::ErrorCode::invariant_violated,
                "Scripted organ conserved return does not match its active boundary"};
        }
        returned_conserved_transfers_.push_back(std::move(transfer));
    }

    [[nodiscard]] std::vector<ConservedTransfer> take_outbound_conserved_transfers() override {
        auto result = std::move(outbound_conserved_transfers_);
        outbound_conserved_transfers_.clear();
        return result;
    }

    [[nodiscard]] std::size_t resident_conserved_transfer_count() const noexcept override {
        return returned_conserved_transfers_.size();
    }

    void stage_entity(EntityTransfer transfer) {
        outbound_entities_.push_back(std::move(transfer));
    }

    void stage_conserved_transfer(ConservedTransfer transfer) {
        outbound_conserved_transfers_.push_back(std::move(transfer));
    }

    [[nodiscard]] const std::vector<EntityTransfer>& returned_entities() const noexcept {
        return returned_entities_;
    }

    [[nodiscard]] const std::vector<ConservedTransfer>&
    returned_conserved_transfers() const noexcept {
        return returned_conserved_transfers_;
    }

  private:
    enum class State : std::uint8_t { building, initialized, finalized };

    std::vector<EntityTransfer> outbound_entities_;
    std::vector<ConservedTransfer> outbound_conserved_transfers_;
    std::vector<EntityTransfer> returned_entities_;
    std::vector<ConservedTransfer> returned_conserved_transfers_;
    std::unordered_set<std::uint64_t> returned_entity_ids_;
    std::unordered_set<std::string> returned_transfer_ids_;
    mehlissa::core::SimulationClock::Duration synchronization_time_{};
    State state_{State::building};
};

[[nodiscard]] mehlissa::models::capillary::CapillaryBedConfig load_capillary_config() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_bed_definition(
               {
                   root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
                   root / "data/schemas/capillary-bed-definition/2.0.0.schema.json",
               })
        .model;
}

[[nodiscard]] mehlissa::models::capillary::CapillaryExchangeProfile load_exchange_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_exchange_profile({
        root / "examples/capillary-models/synthetic-oxygen-exchange-v1.json",
        root / "data/schemas/capillary-exchange-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::capillary::CapillaryEntityObservationProfile
load_entity_observation_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_entity_observation_profile({
        root / "examples/capillary-models/synthetic-nanodevice-observation-v1.json",
        root / "data/schemas/capillary-entity-observation-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] EntityTransfer organ_departure(const std::uint64_t entity_id,
                                             const std::chrono::nanoseconds emitted_at = 0ns) {
    return {
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        entity_id,
        "nanodevice",
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        emitted_at,
    };
}

[[nodiscard]] TransferHeader conserved_departure_header(const std::string& transfer_id) {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        transfer_id,
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        0s,
    };
}

[[nodiscard]] mehlissa::models::cosimulation::OrganCapillaryRoute route() {
    return {"capillary-departure", "arteriole-entry", "venule-exit", "capillary-return"};
}

} // namespace

TEST_CASE("An entity and conserved payloads complete an organ capillary organ round trip",
          "[m4][cosimulation][round-trip]") {
    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    auto organ = std::make_unique<ScriptedOrgan>();
    auto capillary =
        std::make_unique<mehlissa::models::capillary::CapillaryBed>(load_capillary_config());
    auto* organ_observer = organ.get();
    auto* capillary_observer = capillary.get();
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();

    organ_observer->stage_entity(organ_departure(42));
    organ_observer->stage_conserved_transfer(
        PopulationTransfer{conserved_departure_header("population-1"), "nanodevice", 10'000});
    organ_observer->stage_conserved_transfer(SubstanceAmountTransfer{
        conserved_departure_header("substance-1"), "oxygen", mehlissa::core::millimoles(2.5)});
    organ_observer->stage_conserved_transfer(VolumeFlowTransfer{
        conserved_departure_header("flow-1"),
        mehlissa::core::cubic_meters_per_second(synthetic_continuity_flow_m3_s), 1s});

    mehlissa::models::cosimulation::OrganCapillaryCoupler coupler{
        {*organ_observer, *capillary_observer}, route()};
    const mehlissa::models::cosimulation::CoupledTransferCounts expected_transfers{1, 3};
    CHECK(coupler.transfer_to_capillary(0ns) == expected_transfers);
    CHECK(coupler.outstanding_entity_count() == 1);
    CHECK(coupler.outstanding_conserved_transfer_count() == 3);
    CHECK(coupler.pending_departure_count() == 0);

    host.advance(500ms);
    CHECK(coupler.transfer_to_organ(500ms) ==
          mehlissa::models::cosimulation::CoupledTransferCounts{});
    host.advance(500ms);
    CHECK(coupler.transfer_to_organ(1s) == expected_transfers);

    REQUIRE(organ_observer->returned_entities().size() == 1);
    const auto& entity = organ_observer->returned_entities().front();
    CHECK(entity.entity_id == 42);
    CHECK(entity.entity_type == "nanodevice");
    CHECK(entity.source_model_id == "capillary.synthetic.reference.v2");
    CHECK(entity.source_port_id == "venule-exit");
    CHECK(entity.target_model_id == "organ.synthetic");
    CHECK(entity.target_port_id == "capillary-return");
    CHECK(entity.emitted_at == 1s);

    const auto& returned = organ_observer->returned_conserved_transfers();
    REQUIRE(returned.size() == 3);
    CHECK(std::get<PopulationTransfer>(returned[0]).count == 10'000);
    CHECK(mehlissa::core::in_moles(std::get<SubstanceAmountTransfer>(returned[1]).amount) ==
          0.0025);
    const auto& flow = std::get<VolumeFlowTransfer>(returned[2]);
    CHECK(mehlissa::core::in_cubic_meters_per_second(flow.flow_rate) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));
    CHECK(mehlissa::core::in_cubic_meters(mehlissa::models::coupling::integrated_volume(flow)) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));

    CHECK(coupler.outstanding_entity_count() == 0);
    CHECK(coupler.outstanding_conserved_transfer_count() == 0);
    CHECK(coupler.pending_return_count() == 0);
    CHECK(coupler.completed_entity_round_trip_count() == 1);
    CHECK(coupler.completed_conserved_round_trip_count() == 3);
}

TEST_CASE("The organ capillary round trip is stable across compatible host steps",
          "[m4][cosimulation][determinism]") {
    const auto step = GENERATE(100ms, 250ms, 500ms);
    mehlissa::core::ComponentHost host{std::uint64_t{17}};
    auto organ = std::make_unique<ScriptedOrgan>();
    auto capillary =
        std::make_unique<mehlissa::models::capillary::CapillaryBed>(load_capillary_config());
    auto* organ_observer = organ.get();
    auto* capillary_observer = capillary.get();
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();

    organ_observer->stage_entity(organ_departure(7));
    mehlissa::models::cosimulation::OrganCapillaryCoupler coupler{
        {*organ_observer, *capillary_observer}, route()};
    REQUIRE(coupler.transfer_to_capillary(0ns).entities == 1);

    std::size_t returned{};
    for (auto elapsed = 0ns; elapsed < 1s; elapsed += step) {
        host.advance(step);
        returned += coupler.transfer_to_organ(host.context().clock().now()).entities;
    }
    CHECK(returned == 1);
    REQUIRE(organ_observer->returned_entities().size() == 1);
    const EntityTransfer expected_return{
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        7,
        "nanodevice",
        "capillary.synthetic.reference.v2",
        "venule-exit",
        "organ.synthetic",
        "capillary-return",
        1s};
    CHECK(organ_observer->returned_entities().front() == expected_return);
}

TEST_CASE("Balanced substance exchange completes the organ capillary organ route",
          "[m4][cosimulation][exchange][conservation]") {
    mehlissa::core::ComponentHost host{std::uint64_t{23}};
    auto organ = std::make_unique<ScriptedOrgan>();
    auto capillary = std::make_unique<mehlissa::models::capillary::CapillaryBed>(
        load_capillary_config(), load_exchange_profile());
    auto* organ_observer = organ.get();
    auto* capillary_observer = capillary.get();
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();

    organ_observer->stage_conserved_transfer(
        SubstanceAmountTransfer{conserved_departure_header("oxygen-exchange-1"), "oxygen",
                                mehlissa::core::millimoles(2.5)});
    mehlissa::models::cosimulation::OrganCapillaryCoupler coupler{
        {*organ_observer, *capillary_observer}, route()};
    REQUIRE(coupler.transfer_to_capillary(0ns).conserved_transfers == 1);

    host.advance(1s);
    REQUIRE(coupler.transfer_to_organ(1s).conserved_transfers == 1);
    REQUIRE(organ_observer->returned_conserved_transfers().size() == 1);
    const auto& returned =
        std::get<SubstanceAmountTransfer>(organ_observer->returned_conserved_transfers().front());
    CHECK(returned.header.transfer_id == "oxygen-exchange-1");
    CHECK(mehlissa::core::in_moles(returned.amount) == Catch::Approx(0.0015));

    const auto records = capillary_observer->take_exchange_records();
    REQUIRE(records.size() == 1);
    CHECK(mehlissa::models::capillary::is_balanced(records.front()));
    CHECK(coupler.outstanding_conserved_transfer_count() == 0);
    CHECK(coupler.completed_conserved_round_trip_count() == 1);
}

TEST_CASE("Entity observations preserve the complete organ capillary ownership route",
          "[m4][cosimulation][entity-observation][ownership]") {
    mehlissa::models::capillary::CapillaryBedProfiles profiles;
    profiles.entity_observation = load_entity_observation_profile();
    mehlissa::core::ComponentHost host{std::uint64_t{24}};
    auto organ = std::make_unique<ScriptedOrgan>();
    auto capillary = std::make_unique<mehlissa::models::capillary::CapillaryBed>(
        load_capillary_config(), std::move(profiles));
    auto* organ_observer = organ.get();
    auto* capillary_observer = capillary.get();
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();

    organ_observer->stage_entity(organ_departure(45));
    mehlissa::models::cosimulation::OrganCapillaryCoupler coupler{
        {*organ_observer, *capillary_observer}, route()};
    REQUIRE(coupler.transfer_to_capillary(0ns).entities == 1);

    host.advance(1s);
    REQUIRE(coupler.transfer_to_organ(1s).entities == 1);
    REQUIRE(organ_observer->returned_entities().size() == 1);
    CHECK(organ_observer->returned_entities().front().entity_id == 45);
    CHECK(coupler.outstanding_entity_count() == 0);
    CHECK(coupler.completed_entity_round_trip_count() == 1);

    const auto records = capillary_observer->take_entity_observation_records();
    REQUIRE(records.size() == 1);
    CHECK(records.front().entity_id == 45);
    CHECK(mehlissa::models::capillary::has_normalized_outcome_likelihoods(records.front()));
}

TEST_CASE("The coupler retains transfers rejected at either synchronization boundary",
          "[m4][cosimulation][validation]") {
    mehlissa::core::ComponentHost host{std::uint64_t{1}};
    auto organ = std::make_unique<ScriptedOrgan>();
    auto capillary =
        std::make_unique<mehlissa::models::capillary::CapillaryBed>(load_capillary_config());
    auto* organ_observer = organ.get();
    auto* capillary_observer = capillary.get();
    host.add(std::move(organ));
    host.add(std::move(capillary));
    host.initialize();

    mehlissa::models::cosimulation::OrganCapillaryCoupler coupler{
        {*organ_observer, *capillary_observer}, route()};
    auto wrong_route = organ_departure(1);
    wrong_route.target_port_id = "wrong-entry";
    organ_observer->stage_entity(std::move(wrong_route));
    CHECK_THROWS_AS(coupler.transfer_to_capillary(0ns), mehlissa::core::MehlissaError);
    CHECK(coupler.pending_departure_count() == 1);
    CHECK(coupler.outstanding_entity_count() == 0);

    mehlissa::core::ComponentHost retry_host{std::uint64_t{2}};
    auto retry_organ = std::make_unique<ScriptedOrgan>();
    auto retry_capillary =
        std::make_unique<mehlissa::models::capillary::CapillaryBed>(load_capillary_config());
    auto* retry_organ_observer = retry_organ.get();
    auto* retry_capillary_observer = retry_capillary.get();
    retry_host.add(std::move(retry_organ));
    retry_host.add(std::move(retry_capillary));
    retry_host.initialize();
    retry_organ_observer->stage_entity(organ_departure(2));
    mehlissa::models::cosimulation::OrganCapillaryCoupler retry_coupler{
        {*retry_organ_observer, *retry_capillary_observer}, route()};
    REQUIRE(retry_coupler.transfer_to_capillary(0ns).entities == 1);
    retry_host.advance(1s);

    CHECK_THROWS_AS(retry_coupler.transfer_to_organ(900ms), mehlissa::core::MehlissaError);
    CHECK(retry_coupler.pending_return_count() == 1);
    CHECK(retry_coupler.outstanding_entity_count() == 1);
    CHECK(retry_organ_observer->returned_entities().empty());
    CHECK(retry_coupler.transfer_to_organ(1s).entities == 1);
    CHECK(retry_coupler.pending_return_count() == 0);
    CHECK(retry_coupler.outstanding_entity_count() == 0);
}

TEST_CASE("The coupler rejects incompatible endpoint ports", "[m4][cosimulation][validation]") {
    ScriptedOrgan organ;
    mehlissa::models::capillary::CapillaryBed capillary{load_capillary_config()};
    auto invalid_route = route();
    invalid_route.organ_return_port_id = "wrong-return";
    CHECK_THROWS_AS(mehlissa::models::cosimulation::OrganCapillaryCoupler(
                        mehlissa::models::cosimulation::OrganCapillaryEndpoints{organ, capillary},
                        std::move(invalid_route)),
                    mehlissa::core::MehlissaError);
}

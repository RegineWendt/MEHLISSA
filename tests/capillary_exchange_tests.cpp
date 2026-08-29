// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/capillary_exchange.hpp>
#include <mehlissa/models/capillary/capillary_exchange_profile.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numbers>
#include <string>
#include <utility>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::capillary::CapillaryBed;
using mehlissa::models::capillary::CapillaryExchangeProfile;
using mehlissa::models::coupling::PopulationTransfer;
using mehlissa::models::coupling::SubstanceAmountTransfer;
using mehlissa::models::coupling::TransferHeader;
using mehlissa::models::coupling::VolumeFlowTransfer;

constexpr double synthetic_continuity_flow_m3_s = std::numbers::pi * 1.0e-13;

[[nodiscard]] mehlissa::models::capillary::CapillaryBedConfig load_capillary_config() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_bed_definition(
               {
                   root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
                   root / "data/schemas/capillary-bed-definition/2.0.0.schema.json",
               })
        .model;
}

[[nodiscard]] CapillaryExchangeProfile load_exchange_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_exchange_profile({
        root / "examples/capillary-models/synthetic-oxygen-exchange-v1.json",
        root / "data/schemas/capillary-exchange-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] TransferHeader transfer_header(const std::string& transfer_id,
                                             const std::chrono::nanoseconds emitted_at = 0ns) {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        transfer_id,
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        emitted_at,
    };
}

} // namespace

TEST_CASE("A strict capillary-exchange profile loads with scoped evidence",
          "[m4][capillary][exchange][schema]") {
    const auto profile = load_exchange_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-oxygen-exchange-v1");
    CHECK(profile.compatible_model_id == "capillary.synthetic.reference.v2");
    CHECK(profile.unmatched_substance_policy == "pass_through");
    REQUIRE(profile.substance_rules.size() == 1);
    CHECK(profile.substance_rules.front().substance_id == "oxygen");
    CHECK(profile.substance_rules.front().blood_to_endothelium_fraction == 0.4);
    CHECK(profile.sources.size() == 2);
    CHECK(profile.limitations.size() == 4);
}

TEST_CASE("Capillary exchange balances blood endothelium interstitium and cell",
          "[m4][capillary][exchange][conservation]") {
    mehlissa::core::ComponentHost host{std::uint64_t{51}};
    auto component =
        std::make_unique<CapillaryBed>(load_capillary_config(), load_exchange_profile());
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    CHECK(capillary->has_exchange_profile());
    CHECK(capillary->exchange_profile_id() == "synthetic-oxygen-exchange-v1");
    capillary->accept_conserved_transfer(SubstanceAmountTransfer{
        transfer_header("oxygen-1"), "oxygen", mehlissa::core::millimoles(2.5)});
    capillary->accept_conserved_transfer(SubstanceAmountTransfer{
        transfer_header("glucose-1"), "glucose", mehlissa::core::millimoles(1.0)});
    capillary->accept_conserved_transfer(
        PopulationTransfer{transfer_header("population-1"), "nanodevice", 100});
    capillary->accept_conserved_transfer(VolumeFlowTransfer{
        transfer_header("flow-1"),
        mehlissa::core::cubic_meters_per_second(synthetic_continuity_flow_m3_s), 1s});

    host.advance(1s);
    const auto outbound = capillary->take_outbound_conserved_transfers();
    REQUIRE(outbound.size() == 4);
    const auto& oxygen = std::get<SubstanceAmountTransfer>(outbound[0]);
    CHECK(mehlissa::core::in_moles(oxygen.amount) == Catch::Approx(0.0015));
    const auto& glucose = std::get<SubstanceAmountTransfer>(outbound[1]);
    CHECK(mehlissa::core::in_moles(glucose.amount) == Catch::Approx(0.001));
    CHECK(std::get<PopulationTransfer>(outbound[2]).count == 100);
    CHECK(mehlissa::core::in_cubic_meters_per_second(
              std::get<VolumeFlowTransfer>(outbound[3]).flow_rate) ==
          Catch::Approx(synthetic_continuity_flow_m3_s));

    CHECK(capillary->exchange_record_count() == 1);
    const auto records = capillary->take_exchange_records();
    REQUIRE(records.size() == 1);
    const auto& record = records.front();
    CHECK(record.transfer_id == "oxygen-1");
    CHECK(record.substance_id == "oxygen");
    CHECK(record.profile_id == "synthetic-oxygen-exchange-v1");
    CHECK(record.reported_at == 1s);
    CHECK(mehlissa::core::in_moles(record.incoming_blood_amount) == Catch::Approx(0.0025));
    CHECK(mehlissa::core::in_moles(record.outgoing_blood_amount) == Catch::Approx(0.0015));
    CHECK(mehlissa::core::in_moles(record.endothelium_amount) == Catch::Approx(0.0005));
    CHECK(mehlissa::core::in_moles(record.interstitium_amount) == Catch::Approx(0.000375));
    CHECK(mehlissa::core::in_moles(record.cell_amount) == Catch::Approx(0.000125));
    CHECK(mehlissa::models::capillary::is_balanced(record));
    CHECK(mehlissa::models::capillary::exchange_balance_error_moles(record) ==
          Catch::Approx(0.0).margin(1.0e-18));

    const auto inventory = capillary->tissue_inventory("oxygen");
    CHECK(mehlissa::core::in_moles(inventory.endothelium_amount) == Catch::Approx(0.0005));
    CHECK(mehlissa::core::in_moles(inventory.interstitium_amount) == Catch::Approx(0.000375));
    CHECK(mehlissa::core::in_moles(inventory.cell_amount) == Catch::Approx(0.000125));
    const auto unmatched_inventory = capillary->tissue_inventory("glucose");
    CHECK(mehlissa::core::in_moles(unmatched_inventory.endothelium_amount) == 0.0);
}

TEST_CASE("Repeated exchange accumulates tissue inventory without reprocessing records",
          "[m4][capillary][exchange][inventory]") {
    mehlissa::core::ComponentHost host{std::uint64_t{52}};
    auto component =
        std::make_unique<CapillaryBed>(load_capillary_config(), load_exchange_profile());
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();

    capillary->accept_conserved_transfer(SubstanceAmountTransfer{
        transfer_header("oxygen-1"), "oxygen", mehlissa::core::millimoles(2.5)});
    host.advance(1s);
    REQUIRE(capillary->take_exchange_records().size() == 1);
    static_cast<void>(capillary->take_outbound_conserved_transfers());

    capillary->accept_conserved_transfer(SubstanceAmountTransfer{
        transfer_header("oxygen-2", 1s), "oxygen", mehlissa::core::millimoles(2.5)});
    host.advance(1s);
    REQUIRE(capillary->take_exchange_records().size() == 1);

    const auto inventory = capillary->tissue_inventory("oxygen");
    CHECK(mehlissa::core::in_moles(inventory.endothelium_amount) == Catch::Approx(0.001));
    CHECK(mehlissa::core::in_moles(inventory.interstitium_amount) == Catch::Approx(0.00075));
    CHECK(mehlissa::core::in_moles(inventory.cell_amount) == Catch::Approx(0.00025));
}

TEST_CASE("Capillary exchange rejects incompatible or non-conservative profiles",
          "[m4][capillary][exchange][validation]") {
    auto wrong_model = load_exchange_profile();
    wrong_model.compatible_model_id = "capillary.other";
    CHECK_THROWS_AS(CapillaryBed(load_capillary_config(), std::move(wrong_model)),
                    mehlissa::core::MehlissaError);

    auto duplicate_substance = load_exchange_profile();
    duplicate_substance.substance_rules.push_back(duplicate_substance.substance_rules.front());
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_exchange_profile(duplicate_substance),
        mehlissa::core::MehlissaError);

    auto no_blood_return = load_exchange_profile();
    no_blood_return.substance_rules.front().blood_to_endothelium_fraction = 1.0;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_exchange_profile(no_blood_return),
        mehlissa::core::MehlissaError);

    auto invalid_fraction = load_exchange_profile();
    invalid_fraction.substance_rules.front().interstitium_to_cell_fraction = -0.1;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_exchange_profile(invalid_fraction),
        mehlissa::core::MehlissaError);
}

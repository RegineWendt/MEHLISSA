// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/capillary_exchange_profile.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cosimulation/capillary_cell_signal_profile.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>
#include <mehlissa/models/coupling/extracellular_signal.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] mehlissa::models::capillary::CapillaryBedConfig load_capillary_config() {
    const auto root = project_root();
    return mehlissa::models::capillary::load_capillary_bed_definition(
               {root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
                root / "data/schemas/capillary-bed-definition/2.0.0.schema.json"})
        .model;
}

[[nodiscard]] mehlissa::models::capillary::CapillaryExchangeProfile load_exchange_profile() {
    const auto root = project_root();
    return mehlissa::models::capillary::load_capillary_exchange_profile({
        root / "examples/capillary-models/synthetic-oxygen-exchange-v1.json",
        root / "data/schemas/capillary-exchange-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::cell::ReceptorLigandProfile load_cell_profile() {
    const auto root = project_root();
    return mehlissa::models::cell::load_receptor_ligand_profile({
        root / "examples/cell-models/synthetic-receptor-ligand-v1.json",
        root / "data/schemas/receptor-ligand-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::cosimulation::CapillaryCellSignalProfile load_signal_profile() {
    const auto root = project_root();
    return mehlissa::models::cosimulation::load_capillary_cell_signal_profile({
        root / "examples/cosimulation/synthetic-capillary-cell-signal-v1.json",
        root / "data/schemas/capillary-cell-signal-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::coupling::TransferHeader transfer_header() {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        "m5-signal-source-transfer",
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        0ns,
    };
}

struct PreparedReference final {
    std::unique_ptr<mehlissa::core::ComponentHost> host;
    mehlissa::models::capillary::CapillaryBed* capillary{};
};

[[nodiscard]] PreparedReference prepare_reference() {
    auto host = std::make_unique<mehlissa::core::ComponentHost>(std::uint64_t{5202});
    auto component = std::make_unique<mehlissa::models::capillary::CapillaryBed>(
        load_capillary_config(), load_exchange_profile());
    auto* capillary = component.get();
    host->add(std::move(component));
    host->initialize();
    capillary->accept_conserved_transfer(mehlissa::models::coupling::SubstanceAmountTransfer{
        transfer_header(), "oxygen", mehlissa::core::moles(2.0e-18)});
    host->advance(1s);
    return {std::move(host), capillary};
}

} // namespace

TEST_CASE("A strict capillary-cell signal profile maps one evidence-scoped boundary",
          "[m5][cosimulation][cell-signal][schema]") {
    const auto profile = load_signal_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-capillary-cell-signal-v1");
    CHECK(profile.coupler.source_model_id == "capillary.synthetic.reference.v2");
    CHECK(profile.coupler.source_compartment_id == "interstitium");
    CHECK(profile.coupler.signal_id == "oxygen");
    CHECK(profile.coupler.target_cell_model_id == "cell.receptor-ligand.synthetic.v1");
    CHECK(profile.coupler.ligand_id == "synthetic-ligand");
    CHECK(profile.validity.evidence_class == "software_test_surrogate");
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 6);
}

TEST_CASE("An M4 tissue signal triggers the analytical M5 receptor response without consumption",
          "[m5][cosimulation][cell-signal][handoff][conservation]") {
    auto prepared = prepare_reference();
    const auto profile = load_signal_profile();
    const auto cell_profile = load_cell_profile();
    const auto cell_model = mehlissa::models::cell::make_receptor_ligand_model(cell_profile);
    auto coupler = mehlissa::models::cosimulation::make_capillary_cell_signal_coupler(profile);

    const auto before = prepared.capillary->tissue_inventory("oxygen");
    REQUIRE(mehlissa::core::in_moles(before.interstitium_amount) == Catch::Approx(3.0e-19));
    const auto evaluation =
        coupler.evaluate(*prepared.capillary, *cell_model, profile.reference_case.sample_id,
                         profile.reference_case.observed_at);
    const auto after = prepared.capillary->tissue_inventory("oxygen");

    CHECK(evaluation.profile_id == profile.profile_id);
    CHECK(evaluation.source_sample.source_model_id == profile.coupler.source_model_id);
    CHECK(evaluation.source_sample.source_compartment_id == "interstitium");
    CHECK(evaluation.source_sample.sampling_semantics ==
          mehlissa::models::coupling::non_consuming_uniform_inventory_snapshot);
    CHECK(mehlissa::core::in_moles(evaluation.source_sample.represented_amount) ==
          Catch::Approx(mehlissa::core::in_moles(profile.reference_case.expected_source_amount)));
    CHECK(mehlissa::core::in_moles_per_cubic_meter(
              mehlissa::models::coupling::extracellular_signal_concentration(
                  evaluation.source_sample)) ==
          Catch::Approx(mehlissa::core::in_moles_per_cubic_meter(
              profile.reference_case.expected_ligand_concentration)));
    CHECK(mehlissa::models::coupling::extracellular_signal_valid_until(evaluation.source_sample) ==
          11s);
    CHECK(evaluation.cell_response.final_bound_fraction ==
          Catch::Approx(profile.reference_case.expected_final_bound_fraction)
              .margin(profile.reference_case.absolute_tolerance));
    REQUIRE(evaluation.cell_response.detection_threshold_reached);
    const auto crossing = evaluation.cell_response.first_threshold_crossing_time.value_or(
        mehlissa::core::SimulationClock::Duration::min());
    CHECK(std::chrono::duration<double>{crossing}.count() ==
          Catch::Approx(profile.reference_case.expected_threshold_crossing_seconds).margin(1.0e-9));
    CHECK(mehlissa::core::in_moles(after.endothelium_amount) ==
          Catch::Approx(mehlissa::core::in_moles(before.endothelium_amount)));
    CHECK(mehlissa::core::in_moles(after.interstitium_amount) ==
          Catch::Approx(mehlissa::core::in_moles(before.interstitium_amount)));
    CHECK(mehlissa::core::in_moles(after.cell_amount) ==
          Catch::Approx(mehlissa::core::in_moles(before.cell_amount)));
    CHECK(coupler.completed_sample_count() == 1);
    CHECK_THROWS_AS(coupler.evaluate(*prepared.capillary, *cell_model,
                                     profile.reference_case.sample_id,
                                     profile.reference_case.observed_at),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("Capillary-cell hand-off rejects stale time incompatible mapping and invalid samples",
          "[m5][cosimulation][cell-signal][validation]") {
    auto prepared = prepare_reference();
    const auto cell_model = mehlissa::models::cell::make_receptor_ligand_model(load_cell_profile());

    auto wrong_source_profile = load_signal_profile();
    wrong_source_profile.coupler.source_model_id = "capillary.other";
    auto wrong_source =
        mehlissa::models::cosimulation::make_capillary_cell_signal_coupler(wrong_source_profile);
    CHECK_THROWS_AS(wrong_source.evaluate(*prepared.capillary, *cell_model, "wrong-source", 1s),
                    mehlissa::core::MehlissaError);

    auto coupler =
        mehlissa::models::cosimulation::make_capillary_cell_signal_coupler(load_signal_profile());
    CHECK_THROWS_AS(coupler.evaluate(*prepared.capillary, *cell_model, "retryable-sample", 0s),
                    mehlissa::core::MehlissaError);
    CHECK(coupler.completed_sample_count() == 0);
    CHECK_NOTHROW(coupler.evaluate(*prepared.capillary, *cell_model, "retryable-sample", 1s));

    mehlissa::models::coupling::ExtracellularSignalObservationRequest intracellular{
        std::string{mehlissa::models::coupling::extracellular_signal_contract_version},
        "invalid-cell-inventory",
        "oxygen",
        "cell",
        mehlissa::core::cubic_meters(1.0e-15),
        1s,
        10s,
    };
    CHECK_THROWS_AS(prepared.capillary->observe_extracellular_signal(intracellular),
                    mehlissa::core::MehlissaError);

    auto invalid_profile = load_signal_profile();
    invalid_profile.coupler.represented_volume = mehlissa::core::cubic_meters(0.0);
    CHECK_THROWS_AS(
        mehlissa::models::cosimulation::validate_capillary_cell_signal_profile(invalid_profile),
        mehlissa::core::MehlissaError);

    auto duplicate_source = load_signal_profile();
    duplicate_source.sources.push_back(duplicate_source.sources.front());
    CHECK_THROWS_AS(
        mehlissa::models::cosimulation::validate_capillary_cell_signal_profile(duplicate_source),
        mehlissa::core::MehlissaError);
}

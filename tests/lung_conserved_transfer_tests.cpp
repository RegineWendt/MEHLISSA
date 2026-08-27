// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>
#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::coupling::ConservedTransfer;
using mehlissa::models::coupling::PopulationTransfer;
using mehlissa::models::coupling::SubstanceAmountTransfer;
using mehlissa::models::coupling::TransferHeader;
using mehlissa::models::coupling::VolumeFlowTransfer;
using mehlissa::models::organ::LungModelVariant;

[[nodiscard]] mehlissa::models::organ::LungModelConfig lung_config(const LungModelVariant variant) {
    return {
        variant,
        "organ.lung",
        variant == LungModelVariant::effective_compartment ? "lung.compartment.v1"
                                                           : "lung.regional.v1",
        "arterial-entry",
        "venous-exit",
        "body",
        "venous-return",
        variant == LungModelVariant::effective_compartment ? 2s : 0s,
        variant == LungModelVariant::regional_circulation
            ? std::vector<mehlissa::models::organ::PulmonaryTransitRegion>{{"artery", 500ms},
                                                                           {"capillary-surrogate",
                                                                            1s},
                                                                           {"vein", 500ms}}
            : std::vector<mehlissa::models::organ::PulmonaryTransitRegion>{},
    };
}

[[nodiscard]] TransferHeader input_header(const std::string& transfer_id,
                                          const std::string& lung_model_id) {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        transfer_id,
        "body",
        "arterial-departure",
        lung_model_id,
        "arterial-entry",
        0s,
    };
}

void check_return_header(const ConservedTransfer& transfer, const std::string& lung_model_id) {
    const auto& header = mehlissa::models::coupling::transfer_header(transfer);
    CHECK(header.source_model_id == lung_model_id);
    CHECK(header.source_port_id == "venous-exit");
    CHECK(header.target_model_id == "body");
    CHECK(header.target_port_id == "venous-return");
    CHECK(header.emitted_at == 2s);
}

} // namespace

TEST_CASE("Both lung variants conserve population substance and volume flow",
          "[m3][organ][conservation]") {
    const auto variant =
        GENERATE(LungModelVariant::effective_compartment, LungModelVariant::regional_circulation);
    auto lung = mehlissa::models::organ::make_lung_model(lung_config(variant));
    auto* observer = lung.get();
    const std::string lung_model_id{observer->model_id()};

    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    host.add(std::move(lung));
    host.initialize();

    observer->accept_conserved_transfer(
        PopulationTransfer{input_header("population-1", lung_model_id), "nanodevice", 10'000});
    observer->accept_conserved_transfer(SubstanceAmountTransfer{
        input_header("substance-1", lung_model_id), "oxygen", mehlissa::core::millimoles(2.5)});
    observer->accept_conserved_transfer(
        VolumeFlowTransfer{input_header("flow-1", lung_model_id),
                           mehlissa::core::cubic_meters_per_second(0.0001), 2s});

    CHECK(observer->resident_conserved_transfer_count() == 3);
    host.advance(1s);
    CHECK(observer->take_outbound_conserved_transfers().empty());
    CHECK(observer->resident_conserved_transfer_count() == 3);
    host.advance(1s);

    const auto returned = observer->take_outbound_conserved_transfers();
    REQUIRE(returned.size() == 3);
    CHECK(observer->resident_conserved_transfer_count() == 0);
    for (const auto& transfer : returned) {
        check_return_header(transfer, lung_model_id);
    }

    const auto& population = std::get<PopulationTransfer>(returned[0]);
    CHECK(population.header.transfer_id == "population-1");
    CHECK(population.population_type == "nanodevice");
    CHECK(population.count == 10'000);

    const auto& substance = std::get<SubstanceAmountTransfer>(returned[1]);
    CHECK(substance.header.transfer_id == "substance-1");
    CHECK(substance.substance_id == "oxygen");
    CHECK(mehlissa::core::in_moles(substance.amount) == 0.0025);

    const auto& flow = std::get<VolumeFlowTransfer>(returned[2]);
    CHECK(flow.header.transfer_id == "flow-1");
    CHECK(mehlissa::core::in_cubic_meters_per_second(flow.flow_rate) == 0.0001);
    CHECK(flow.interval == 2s);
    CHECK(mehlissa::core::in_cubic_meters(mehlissa::models::coupling::integrated_volume(flow)) ==
          0.0002);
}

TEST_CASE("Both lung variants reject invalid conserved transfer ownership",
          "[m3][organ][conservation]") {
    const auto variant =
        GENERATE(LungModelVariant::effective_compartment, LungModelVariant::regional_circulation);
    auto lung = mehlissa::models::organ::make_lung_model(lung_config(variant));
    auto* observer = lung.get();
    const std::string lung_model_id{observer->model_id()};

    mehlissa::core::ComponentHost host{std::uint64_t{2026}};
    host.add(std::move(lung));
    host.initialize();

    const PopulationTransfer accepted{input_header("population-1", lung_model_id), "nanodevice",
                                      100};
    observer->accept_conserved_transfer(accepted);
    CHECK_THROWS_AS(observer->accept_conserved_transfer(accepted), mehlissa::core::MehlissaError);

    auto wrong_route = input_header("population-2", lung_model_id);
    wrong_route.target_port_id = "wrong-entry";
    CHECK_THROWS_AS(observer->accept_conserved_transfer(
                        PopulationTransfer{std::move(wrong_route), "nanodevice", 100}),
                    mehlissa::core::MehlissaError);

    auto wrong_time = input_header("population-3", lung_model_id);
    wrong_time.emitted_at = 1s;
    CHECK_THROWS_AS(observer->accept_conserved_transfer(
                        PopulationTransfer{std::move(wrong_time), "nanodevice", 100}),
                    mehlissa::core::MehlissaError);
}

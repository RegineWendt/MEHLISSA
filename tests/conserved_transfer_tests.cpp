// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <string>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::coupling::TransferHeader;

[[nodiscard]] TransferHeader header(const std::string& id) {
    return {std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
            id,
            "body",
            "out",
            "lung",
            "in",
            5s};
}

} // namespace

TEST_CASE("Population substance and flow transfers balance by stable transfer ID",
          "[m3][coupling][conservation]") {
    using namespace mehlissa::models::coupling;

    const PopulationTransfer population{header("population-1"), "nanodevice", 10'000};
    const SubstanceAmountTransfer substance{header("substance-1"), "oxygen",
                                            mehlissa::core::millimoles(2.5)};
    const VolumeFlowTransfer flow{header("flow-1"), mehlissa::core::cubic_meters_per_second(0.0001),
                                  2s};

    ConservationLedger ledger;
    ledger.record_sent(population);
    ledger.record_sent(substance);
    ledger.record_sent(flow);
    CHECK(ledger.outstanding_transfer_count() == 3);

    ledger.record_received(population);
    ledger.record_received(substance);
    ledger.record_received(flow);
    CHECK(ledger.outstanding_transfer_count() == 0);
    CHECK_NOTHROW(ledger.verify_balanced());
    CHECK(mehlissa::core::in_cubic_meters(integrated_volume(flow)) == 0.0002);
}

TEST_CASE("A conservation ledger rejects changed payloads and duplicate transfer IDs",
          "[m3][coupling][conservation]") {
    using namespace mehlissa::models::coupling;

    const PopulationTransfer sent{header("population-1"), "nanodevice", 100};
    const PopulationTransfer changed{header("population-1"), "nanodevice", 99};
    ConservationLedger ledger;
    ledger.record_sent(sent);
    REQUIRE_THROWS_AS(ledger.record_sent(sent), mehlissa::core::MehlissaError);
    ledger.record_received(changed);
    CHECK(ledger.outstanding_transfer_count() == 2);
    REQUIRE_THROWS_AS(ledger.verify_balanced(), mehlissa::core::MehlissaError);
}

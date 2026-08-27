// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>
#include <mehlissa/models/organ/pulmonary_circulation.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace std::chrono_literals;

TEST_CASE("Detailed pulmonary regions implement the same entity contract",
          "[m3][organ][interchangeability]") {
    auto pulmonary = std::make_unique<mehlissa::models::organ::PulmonaryCirculation>(
        mehlissa::models::organ::PulmonaryCirculationConfig{"organ.lung.pulmonary-circulation",
                                                            "lung.pulmonary.v1",
                                                            "pulmonary-arterial-entry",
                                                            "pulmonary-venous-exit",
                                                            "body.bvs95",
                                                            "pulmonary-venous-return",
                                                            {{"pulmonary-artery", 500ms},
                                                             {"regional-capillary-surrogate", 1s},
                                                             {"pulmonary-vein", 500ms}}});
    auto* observer = pulmonary.get();
    mehlissa::core::ComponentHost host{std::uint64_t{7}};
    host.add(std::move(pulmonary));
    host.initialize();

    observer->accept_entity({
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        17,
        "nanodevice",
        "body.bvs95",
        "pulmonary-arterial-departure",
        "lung.pulmonary.v1",
        "pulmonary-arterial-entry",
        0ns,
    });
    CHECK(observer->region_count() == 3);
    host.advance(1s);
    CHECK(observer->resident_count() == 1);
    host.advance(1s);

    const auto outbound = observer->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().entity_id == 17);
    CHECK(outbound.front().source_port_id == "pulmonary-venous-exit");
    CHECK(outbound.front().target_port_id == "pulmonary-venous-return");
    CHECK(outbound.front().emitted_at == 2s);
}

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/legacy_95_migration.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::models::body::VascularGraph migrate_reference() {
    return mehlissa::models::body::migrate_legacy_95({
        root_path() / "mehlissa2.0" / "data" / "95_vasculature.csv",
        root_path() / "mehlissa2.0" / "data" / "95_transitions.csv",
    });
}

[[nodiscard]] std::string read_file(const std::filesystem::path& path) {
    std::ifstream input{path, std::ios::binary};
    REQUIRE(input);
    return {std::istreambuf_iterator<char>{input}, {}};
}

} // namespace

TEST_CASE("Legacy 95 data migrates losslessly into explicit SI topology", "[body][legacy-95]") {
    const auto graph = migrate_reference();

    REQUIRE(graph.model_id == "bvs95-dissertation-rest");
    REQUIRE(graph.segments.size() == 95);
    const auto* head = graph.find_segment("bvs95-009");
    REQUIRE(head != nullptr);
    REQUIRE(head->type == mehlissa::models::body::VesselType::organ_bed);
    REQUIRE(mehlissa::core::in_meters(head->geometry.length) ==
            Catch::Approx(0.04).epsilon(1.0e-12));
    REQUIRE(head->transitions.size() == 2);
    REQUIRE(head->transitions.at(0).successor_id == "bvs95-081");
    REQUIRE(head->transitions.at(1).successor_id == "bvs95-083");
}

TEST_CASE("Legacy vessel 9 uses the sourced left-right jugular cohort mean", "[body][legacy-95]") {
    const auto graph = migrate_reference();
    const auto* head = graph.find_segment("bvs95-009");
    REQUIRE(head != nullptr);

    REQUIRE(head->transitions.at(0).probability == Catch::Approx(0.2875));
    REQUIRE(head->transitions.at(1).probability == Catch::Approx(0.7125));
    REQUIRE(head->source_refs.back() == "jugular-flow-mri-2009");
}

TEST_CASE("Legacy migration turns relative dissertation perfusion into conserved SI flow",
          "[body][legacy-95]") {
    const auto graph = migrate_reference();
    const auto* heart = graph.find_segment("bvs95-002");
    const auto* systemic = graph.find_segment("bvs95-001");
    const auto* coronary = graph.find_segment("bvs95-095");
    REQUIRE(heart != nullptr);
    REQUIRE(systemic != nullptr);
    REQUIRE(coronary != nullptr);

    REQUIRE(mehlissa::core::in_cubic_meters_per_second(heart->hemodynamics.flow_rate) ==
            Catch::Approx(0.0001));
    REQUIRE(mehlissa::core::in_cubic_meters_per_second(systemic->hemodynamics.flow_rate) ==
            Catch::Approx(0.000095));
    REQUIRE(mehlissa::core::in_cubic_meters_per_second(coronary->hemodynamics.flow_rate) ==
            Catch::Approx(0.000005));
    REQUIRE_NOTHROW(mehlissa::models::body::validate_vascular_graph(graph));
}

TEST_CASE("The checked-in legacy migration remains schema-valid", "[body][legacy-95][schema]") {
    const auto graph = mehlissa::models::body::load_vascular_graph({
        root_path() / "data" / "body-models" / "bvs95-dissertation-rest-v1.json",
        root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });

    REQUIRE(graph.model_id == "bvs95-dissertation-rest");
    REQUIRE(graph.segments.size() == 95);
}

TEST_CASE("Legacy migration reproduces the checked-in graph byte for byte",
          "[body][legacy-95][determinism]") {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto generated_path = std::filesystem::temp_directory_path() /
                                ("mehlissa-bvs95-" + std::to_string(unique) + ".json");
    mehlissa::models::body::write_vascular_graph(migrate_reference(), generated_path);

    const auto reference_path =
        root_path() / "data" / "body-models" / "bvs95-dissertation-rest-v1.json";
    REQUIRE(read_file(generated_path) == read_file(reference_path));
    std::filesystem::remove(generated_path);
}

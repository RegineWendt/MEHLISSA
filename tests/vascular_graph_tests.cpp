// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/vascular_graph.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] mehlissa::models::body::VascularGraph load_reference_graph() {
    return mehlissa::models::body::load_vascular_graph({
        root_path() / "examples" / "body-models" / "synthetic-branching-circuit.json",
        root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
    });
}

} // namespace

TEST_CASE("A versioned vascular graph loads with SI quantities and stable IDs",
          "[body][vascular-graph]") {
    const auto graph = load_reference_graph();

    REQUIRE(graph.schema_version == "1.0.0");
    REQUIRE(graph.model_id == "synthetic-branching-circuit");
    REQUIRE(graph.segments.size() == 4);
    const auto* artery = graph.find_segment("artery-10");
    REQUIRE(artery != nullptr);
    REQUIRE(artery->type == mehlissa::models::body::VesselType::artery);
    REQUIRE(mehlissa::core::in_meters(artery->geometry.length) == 0.4);
    REQUIRE(mehlissa::core::in_cubic_meters_per_second(artery->hemodynamics.flow_rate) == 0.00001);
    REQUIRE(artery->transitions.size() == 2);
    REQUIRE(graph.find_segment("missing") == nullptr);
}

TEST_CASE("A vascular graph rejects incomplete transition probabilities",
          "[body][vascular-graph]") {
    auto graph = load_reference_graph();
    graph.segments.front().transitions.front().probability = 0.3;

    REQUIRE_THROWS_WITH(mehlissa::models::body::validate_vascular_graph(graph),
                        Catch::Matchers::ContainsSubstring("do not sum to one"));
}

TEST_CASE("A vascular graph rejects an inconsistent geometric length", "[body][vascular-graph]") {
    auto graph = load_reference_graph();
    graph.segments.front().geometry.length = mehlissa::core::meters(0.5);

    REQUIRE_THROWS_WITH(mehlissa::models::body::validate_vascular_graph(graph),
                        Catch::Matchers::ContainsSubstring("length does not match"));
}

TEST_CASE("A vascular graph rejects flow that violates junction conservation",
          "[body][vascular-graph]") {
    auto graph = load_reference_graph();
    auto* branch = &graph.segments.at(1);
    branch->hemodynamics.flow_rate = mehlissa::core::cubic_meters_per_second(0.000005);
    branch->hemodynamics.mean_velocity = mehlissa::core::meters_per_second(
        0.000005 / mehlissa::core::in_square_meters(branch->geometry.cross_section_area));
    graph.segments.front().transitions = {
        {"organ-a", 5.0 / 11.0},
        {"organ-b", 6.0 / 11.0},
    };

    REQUIRE_THROWS_WITH(mehlissa::models::body::validate_vascular_graph(graph),
                        Catch::Matchers::ContainsSubstring("Flow is not conserved"));
}

TEST_CASE("A vascular graph rejects unknown source references", "[body][vascular-graph]") {
    auto graph = load_reference_graph();
    graph.segments.front().source_refs.front() = "missing-source";

    REQUIRE_THROWS_WITH(mehlissa::models::body::validate_vascular_graph(graph),
                        Catch::Matchers::ContainsSubstring("unknown data source"));
}

TEST_CASE("The vascular schema rejects unknown fields before semantic decoding",
          "[body][vascular-graph][schema]") {
    const auto source_path =
        root_path() / "examples" / "body-models" / "synthetic-branching-circuit.json";
    std::ifstream source{source_path, std::ios::binary};
    REQUIRE(source);
    std::string contents{std::istreambuf_iterator<char>{source}, {}};
    const auto field = contents.find("flow_rate_m3_s");
    REQUIRE(field != std::string::npos);
    contents.replace(field, std::string{"flow_rate_m3_s"}.size(), "unexpected_flow");

    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto invalid_path = std::filesystem::temp_directory_path() /
                              ("mehlissa-invalid-vascular-" + std::to_string(unique) + ".json");
    {
        std::ofstream output{invalid_path, std::ios::binary};
        REQUIRE(output);
        output << contents;
    }

    REQUIRE_THROWS_WITH(mehlissa::models::body::load_vascular_graph(
                            {invalid_path, root_path() / "data" / "schemas" / "vascular-graph" /
                                               "1.0.0.schema.json"}),
                        Catch::Matchers::ContainsSubstring("does not satisfy its schema"));
    std::filesystem::remove(invalid_path);
}

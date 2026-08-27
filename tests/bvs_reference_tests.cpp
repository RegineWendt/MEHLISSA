// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/bvs_reference.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] const mehlissa::models::body::BvsReferenceReport& reference_report() {
    static const auto report = mehlissa::models::body::run_bvs_reference(
        mehlissa::models::body::load_vascular_graph({
            root_path() / "data" / "body-models" / "bvs95-dissertation-rest-v1.json",
            root_path() / "data" / "schemas" / "vascular-graph" / "1.0.0.schema.json",
        }));
    return report;
}

[[nodiscard]] std::string read_file(const std::filesystem::path& path) {
    std::ifstream input{path, std::ios::binary};
    REQUIRE(input);
    return {std::istreambuf_iterator<char>{input}, {}};
}

} // namespace

TEST_CASE("M2.4 reference run passes its gates and reproduces its report",
          "[body][bvs][schema][determinism][slow]") {
    const auto& report = reference_report();

    REQUIRE(report.population_conserved);
    REQUIRE(report.perfusion_gate_passed);
    REQUIRE(report.equilibrium_gate_passed);
    REQUIRE(report.injection_site_gate_passed);
    REQUIRE(report.population_scale_gate_passed);
    REQUIRE(report.overall_passed);

    REQUIRE(report.reference_particle_count == 6'359);
    REQUIRE(report.large_particle_count == 63'590);
    REQUIRE(report.perfusion.size() == 23);
    REQUIRE(report.mean_perfusion_error_vs_literature_percentage_points <= 0.01);
    REQUIRE(report.maximum_perfusion_error_vs_literature_percentage_points <= 0.01);
    REQUIRE(report.mean_perfusion_difference_vs_dissertation_percentage_points <= 0.5);

    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto generated_path = std::filesystem::temp_directory_path() /
                                ("mehlissa-bvs-reference-" + std::to_string(unique) + ".json");
    mehlissa::models::body::write_bvs_reference_report(
        reference_report(),
        {generated_path,
         root_path() / "data" / "schemas" / "bvs-reference-report" / "1.0.0.schema.json"});

    const auto reference_path =
        root_path() / "data" / "reference-results" / "bvs95-dissertation-rest-m2.4.json";
    REQUIRE(read_file(generated_path) == read_file(reference_path));
    std::filesystem::remove(generated_path);
}

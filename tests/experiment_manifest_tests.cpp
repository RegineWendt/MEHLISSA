// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/experiment_manifest.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] std::filesystem::path schema_path() {
    return root_path() / "data" / "schemas" / "experiment" / "1.0.0.schema.json";
}

[[nodiscard]] std::filesystem::path experiment_path(const std::string& filename) {
    return root_path() / "tests" / "data" / "experiments" / filename;
}

} // namespace

TEST_CASE("A minimal versioned experiment manifest is decoded", "[experiment][manifest]") {
    using namespace std::chrono_literals;

    const auto manifest = mehlissa::experiment::load_experiment_manifest(
        root_path() / "examples" / "experiments" / "minimal.json", schema_path());

    REQUIRE(manifest.schema_version == "1.0.0");
    REQUIRE(manifest.experiment_id == "minimal-deterministic-run");
    REQUIRE(manifest.duration == 10s);
    REQUIRE(manifest.master_seed == std::uint64_t{12345});
    REQUIRE(manifest.models.empty());
    REQUIRE(manifest.output_directory ==
            std::filesystem::path{"results/minimal-deterministic-run"});
}

TEST_CASE("The schema rejects a missing master seed", "[experiment][manifest]") {
    REQUIRE_THROWS_AS(mehlissa::experiment::load_experiment_manifest(
                          experiment_path("missing-seed.json"), schema_path()),
                      mehlissa::experiment::ManifestError);
}

TEST_CASE("The schema rejects unknown properties", "[experiment][manifest]") {
    REQUIRE_THROWS_AS(mehlissa::experiment::load_experiment_manifest(
                          experiment_path("unknown-property.json"), schema_path()),
                      mehlissa::experiment::ManifestError);
}

TEST_CASE("Duration conversion rejects nanosecond overflow", "[experiment][manifest]") {
    REQUIRE_THROWS_WITH(mehlissa::experiment::load_experiment_manifest(
                            experiment_path("duration-overflow.json"), schema_path()),
                        "Experiment duration exceeds the supported nanosecond range");
}

TEST_CASE("Missing manifest and schema files report their role", "[experiment][manifest]") {
    REQUIRE_THROWS_WITH(
        mehlissa::experiment::load_experiment_manifest(experiment_path("does-not-exist.json"),
                                                       schema_path()),
        "Cannot open experiment manifest: " + experiment_path("does-not-exist.json").string());

    REQUIRE_THROWS_WITH(
        mehlissa::experiment::load_experiment_manifest(
            root_path() / "examples" / "experiments" / "minimal.json",
            root_path() / "data" / "schemas" / "experiment" / "does-not-exist.json"),
        "Cannot open experiment schema: " +
            (root_path() / "data" / "schemas" / "experiment" / "does-not-exist.json").string());
}

TEST_CASE("An invalid experiment schema is rejected before the manifest",
          "[experiment][manifest]") {
    const auto invalid_schema =
        root_path() / "tests" / "data" / "schemas" / "invalid-experiment-schema.json";

    REQUIRE_THROWS_WITH(
        mehlissa::experiment::load_experiment_manifest(
            root_path() / "examples" / "experiments" / "minimal.json", invalid_schema),
        Catch::Matchers::StartsWith("Invalid experiment schema '" + invalid_schema.string() +
                                    "':"));
}

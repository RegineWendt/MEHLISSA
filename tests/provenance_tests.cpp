// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/experiment_manifest.hpp>
#include <mehlissa/experiment/provenance.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] std::filesystem::path experiment_schema_path() {
    return root_path() / "data" / "schemas" / "experiment" / "1.0.0.schema.json";
}

[[nodiscard]] std::filesystem::path provenance_schema_path() {
    return root_path() / "data" / "schemas" / "provenance" / "1.0.0.schema.json";
}

[[nodiscard]] std::filesystem::path make_test_directory() {
    const auto unique_value = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("mehlissa-provenance-test-" + std::to_string(unique_value));
}

} // namespace

TEST_CASE("SHA-256 uses the standard digest", "[experiment][provenance]") {
    const auto directory = make_test_directory();
    const auto input_path = directory / "input.bin";
    std::filesystem::create_directories(directory);
    {
        std::ofstream input{input_path, std::ios::binary};
        REQUIRE(input);
        input << "abc";
    }

    REQUIRE(mehlissa::experiment::sha256_file(input_path) ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    std::filesystem::remove_all(directory);
}

TEST_CASE("SHA-256 reports a missing input file", "[experiment][provenance]") {
    const auto missing_path = make_test_directory() / "missing.json";
    REQUIRE_THROWS_WITH(mehlissa::experiment::sha256_file(missing_path),
                        "Cannot open file for SHA-256: " + missing_path.string());
}

TEST_CASE("Build metadata identifies the current binary", "[experiment][provenance]") {
    const auto metadata = mehlissa::experiment::current_build_metadata();

    REQUIRE(!metadata.software_version.empty());
    REQUIRE((metadata.git_commit == "unknown" || metadata.git_commit.size() == 40));
    REQUIRE(!metadata.build_type.empty());
    REQUIRE(!metadata.compiler_id.empty());
    REQUIRE(!metadata.compiler_version.empty());
    REQUIRE(!metadata.operating_system.empty());
    REQUIRE(!metadata.architecture.empty());
}

TEST_CASE("A run writes schema-valid reproducibility evidence", "[experiment][provenance]") {
    using namespace std::chrono_literals;

    const auto manifest_path = root_path() / "examples" / "experiments" / "minimal.json";
    const auto manifest =
        mehlissa::experiment::load_experiment_manifest(manifest_path, experiment_schema_path());
    const auto directory = make_test_directory();
    const auto output_path = directory / "provenance.json";

    const mehlissa::experiment::ProvenanceRequest request{
        manifest_path,
        output_path,
        {
            "2026-08-26T15:00:00Z",
            "2026-08-26T15:00:01Z",
            "completed",
            10s,
        },
    };
    const mehlissa::experiment::BuildMetadata build{
        "0.1.0",   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        false,     "Debug",
        "MSVC",    "19.50",
        "Windows", "AMD64",
    };

    mehlissa::experiment::write_provenance(manifest, request, build);
    mehlissa::experiment::validate_provenance_file({output_path, provenance_schema_path()});

    {
        std::ifstream output{output_path, std::ios::binary};
        REQUIRE(output);
        const std::string contents{std::istreambuf_iterator<char>{output}, {}};
        REQUIRE(contents.find(mehlissa::experiment::sha256_file(manifest_path)) !=
                std::string::npos);
        REQUIRE(contents.find("\"master_seed\": 12345") != std::string::npos);
        REQUIRE(contents.find("\"simulation_time_ns\": 10000000000") != std::string::npos);
    }

    std::filesystem::remove_all(directory);
}

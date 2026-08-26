// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/experiment/checkpoint_manifest.hpp>

#include <mehlissa/experiment/provenance.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

[[nodiscard]] std::filesystem::path root_path() { return MEHLISSA_TEST_ROOT; }

[[nodiscard]] std::filesystem::path checkpoint_schema_path() {
    return root_path() / "data" / "schemas" / "checkpoint" / "1.0.0.schema.json";
}

[[nodiscard]] std::filesystem::path make_test_directory() {
    const auto unique_value = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("mehlissa-checkpoint-test-" + std::to_string(unique_value));
}

[[nodiscard]] mehlissa::experiment::CheckpointManifest
make_checkpoint(const std::filesystem::path& directory) {
    using namespace std::chrono_literals;

    const auto state_path = directory / "components" / "transport.json";
    std::filesystem::create_directories(state_path.parent_path());
    {
        std::ofstream state{state_path, std::ios::binary};
        state << "{\"counter\":7}\n";
    }

    return {
        "1.0.0",
        "minimal-deterministic-run",
        mehlissa::experiment::sha256_file(root_path() / "examples" / "experiments" /
                                          "minimal.json"),
        "0.1.0",
        3,
        "2026-08-26T21:00:00Z",
        42ns,
        12345,
        {{"z-stream", 9}, {"a-stream", 2}},
        {{"transport", "1.0.0", "components/transport.json",
          mehlissa::experiment::sha256_file(state_path)}},
    };
}

} // namespace

TEST_CASE("A checkpoint manifest round-trips with verified component state",
          "[experiment][checkpoint]") {
    using namespace std::chrono_literals;

    const auto directory = make_test_directory();
    const auto checkpoint_path = directory / "checkpoint.json";
    const auto checkpoint = make_checkpoint(directory);

    mehlissa::experiment::write_checkpoint_manifest(checkpoint,
                                                    {checkpoint_path, checkpoint_schema_path()});
    const auto restored =
        mehlissa::experiment::load_checkpoint_manifest({checkpoint_path, checkpoint_schema_path()});

    REQUIRE(restored.schema_version == "1.0.0");
    REQUIRE(restored.experiment_id == checkpoint.experiment_id);
    REQUIRE(restored.experiment_manifest_sha256 == checkpoint.experiment_manifest_sha256);
    REQUIRE(restored.software_version == "0.1.0");
    REQUIRE(restored.sequence == 3);
    REQUIRE(restored.simulation_time == 42ns);
    REQUIRE(restored.master_seed == 12345);
    REQUIRE(restored.random_streams ==
            std::vector<mehlissa::core::RandomStreamState>{{"a-stream", 2}, {"z-stream", 9}});
    REQUIRE(restored.components == checkpoint.components);

    std::filesystem::remove_all(directory);
}

TEST_CASE("A checkpoint rejects a component state outside its directory",
          "[experiment][checkpoint]") {
    const auto directory = make_test_directory();
    auto checkpoint = make_checkpoint(directory);
    checkpoint.components.front().state_file = "../transport.json";

    REQUIRE_THROWS_AS(mehlissa::experiment::write_checkpoint_manifest(
                          checkpoint, {directory / "checkpoint.json", checkpoint_schema_path()}),
                      mehlissa::experiment::CheckpointError);

    std::filesystem::remove_all(directory);
}

TEST_CASE("A checkpoint detects modified component state", "[experiment][checkpoint]") {
    const auto directory = make_test_directory();
    const auto checkpoint_path = directory / "checkpoint.json";
    const auto checkpoint = make_checkpoint(directory);
    mehlissa::experiment::write_checkpoint_manifest(checkpoint,
                                                    {checkpoint_path, checkpoint_schema_path()});

    {
        std::ofstream state{directory / "components" / "transport.json", std::ios::binary};
        state << "{\"counter\":8}\n";
    }

    REQUIRE_THROWS_AS(
        mehlissa::experiment::load_checkpoint_manifest({checkpoint_path, checkpoint_schema_path()}),
        mehlissa::experiment::CheckpointError);

    std::filesystem::remove_all(directory);
}

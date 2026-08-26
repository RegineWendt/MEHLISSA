// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_EXPERIMENT_CHECKPOINT_MANIFEST_HPP
#define MEHLISSA_EXPERIMENT_CHECKPOINT_MANIFEST_HPP

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/core/simulation_context.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::experiment {

inline constexpr auto supported_checkpoint_schema_version = "1.0.0";

struct ComponentSnapshotReference final {
    std::string name;
    std::string state_schema_version;
    std::filesystem::path state_file;
    std::string sha256;

    [[nodiscard]] bool operator==(const ComponentSnapshotReference&) const = default;
};

struct CheckpointManifest final {
    std::string schema_version{supported_checkpoint_schema_version};
    std::string experiment_id;
    std::string experiment_manifest_sha256;
    std::string software_version;
    std::uint64_t sequence{};
    std::string created_at_utc;
    core::SimulationClock::Duration simulation_time;
    std::uint64_t master_seed{};
    std::vector<core::RandomStreamState> random_streams;
    std::vector<ComponentSnapshotReference> components;
};

struct CheckpointWriteRequest final {
    std::filesystem::path output_path;
    std::filesystem::path schema_path;
};

struct CheckpointLoadRequest final {
    std::filesystem::path checkpoint_path;
    std::filesystem::path schema_path;
};

class CheckpointError final : public core::MehlissaError {
  public:
    using core::MehlissaError::MehlissaError;
};

void write_checkpoint_manifest(const CheckpointManifest& checkpoint,
                               const CheckpointWriteRequest& request);

[[nodiscard]] CheckpointManifest load_checkpoint_manifest(const CheckpointLoadRequest& request);

} // namespace mehlissa::experiment

#endif // MEHLISSA_EXPERIMENT_CHECKPOINT_MANIFEST_HPP

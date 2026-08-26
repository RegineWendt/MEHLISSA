// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_EXPERIMENT_EXPERIMENT_MANIFEST_HPP
#define MEHLISSA_EXPERIMENT_EXPERIMENT_MANIFEST_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace mehlissa::experiment {

inline constexpr auto supported_experiment_schema_version = "1.0.0";

struct ExperimentManifest final {
    std::string schema_version;
    std::string experiment_id;
    core::SimulationClock::Duration duration;
    std::uint64_t master_seed{};
    std::vector<std::string> models;
    std::filesystem::path output_directory;
};

class ManifestError final : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

[[nodiscard]] ExperimentManifest
load_experiment_manifest(const std::filesystem::path& manifest_path,
                         const std::filesystem::path& schema_path);

} // namespace mehlissa::experiment

#endif // MEHLISSA_EXPERIMENT_EXPERIMENT_MANIFEST_HPP

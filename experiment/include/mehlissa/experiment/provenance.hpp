// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_EXPERIMENT_PROVENANCE_HPP
#define MEHLISSA_EXPERIMENT_PROVENANCE_HPP

#include <mehlissa/core/simulation_clock.hpp>
#include <mehlissa/experiment/experiment_manifest.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace mehlissa::experiment {

inline constexpr auto supported_provenance_schema_version = "1.0.0";

struct BuildMetadata final {
    std::string software_version;
    std::string git_commit;
    bool git_dirty{};
    std::string build_type;
    std::string compiler_id;
    std::string compiler_version;
    std::string operating_system;
    std::string architecture;
};

struct RunMetadata final {
    std::string started_at_utc;
    std::string completed_at_utc;
    std::string status;
    core::SimulationClock::Duration simulation_time;
};

struct ProvenanceRequest final {
    std::filesystem::path manifest_path;
    std::filesystem::path output_path;
    RunMetadata run;
};

struct ProvenanceValidation final {
    std::filesystem::path document_path;
    std::filesystem::path schema_path;
};

class ProvenanceError final : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

[[nodiscard]] BuildMetadata current_build_metadata();
[[nodiscard]] std::string current_utc_timestamp();
[[nodiscard]] std::string sha256_file(const std::filesystem::path& path);

void write_provenance(const ExperimentManifest& manifest, const ProvenanceRequest& request,
                      const BuildMetadata& build_metadata);

void write_provenance(const ExperimentManifest& manifest, const ProvenanceRequest& request);

void validate_provenance_file(const ProvenanceValidation& validation);

} // namespace mehlissa::experiment

#endif // MEHLISSA_EXPERIMENT_PROVENANCE_HPP

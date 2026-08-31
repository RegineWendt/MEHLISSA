// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_PROFILE_HPP

#include <mehlissa/models/cell/receptor_ligand_model.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto receptor_ligand_profile_schema_version = "1.0.0";

struct ReceptorLigandValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct ReceptorLigandSource final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct ReceptorLigandReferenceCase final {
    std::string request_id;
    core::Concentration ligand_concentration{};
    core::SimulationClock::Duration observation_time{};
    double initial_bound_fraction{};
    double expected_final_bound_fraction{};
    double expected_threshold_crossing_seconds{};
    double absolute_tolerance{};
};

struct ReceptorLigandProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    ReceptorLigandModelConfig model;
    ReceptorLigandReferenceCase reference_case;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct ReceptorLigandProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_receptor_ligand_profile(const ReceptorLigandProfile& profile);

[[nodiscard]] ReceptorLigandProfile
load_receptor_ligand_profile(const ReceptorLigandProfileLoadRequest& request);
[[nodiscard]] std::unique_ptr<ReceptorLigandModel>
make_receptor_ligand_model(const ReceptorLigandProfile& profile);
[[nodiscard]] ReceptorLigandRequest
make_receptor_ligand_reference_request(const ReceptorLigandProfile& profile);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_RECEPTOR_LIGAND_PROFILE_HPP

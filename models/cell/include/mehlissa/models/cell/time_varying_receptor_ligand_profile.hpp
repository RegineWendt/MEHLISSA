// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_PROFILE_HPP

#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cell/time_varying_receptor_ligand_model.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto time_varying_receptor_ligand_profile_schema_version = "1.0.0";
inline constexpr auto constant_analytical_limit_role = "constant_analytical_limit";
inline constexpr auto piecewise_analytical_pulse_role = "piecewise_analytical_pulse";

struct TimeVaryingReceptorLigandReferenceCase final {
    std::string role;
    TimeVaryingReceptorLigandRequest request;
    double expected_final_bound_fraction{};
    double expected_peak_bound_fraction{};
    double expected_threshold_crossing_seconds{};
    double fraction_absolute_tolerance{};
    double time_absolute_tolerance_seconds{};
};

struct TimeVaryingReceptorLigandProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    TimeVaryingReceptorLigandModelConfig model;
    std::vector<TimeVaryingReceptorLigandReferenceCase> reference_cases;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct TimeVaryingReceptorLigandProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_time_varying_receptor_ligand_profile(const TimeVaryingReceptorLigandProfile& profile);
[[nodiscard]] TimeVaryingReceptorLigandProfile load_time_varying_receptor_ligand_profile(
    const TimeVaryingReceptorLigandProfileLoadRequest& request);
[[nodiscard]] std::unique_ptr<TimeVaryingReceptorLigandModel>
make_time_varying_receptor_ligand_model(const TimeVaryingReceptorLigandProfile& profile);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_TIME_VARYING_RECEPTOR_LIGAND_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_PROFILE_HPP

#include <mehlissa/models/cell/apoptosis_population.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto apoptosis_population_profile_schema_version = "1.0.0";

struct ApoptosisPopulationExpected final {
    std::uint64_t total_cells{};
    std::uint64_t viable_cells{};
    std::uint64_t apoptosis_committed_cells{};
    double apoptosis_committed_fraction{};
    double cell_weighted_mean_effect_fraction{};
    double fraction_tolerance{};
};

struct ApoptosisPopulationSensitivityCase final {
    std::string case_id;
    core::Amount half_max_effect_amount{};
    double hill_coefficient{};
    double apoptosis_commitment_threshold{};
    double expected_apoptosis_committed_fraction{};
    double expected_mean_effect_fraction{};
};

struct ApoptosisPopulationProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    ApoptosisPopulationConfig model;
    ApoptosisPopulationRequest reference_request;
    ApoptosisPopulationExpected expected;
    std::vector<ApoptosisPopulationSensitivityCase> sensitivity_cases;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct ApoptosisPopulationProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_apoptosis_population_profile(const ApoptosisPopulationProfile& profile);
[[nodiscard]] ApoptosisPopulationProfile
load_apoptosis_population_profile(const ApoptosisPopulationProfileLoadRequest& request);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_APOPTOSIS_POPULATION_PROFILE_HPP

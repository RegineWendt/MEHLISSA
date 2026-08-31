// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_PROFILE_HPP

#include <mehlissa/models/cell/receptor_ligand_profile.hpp>
#include <mehlissa/models/cell/stochastic_receptor_ligand_population.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto stochastic_receptor_ligand_profile_schema_version = "1.0.0";

struct StochasticPopulationReference final {
    StochasticPopulationConfig ensemble;
    StochasticPopulationRequest request;
    double expected_positive_mean{};
    double expected_negative_mean{};
    double expected_positive_variance{};
    double expected_negative_variance{};
    double mean_absolute_tolerance{};
    double variance_absolute_tolerance{};
    double maximum_false_negative_rate{};
    double maximum_false_positive_rate{};
};

struct StochasticReceptorLigandProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    StochasticReceptorLigandModelConfig model;
    StochasticPopulationReference reference;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct StochasticReceptorLigandProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_stochastic_receptor_ligand_profile(const StochasticReceptorLigandProfile& profile);
[[nodiscard]] StochasticReceptorLigandProfile
load_stochastic_receptor_ligand_profile(const StochasticReceptorLigandProfileLoadRequest& request);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_STOCHASTIC_RECEPTOR_LIGAND_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_PROFILE_HPP

#include <mehlissa/models/cell/intracellular_response_network.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto intracellular_response_profile_schema_version = "1.0.0";

struct IntracellularComparisonReference final {
    IntracellularNetworkRequest request;
    std::uint64_t master_seed{};
    std::string stream_prefix;
    std::size_t population_size{};
    double expected_ode_messenger_fraction{};
    double expected_ode_effector_fraction{};
    double expected_ode_response_seconds{};
    double ode_fraction_tolerance{};
    double ode_time_tolerance_seconds{};
    double maximum_ssa_messenger_mean_error{};
    double maximum_ssa_effector_mean_error{};
};

struct IntracellularResponseProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    IntracellularOdeConfig ode;
    IntracellularSsaConfig ssa;
    IntracellularComparisonReference reference;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct IntracellularPopulationComparison final {
    IntracellularOdeResponse deterministic;
    std::size_t population_size{};
    double mean_messenger_fraction{};
    double variance_messenger_fraction{};
    double mean_effector_fraction{};
    double variance_effector_fraction{};
    std::size_t responding_cells{};
    std::size_t total_reaction_events{};
    std::uint64_t total_random_draws{};
};

struct IntracellularResponseProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_intracellular_response_profile(const IntracellularResponseProfile& profile);
[[nodiscard]] IntracellularResponseProfile
load_intracellular_response_profile(const IntracellularResponseProfileLoadRequest& request);
[[nodiscard]] IntracellularPopulationComparison
compare_intracellular_ode_ssa(const IntracellularResponseProfile& profile);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_INTRACELLULAR_RESPONSE_PROFILE_HPP

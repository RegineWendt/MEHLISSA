// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_PROFILE_HPP
#define MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_PROFILE_HPP

#include <mehlissa/models/cosimulation/capillary_cell_signal_coupler.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cosimulation {

inline constexpr auto capillary_cell_signal_profile_schema_version = "1.0.0";

struct CapillaryCellSignalReferenceCase final {
    std::string sample_id;
    core::SimulationClock::Duration observed_at{};
    core::Amount expected_source_amount{};
    core::Concentration expected_ligand_concentration{};
    double expected_final_bound_fraction{};
    double expected_threshold_crossing_seconds{};
    double absolute_tolerance{};
};

struct CapillaryCellSignalValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct CapillaryCellSignalSourceReference final {
    std::string id;
    std::string citation;
    std::string location;
    std::string license;
    std::string role;
};

struct CapillaryCellSignalProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    CapillaryCellSignalCouplerConfig coupler;
    CapillaryCellSignalReferenceCase reference_case;
    CapillaryCellSignalValidity validity;
    std::vector<CapillaryCellSignalSourceReference> sources;
    std::vector<std::string> limitations;
};

struct CapillaryCellSignalProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_capillary_cell_signal_profile(const CapillaryCellSignalProfile& profile);
[[nodiscard]] CapillaryCellSignalProfile
load_capillary_cell_signal_profile(const CapillaryCellSignalProfileLoadRequest& request);
[[nodiscard]] CapillaryCellSignalCoupler
make_capillary_cell_signal_coupler(const CapillaryCellSignalProfile& profile);

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_PROFILE_HPP

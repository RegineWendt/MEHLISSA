// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_PROFILE_HPP

#include <mehlissa/models/cell/apoptosis_response.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto apoptosis_response_profile_schema_version = "1.0.0";

struct HigherLayerFeedbackTarget final {
    std::string event_id;
    std::string target_model_id;
    std::string target_port_id;
};

struct ApoptosisResponseReferenceCase final {
    std::string request_id;
    std::string source_delivery_request_id;
    std::string source_delivery_model_id;
    core::Amount expected_intracellular_drug_amount{};
    double expected_effect_fraction{};
    CellState expected_state{CellState::viable};
    core::Amount amount_tolerance{};
    double effect_tolerance{};
};

struct ApoptosisResponseProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    ApoptosisResponseConfig model;
    HigherLayerFeedbackTarget feedback_target;
    ApoptosisResponseReferenceCase reference_case;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct ApoptosisResponseProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_apoptosis_response_profile(const ApoptosisResponseProfile& profile);
[[nodiscard]] ApoptosisResponseProfile
load_apoptosis_response_profile(const ApoptosisResponseProfileLoadRequest& request);
[[nodiscard]] ApoptosisResponseRequest
make_apoptosis_reference_request(const ApoptosisResponseProfile& profile,
                                 const DrugDeliveryResponse& delivery);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_APOPTOSIS_RESPONSE_PROFILE_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_DRUG_DELIVERY_PROFILE_HPP
#define MEHLISSA_MODELS_CELL_DRUG_DELIVERY_PROFILE_HPP

#include <mehlissa/models/cell/drug_delivery.hpp>
#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr auto drug_delivery_profile_schema_version = "1.0.0";

struct DrugDeliveryReferenceCase final {
    std::string request_id;
    NanodeviceActivationTarget activation_target;
    std::string source_request_id;
    std::string source_network_id;
    core::SimulationClock::Duration expected_activation_offset{};
    core::SimulationClock::Duration observation_after_activation{};
    core::Amount expected_device_payload_amount{};
    core::Amount expected_extracellular_drug_amount{};
    core::Amount expected_intracellular_drug_amount{};
    core::Amount amount_tolerance{};
    double activation_time_tolerance_seconds{};
};

struct DrugDeliveryProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string implementation_kind;
    DrugDeliveryConfig model;
    DrugDeliveryReferenceCase reference_case;
    ReceptorLigandValidity validity;
    std::vector<ReceptorLigandSource> sources;
    std::vector<std::string> limitations;
};

struct DrugDeliveryProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_drug_delivery_profile(const DrugDeliveryProfile& profile);
[[nodiscard]] DrugDeliveryProfile
load_drug_delivery_profile(const DrugDeliveryProfileLoadRequest& request);
[[nodiscard]] DrugDeliveryRequest
make_drug_delivery_reference_request(const DrugDeliveryProfile& profile,
                                     const NanodeviceActivationSignal& activation);

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_DRUG_DELIVERY_PROFILE_HPP

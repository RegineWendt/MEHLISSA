// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/drug_delivery_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::cell {
namespace {

using Json = jsoncons::json;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot open " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid, "Invalid JSON in " + std::string{role} + " '" +
                                                   path.string() + "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration duration_seconds(const double value) {
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] DrugDeliveryProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& model = document.at("model");
    const auto& reference = document.at("reference_case");
    const auto& activation = reference.at("activation");
    const auto& expected = reference.at("expected");
    const auto& validity = document.at("validity");
    DrugDeliveryProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("implementation_kind").as<std::string>(),
        {model.at("model_id").as<std::string>(), model.at("nanodevice_id").as<std::string>(),
         model.at("payload_id").as<std::string>(), model.at("drug_id").as<std::string>(),
         core::moles(model.at("loaded_amount_mol").as<double>()),
         core::per_second(model.at("release_rate_s_1").as<double>()),
         core::per_second(model.at("uptake_rate_s_1").as<double>())},
        {reference.at("request_id").as<std::string>(),
         {activation.at("activation_id").as<std::string>(),
          activation.at("nanodevice_id").as<std::string>(),
          activation.at("payload_id").as<std::string>()},
         activation.at("source_request_id").as<std::string>(),
         activation.at("source_network_id").as<std::string>(),
         duration_seconds(activation.at("expected_trigger_offset_seconds").as<double>()),
         duration_seconds(reference.at("observation_after_activation_seconds").as<double>()),
         core::moles(expected.at("device_payload_amount_mol").as<double>()),
         core::moles(expected.at("extracellular_drug_amount_mol").as<double>()),
         core::moles(expected.at("intracellular_drug_amount_mol").as<double>()),
         core::moles(expected.at("amount_tolerance_mol").as<double>()),
         expected.at("activation_time_tolerance_seconds").as<double>()},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {}};
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

[[nodiscard]] bool positive_finite(const double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

} // namespace

void validate_drug_delivery_profile(const DrugDeliveryProfile& profile) {
    const auto& reference = profile.reference_case;
    const auto expected_total = core::in_moles(reference.expected_device_payload_amount) +
                                core::in_moles(reference.expected_extracellular_drug_amount) +
                                core::in_moles(reference.expected_intracellular_drug_amount);
    const auto loaded = core::in_moles(profile.model.loaded_amount);
    const auto amount_tolerance = core::in_moles(reference.amount_tolerance);
    if (profile.schema_version != drug_delivery_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != analytical_conservative_drug_delivery_kind ||
        reference.request_id.empty() || reference.activation_target.activation_id.empty() ||
        reference.activation_target.nanodevice_id != profile.model.nanodevice_id ||
        reference.activation_target.payload_id != profile.model.payload_id ||
        reference.source_request_id.empty() || reference.source_network_id.empty() ||
        reference.expected_activation_offset < core::SimulationClock::Duration::zero() ||
        reference.observation_after_activation <= core::SimulationClock::Duration::zero() ||
        !positive_finite(core::in_moles(reference.expected_device_payload_amount)) ||
        !positive_finite(core::in_moles(reference.expected_extracellular_drug_amount)) ||
        !positive_finite(core::in_moles(reference.expected_intracellular_drug_amount)) ||
        !positive_finite(amount_tolerance) ||
        std::abs(expected_total - loaded) > amount_tolerance ||
        !positive_finite(reference.activation_time_tolerance_seconds) ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Drug-delivery profile is incomplete or inconsistent");
    }

    const AnalyticalDrugDeliveryModel model{profile.model};
    const NanodeviceActivationSignal activation{std::string{nanodevice_activation_contract_version},
                                                reference.activation_target.activation_id,
                                                reference.activation_target.nanodevice_id,
                                                reference.activation_target.payload_id,
                                                reference.source_request_id,
                                                reference.source_network_id,
                                                reference.expected_activation_offset};
    static_cast<void>(
        model.evaluate({reference.request_id, reference.observation_after_activation, activation}));

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Drug-delivery sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid, "Drug-delivery limitations must not be empty");
        }
    }
}

DrugDeliveryProfile load_drug_delivery_profile(const DrugDeliveryProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "drug-delivery schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "drug-delivery profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_drug_delivery_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Drug-delivery profile validation failed: " + std::string{error.what()});
    }
}

DrugDeliveryRequest
make_drug_delivery_reference_request(const DrugDeliveryProfile& profile,
                                     const NanodeviceActivationSignal& activation) {
    validate_drug_delivery_profile(profile);
    validate_nanodevice_activation_signal(activation);
    const auto& reference = profile.reference_case;
    if (activation.activation_id != reference.activation_target.activation_id ||
        activation.nanodevice_id != reference.activation_target.nanodevice_id ||
        activation.payload_id != reference.activation_target.payload_id ||
        activation.source_request_id != reference.source_request_id ||
        activation.source_network_id != reference.source_network_id ||
        std::abs(std::chrono::duration<double>{activation.trigger_offset -
                                               reference.expected_activation_offset}
                     .count()) > reference.activation_time_tolerance_seconds) {
        invalid(core::ErrorCode::data_invalid,
                "Activation signal is incompatible with the drug-delivery reference");
    }
    return {reference.request_id, reference.observation_after_activation, activation};
}

} // namespace mehlissa::models::cell

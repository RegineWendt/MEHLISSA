// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/apoptosis_response_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

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

[[nodiscard]] CellState decode_state(const std::string_view state) {
    if (state == "viable") {
        return CellState::viable;
    }
    if (state == "apoptosis_committed") {
        return CellState::apoptosis_committed;
    }
    invalid(core::ErrorCode::data_invalid, "Unknown cell state in apoptosis profile");
}

[[nodiscard]] ApoptosisResponseProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& model = document.at("model");
    const auto& feedback = document.at("higher_layer_feedback");
    const auto& reference = document.at("reference_case");
    const auto& expected = reference.at("expected");
    const auto& validity = document.at("validity");
    ApoptosisResponseProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("implementation_kind").as<std::string>(),
        {model.at("model_id").as<std::string>(), model.at("cell_id").as<std::string>(),
         model.at("drug_id").as<std::string>(),
         core::moles(model.at("half_max_effect_amount_mol").as<double>()),
         model.at("hill_coefficient").as<double>(),
         model.at("apoptosis_commitment_threshold").as<double>()},
        {feedback.at("event_id").as<std::string>(),
         feedback.at("target_model_id").as<std::string>(),
         feedback.at("target_port_id").as<std::string>()},
        {reference.at("request_id").as<std::string>(),
         reference.at("source_delivery_request_id").as<std::string>(),
         reference.at("source_delivery_model_id").as<std::string>(),
         core::moles(expected.at("intracellular_drug_amount_mol").as<double>()),
         expected.at("effect_fraction").as<double>(),
         decode_state(expected.at("state").as<std::string>()),
         core::moles(expected.at("amount_tolerance_mol").as<double>()),
         expected.at("effect_tolerance").as<double>()},
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

[[nodiscard]] double expected_hill_effect(const ApoptosisResponseProfile& profile) noexcept {
    const auto amount = core::in_moles(profile.reference_case.expected_intracellular_drug_amount);
    const auto half_max = core::in_moles(profile.model.half_max_effect_amount);
    const auto log_ratio = profile.model.hill_coefficient * std::log(amount / half_max);
    if (log_ratio >= 0.0) {
        return 1.0 / (1.0 + std::exp(-log_ratio));
    }
    const auto ratio = std::exp(log_ratio);
    return ratio / (1.0 + ratio);
}

} // namespace

void validate_apoptosis_response_profile(const ApoptosisResponseProfile& profile) {
    const auto& reference = profile.reference_case;
    const SyntheticHillApoptosisModel model{profile.model};
    const auto calculated_effect = expected_hill_effect(profile);
    const auto calculated_state = calculated_effect >= profile.model.apoptosis_commitment_threshold
                                      ? CellState::apoptosis_committed
                                      : CellState::viable;
    if (profile.schema_version != apoptosis_response_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != synthetic_hill_apoptosis_kind ||
        profile.feedback_target.event_id.empty() ||
        profile.feedback_target.target_model_id.empty() ||
        profile.feedback_target.target_port_id.empty() || reference.request_id.empty() ||
        reference.source_delivery_request_id.empty() ||
        reference.source_delivery_model_id.empty() ||
        !positive_finite(core::in_moles(reference.expected_intracellular_drug_amount)) ||
        !std::isfinite(reference.expected_effect_fraction) ||
        reference.expected_effect_fraction < 0.0 || reference.expected_effect_fraction > 1.0 ||
        !positive_finite(core::in_moles(reference.amount_tolerance)) ||
        !positive_finite(reference.effect_tolerance) ||
        std::abs(calculated_effect - reference.expected_effect_fraction) >
            reference.effect_tolerance ||
        calculated_state != reference.expected_state || profile.validity.population.empty() ||
        profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Apoptosis-response profile is incomplete or inconsistent");
    }

    static_cast<void>(model.kind());
    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-response sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Apoptosis-response limitations must not be empty");
        }
    }
}

ApoptosisResponseProfile
load_apoptosis_response_profile(const ApoptosisResponseProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "apoptosis-response schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document = read_json(request.profile_path, "apoptosis-response profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_apoptosis_response_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Apoptosis-response profile validation failed: " + std::string{error.what()});
    }
}

ApoptosisResponseRequest make_apoptosis_reference_request(const ApoptosisResponseProfile& profile,
                                                          const DrugDeliveryResponse& delivery) {
    validate_apoptosis_response_profile(profile);
    const auto& reference = profile.reference_case;
    if (delivery.request_id != reference.source_delivery_request_id ||
        delivery.model_id != reference.source_delivery_model_id ||
        delivery.drug_id != profile.model.drug_id || !delivery.activated ||
        std::abs(core::in_moles(delivery.intracellular_drug_amount -
                                reference.expected_intracellular_drug_amount)) >
            core::in_moles(reference.amount_tolerance)) {
        invalid(core::ErrorCode::data_invalid,
                "Drug-delivery response is incompatible with the apoptosis reference");
    }
    return {reference.request_id, delivery};
}

} // namespace mehlissa::models::cell

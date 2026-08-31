// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/receptor_ligand_profile.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/cell/analytical_receptor_ligand_model.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::cell {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

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

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::schema_invalid,
                "Invalid receptor-ligand schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Receptor-ligand profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] ReceptorLigandProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    ReceptorLigandProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        {implementation.at("model_id").as<std::string>(),
         implementation.at("receptor_id").as<std::string>(),
         implementation.at("ligand_id").as<std::string>(),
         implementation.at("compartment_id").as<std::string>(),
         core::cubic_meters(implementation.at("cell_volume_m3").as<double>()),
         core::moles_per_cubic_meter(
             implementation.at("total_receptor_concentration_mol_m3").as<double>()),
         core::cubic_meters_per_mole_second(
             implementation.at("association_rate_m3_mol_s").as<double>()),
         core::per_second(implementation.at("dissociation_rate_s_1").as<double>()),
         implementation.at("detection_threshold_fraction").as<double>()},
        {reference.at("request_id").as<std::string>(),
         core::moles_per_cubic_meter(reference.at("ligand_concentration_mol_m3").as<double>()),
         std::chrono::duration_cast<core::SimulationClock::Duration>(
             std::chrono::duration<double>{reference.at("observation_time_seconds").as<double>()}),
         reference.at("initial_bound_fraction").as<double>(),
         reference.at("expected_final_bound_fraction").as<double>(),
         reference.at("expected_threshold_crossing_seconds").as<double>(),
         reference.at("absolute_tolerance").as<double>()},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
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

[[nodiscard]] bool finite_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

void validate_receptor_ligand_profile(const ReceptorLigandProfile& profile) {
    const auto& model = profile.model;
    const auto& reference = profile.reference_case;
    if (profile.schema_version != receptor_ligand_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != analytical_receptor_ligand_kind || model.model_id.empty() ||
        model.receptor_id.empty() || model.ligand_id.empty() || model.compartment_id.empty() ||
        !std::isfinite(core::in_cubic_meters(model.cell_volume)) ||
        core::in_cubic_meters(model.cell_volume) <= 0.0 ||
        !std::isfinite(core::in_moles_per_cubic_meter(model.total_receptor_concentration)) ||
        core::in_moles_per_cubic_meter(model.total_receptor_concentration) <= 0.0 ||
        !std::isfinite(core::in_cubic_meters_per_mole_second(model.association_rate)) ||
        core::in_cubic_meters_per_mole_second(model.association_rate) <= 0.0 ||
        !std::isfinite(core::in_per_second(model.dissociation_rate)) ||
        core::in_per_second(model.dissociation_rate) < 0.0 ||
        !finite_fraction(model.detection_threshold_fraction) ||
        model.detection_threshold_fraction <= 0.0 || reference.request_id.empty() ||
        !std::isfinite(core::in_moles_per_cubic_meter(reference.ligand_concentration)) ||
        core::in_moles_per_cubic_meter(reference.ligand_concentration) < 0.0 ||
        reference.observation_time <= core::SimulationClock::Duration::zero() ||
        !finite_fraction(reference.initial_bound_fraction) ||
        !finite_fraction(reference.expected_final_bound_fraction) ||
        !std::isfinite(reference.expected_threshold_crossing_seconds) ||
        reference.expected_threshold_crossing_seconds < 0.0 ||
        !std::isfinite(reference.absolute_tolerance) || reference.absolute_tolerance <= 0.0 ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Receptor-ligand profile metadata or numerical configuration is invalid");
    }
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid, "Receptor-ligand evidence class is unsupported");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Receptor-ligand source metadata must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid, "Receptor-ligand limitations must not be empty");
        }
    }

    static_cast<void>(AnalyticalReceptorLigandModel{model});
}

ReceptorLigandProfile
load_receptor_ligand_profile(const ReceptorLigandProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "receptor-ligand schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "receptor-ligand profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_receptor_ligand_profile(profile);
    return profile;
}

std::unique_ptr<ReceptorLigandModel>
make_receptor_ligand_model(const ReceptorLigandProfile& profile) {
    validate_receptor_ligand_profile(profile);
    return std::make_unique<AnalyticalReceptorLigandModel>(profile.model);
}

ReceptorLigandRequest make_receptor_ligand_reference_request(const ReceptorLigandProfile& profile) {
    validate_receptor_ligand_profile(profile);
    return {
        profile.reference_case.request_id,
        profile.model.ligand_id,
        profile.model.compartment_id,
        profile.reference_case.ligand_concentration,
        profile.reference_case.observation_time,
        profile.reference_case.initial_bound_fraction,
    };
}

} // namespace mehlissa::models::cell

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/time_varying_receptor_ligand_profile.hpp>

#include <mehlissa/core/error.hpp>

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
        invalid(core::ErrorCode::schema_invalid, "Invalid time-varying receptor-ligand schema '" +
                                                     path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Time-varying receptor-ligand profile does not satisfy its schema '" +
                    path.string() + "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration seconds(const double value) {
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] ReceptorLigandModelConfig decode_binding(const Json& implementation) {
    return {
        implementation.at("model_id").as<std::string>(),
        implementation.at("receptor_id").as<std::string>(),
        implementation.at("ligand_id").as<std::string>(),
        implementation.at("compartment_id").as<std::string>(),
        core::cubic_meters(implementation.at("cell_volume_m3").as<double>()),
        core::moles_per_cubic_meter(
            implementation.at("total_receptor_concentration_mol_m3").as<double>()),
        core::cubic_meters_per_mole_second(
            implementation.at("association_rate_m3_mol_s").as<double>()),
        core::per_second(implementation.at("dissociation_rate_s_1").as<double>()),
        implementation.at("detection_threshold_fraction").as<double>(),
    };
}

[[nodiscard]] TimeVaryingReceptorLigandReferenceCase decode_reference(const Json& reference,
                                                                      const Json& implementation) {
    TimeVaryingReceptorLigandReferenceCase result{
        reference.at("role").as<std::string>(),
        {reference.at("request_id").as<std::string>(),
         implementation.at("ligand_id").as<std::string>(),
         implementation.at("compartment_id").as<std::string>(),
         seconds(reference.at("observation_time_seconds").as<double>()),
         reference.at("initial_bound_fraction").as<double>(),
         {}},
        reference.at("expected_final_bound_fraction").as<double>(),
        reference.at("expected_peak_bound_fraction").as<double>(),
        reference.at("expected_threshold_crossing_seconds").as<double>(),
        reference.at("fraction_absolute_tolerance").as<double>(),
        reference.at("time_absolute_tolerance_seconds").as<double>(),
    };
    for (const auto& knot : reference.at("ligand_trajectory").array_range()) {
        result.request.ligand_trajectory.push_back(
            {seconds(knot.at("offset_seconds").as<double>()),
             core::moles_per_cubic_meter(knot.at("concentration_mol_m3").as<double>())});
    }
    return result;
}

[[nodiscard]] TimeVaryingReceptorLigandProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& solver = document.at("solver");
    const auto& validity = document.at("validity");
    TimeVaryingReceptorLigandProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        {decode_binding(implementation),
         seconds(solver.at("integration_step_seconds").as<double>()),
         solver.at("maximum_integration_steps").as<std::size_t>()},
        {},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& reference : document.at("reference_cases").array_range()) {
        result.reference_cases.push_back(decode_reference(reference, implementation));
    }
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

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

void validate_time_varying_receptor_ligand_profile(
    const TimeVaryingReceptorLigandProfile& profile) {
    if (profile.schema_version != time_varying_receptor_ligand_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != time_varying_receptor_ligand_kind ||
        profile.reference_cases.size() < 2 || profile.validity.population.empty() ||
        profile.validity.physiological_state.empty() || profile.validity.description.empty() ||
        profile.sources.empty() || profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Time-varying receptor-ligand profile metadata is incomplete");
    }
    validate_time_varying_receptor_ligand_config(profile.model);
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid,
                "Time-varying receptor-ligand evidence class is unsupported");
    }

    std::unordered_set<std::string> request_ids;
    std::unordered_set<std::string> roles;
    const TimeVaryingReceptorLigandModel model{profile.model};
    for (const auto& reference : profile.reference_cases) {
        if ((reference.role != constant_analytical_limit_role &&
             reference.role != piecewise_analytical_pulse_role) ||
            !request_ids.insert(reference.request.request_id).second ||
            !roles.insert(reference.role).second ||
            !valid_fraction(reference.expected_final_bound_fraction) ||
            !valid_fraction(reference.expected_peak_bound_fraction) ||
            reference.expected_peak_bound_fraction < reference.expected_final_bound_fraction ||
            !std::isfinite(reference.expected_threshold_crossing_seconds) ||
            reference.expected_threshold_crossing_seconds < 0.0 ||
            !std::isfinite(reference.fraction_absolute_tolerance) ||
            reference.fraction_absolute_tolerance <= 0.0 ||
            !std::isfinite(reference.time_absolute_tolerance_seconds) ||
            reference.time_absolute_tolerance_seconds <= 0.0) {
            invalid(core::ErrorCode::data_invalid,
                    "Time-varying reference cases must be complete, unique, and physical");
        }
        static_cast<void>(model.evaluate(reference.request));
    }
    if (!roles.contains(constant_analytical_limit_role) ||
        !roles.contains(piecewise_analytical_pulse_role)) {
        invalid(core::ErrorCode::data_invalid,
                "Time-varying profile requires constant-limit and pulse references");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Time-varying receptor-ligand sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Time-varying receptor-ligand limitations must not be empty");
        }
    }
}

TimeVaryingReceptorLigandProfile load_time_varying_receptor_ligand_profile(
    const TimeVaryingReceptorLigandProfileLoadRequest& request) {
    const auto schema_document =
        read_json(request.schema_path, "time-varying receptor-ligand schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document =
        read_json(request.profile_path, "time-varying receptor-ligand profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_time_varying_receptor_ligand_profile(profile);
    return profile;
}

std::unique_ptr<TimeVaryingReceptorLigandModel>
make_time_varying_receptor_ligand_model(const TimeVaryingReceptorLigandProfile& profile) {
    validate_time_varying_receptor_ligand_profile(profile);
    return std::make_unique<TimeVaryingReceptorLigandModel>(profile.model);
}

} // namespace mehlissa::models::cell

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/capillary_cell_signal_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>

namespace mehlissa::models::cosimulation {
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
                "Invalid capillary-cell signal schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-cell signal profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration seconds(const double value) {
    return std::chrono::duration_cast<core::SimulationClock::Duration>(
        std::chrono::duration<double>{value});
}

[[nodiscard]] CapillaryCellSignalProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& source = document.at("source");
    const auto& target = document.at("target");
    const auto& exposure = document.at("exposure");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    CapillaryCellSignalProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {identity.at("id").as<std::string>(), source.at("model_id").as<std::string>(),
         source.at("compartment_id").as<std::string>(), source.at("signal_id").as<std::string>(),
         core::cubic_meters(source.at("represented_volume_m3").as<double>()),
         target.at("cell_model_id").as<std::string>(), target.at("ligand_id").as<std::string>(),
         target.at("compartment_id").as<std::string>(),
         seconds(exposure.at("duration_seconds").as<double>()),
         exposure.at("initial_bound_fraction").as<double>()},
        {reference.at("sample_id").as<std::string>(),
         seconds(reference.at("observed_at_seconds").as<double>()),
         core::moles(reference.at("expected_source_amount_mol").as<double>()),
         core::moles_per_cubic_meter(
             reference.at("expected_ligand_concentration_mol_m3").as<double>()),
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
    for (const auto& item : document.at("sources").array_range()) {
        result.sources.push_back(
            {item.at("id").as<std::string>(), item.at("citation").as<std::string>(),
             item.at("location").as<std::string>(), item.at("license").as<std::string>(),
             item.at("role").as<std::string>()});
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

void validate_capillary_cell_signal_profile(const CapillaryCellSignalProfile& profile) {
    const auto& reference = profile.reference_case;
    if (profile.schema_version != capillary_cell_signal_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.coupler.profile_id != profile.profile_id || reference.sample_id.empty() ||
        reference.observed_at < core::SimulationClock::Duration::zero() ||
        !std::isfinite(core::in_moles(reference.expected_source_amount)) ||
        core::in_moles(reference.expected_source_amount) < 0.0 ||
        !std::isfinite(core::in_moles_per_cubic_meter(reference.expected_ligand_concentration)) ||
        core::in_moles_per_cubic_meter(reference.expected_ligand_concentration) < 0.0 ||
        !valid_fraction(reference.expected_final_bound_fraction) ||
        !std::isfinite(reference.expected_threshold_crossing_seconds) ||
        reference.expected_threshold_crossing_seconds < 0.0 ||
        !std::isfinite(reference.absolute_tolerance) || reference.absolute_tolerance <= 0.0 ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-cell signal profile metadata or reference case is invalid");
    }
    validate_capillary_cell_signal_coupler_config(profile.coupler);
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-cell signal evidence class is unsupported");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-cell signal sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-cell signal limitations must not be empty");
        }
    }
}

CapillaryCellSignalProfile
load_capillary_cell_signal_profile(const CapillaryCellSignalProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "capillary-cell signal schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "capillary-cell signal profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_capillary_cell_signal_profile(profile);
    return profile;
}

CapillaryCellSignalCoupler
make_capillary_cell_signal_coupler(const CapillaryCellSignalProfile& profile) {
    validate_capillary_cell_signal_profile(profile);
    return CapillaryCellSignalCoupler{profile.coupler};
}

} // namespace mehlissa::models::cosimulation

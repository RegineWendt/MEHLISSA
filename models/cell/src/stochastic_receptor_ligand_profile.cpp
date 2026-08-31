// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/stochastic_receptor_ligand_profile.hpp>

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

[[nodiscard]] StochasticReceptorLigandRequest decode_request(const Json& input,
                                                             const Json& implementation) {
    StochasticReceptorLigandRequest result{
        input.at("request_id").as<std::string>(),
        implementation.at("ligand_id").as<std::string>(),
        implementation.at("compartment_id").as<std::string>(),
        duration_seconds(input.at("observation_time_seconds").as<double>()),
        input.at("initial_bound_receptors").as<std::uint32_t>(),
        {}};
    for (const auto& knot : input.at("ligand_trajectory").array_range()) {
        result.ligand_trajectory.push_back(
            {duration_seconds(knot.at("offset_seconds").as<double>()),
             core::moles_per_cubic_meter(knot.at("concentration_mol_m3").as<double>())});
    }
    return result;
}

[[nodiscard]] StochasticReceptorLigandProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& solver = document.at("solver");
    const auto& experiment = document.at("population_experiment");
    const auto& gates = experiment.at("acceptance_gates");
    const auto& validity = document.at("validity");
    StochasticReceptorLigandProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        {implementation.at("model_id").as<std::string>(),
         implementation.at("receptor_id").as<std::string>(),
         implementation.at("ligand_id").as<std::string>(),
         implementation.at("compartment_id").as<std::string>(),
         implementation.at("receptor_count").as<std::uint32_t>(),
         core::cubic_meters_per_mole_second(
             implementation.at("association_rate_m3_mol_s").as<double>()),
         core::per_second(implementation.at("dissociation_rate_s_1").as<double>()),
         implementation.at("detection_threshold_fraction").as<double>(),
         solver.at("maximum_reaction_events").as<std::size_t>(),
         solver.at("maximum_recorded_samples").as<std::size_t>()},
        {{experiment.at("master_seed").as<std::uint64_t>(),
          experiment.at("stream_prefix").as<std::string>(),
          experiment.at("cells_per_cohort").as<std::size_t>()},
         {decode_request(experiment.at("signal_positive"), implementation),
          decode_request(experiment.at("signal_negative"), implementation)},
         gates.at("expected_positive_mean").as<double>(),
         gates.at("expected_negative_mean").as<double>(),
         gates.at("expected_positive_variance").as<double>(),
         gates.at("expected_negative_variance").as<double>(),
         gates.at("mean_absolute_tolerance").as<double>(),
         gates.at("variance_absolute_tolerance").as<double>(),
         gates.at("maximum_false_negative_rate").as<double>(),
         gates.at("maximum_false_positive_rate").as<double>()},
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

void validate_stochastic_receptor_ligand_profile(const StochasticReceptorLigandProfile& profile) {
    if (profile.schema_version != stochastic_receptor_ligand_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != stochastic_receptor_ligand_kind ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Stochastic receptor-ligand profile metadata is incomplete");
    }
    validate_stochastic_receptor_ligand_config(profile.model);
    const auto& reference = profile.reference;
    if (reference.ensemble.stream_prefix.empty() || reference.ensemble.cells_per_cohort < 2 ||
        !positive_finite(reference.expected_positive_mean) ||
        !positive_finite(reference.expected_negative_mean) ||
        !positive_finite(reference.expected_positive_variance) ||
        !positive_finite(reference.expected_negative_variance) ||
        !positive_finite(reference.mean_absolute_tolerance) ||
        !positive_finite(reference.variance_absolute_tolerance) ||
        reference.maximum_false_negative_rate < 0.0 ||
        reference.maximum_false_negative_rate > 1.0 ||
        reference.maximum_false_positive_rate < 0.0 ||
        reference.maximum_false_positive_rate > 1.0) {
        invalid(core::ErrorCode::data_invalid,
                "Stochastic population reference or acceptance gates are invalid");
    }
    StochasticReceptorLigandModel model{profile.model};
    core::RandomStream positive{reference.ensemble.master_seed,
                                reference.ensemble.stream_prefix + ".validation.positive"};
    core::RandomStream negative{reference.ensemble.master_seed,
                                reference.ensemble.stream_prefix + ".validation.negative"};
    static_cast<void>(model.evaluate(reference.request.signal_positive, positive));
    static_cast<void>(model.evaluate(reference.request.signal_negative, negative));

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Stochastic receptor-ligand sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Stochastic receptor-ligand limitations must not be empty");
        }
    }
}

StochasticReceptorLigandProfile
load_stochastic_receptor_ligand_profile(const StochasticReceptorLigandProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "stochastic receptor schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "stochastic receptor profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_stochastic_receptor_ligand_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Stochastic receptor-ligand profile validation failed: " +
                    std::string{error.what()});
    }
}

} // namespace mehlissa::models::cell

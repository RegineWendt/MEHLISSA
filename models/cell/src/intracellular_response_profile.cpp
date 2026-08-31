// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/intracellular_response_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

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

[[nodiscard]] IntracellularNetworkKinetics decode_kinetics(const Json& network) {
    return {network.at("network_id").as<std::string>(),
            core::per_second(network.at("messenger_activation_rate_s_1").as<double>()),
            core::per_second(network.at("messenger_deactivation_rate_s_1").as<double>()),
            core::per_second(network.at("effector_activation_rate_s_1").as<double>()),
            core::per_second(network.at("effector_deactivation_rate_s_1").as<double>()),
            network.at("response_threshold_fraction").as<double>()};
}

[[nodiscard]] IntracellularNetworkRequest decode_request(const Json& reference) {
    IntracellularNetworkRequest request{
        reference.at("request_id").as<std::string>(),
        duration_seconds(reference.at("observation_time_seconds").as<double>()),
        reference.at("initial_active_messenger_fraction").as<double>(),
        reference.at("initial_active_effector_fraction").as<double>(),
        {}};
    for (const auto& knot : reference.at("receptor_trajectory").array_range()) {
        request.receptor_trajectory.push_back(
            {duration_seconds(knot.at("offset_seconds").as<double>()),
             knot.at("bound_fraction").as<double>()});
    }
    return request;
}

[[nodiscard]] IntracellularResponseProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& network = document.at("network");
    const auto kinetics = decode_kinetics(network);
    const auto& ode = document.at("ode_solver");
    const auto& ssa = document.at("ssa_solver");
    const auto& reference = document.at("comparison_reference");
    const auto& gates = reference.at("acceptance_gates");
    const auto& validity = document.at("validity");
    IntracellularResponseProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        {kinetics, duration_seconds(ode.at("integration_step_seconds").as<double>()),
         ode.at("maximum_integration_steps").as<std::size_t>(),
         ode.at("maximum_recorded_samples").as<std::size_t>()},
        {kinetics, ssa.at("messenger_molecule_count").as<std::uint32_t>(),
         ssa.at("effector_molecule_count").as<std::uint32_t>(),
         ssa.at("maximum_reaction_events").as<std::size_t>(),
         ssa.at("maximum_recorded_samples").as<std::size_t>()},
        {decode_request(reference), reference.at("master_seed").as<std::uint64_t>(),
         reference.at("stream_prefix").as<std::string>(),
         reference.at("population_size").as<std::size_t>(),
         gates.at("expected_ode_messenger_fraction").as<double>(),
         gates.at("expected_ode_effector_fraction").as<double>(),
         gates.at("expected_ode_response_seconds").as<double>(),
         gates.at("ode_fraction_tolerance").as<double>(),
         gates.at("ode_time_tolerance_seconds").as<double>(),
         gates.at("maximum_ssa_messenger_mean_error").as<double>(),
         gates.at("maximum_ssa_effector_mean_error").as<double>()},
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

[[nodiscard]] bool valid_fraction(const double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

[[nodiscard]] bool same_kinetics(const IntracellularNetworkKinetics& left,
                                 const IntracellularNetworkKinetics& right) noexcept {
    return left.network_id == right.network_id &&
           left.messenger_activation_rate == right.messenger_activation_rate &&
           left.messenger_deactivation_rate == right.messenger_deactivation_rate &&
           left.effector_activation_rate == right.effector_activation_rate &&
           left.effector_deactivation_rate == right.effector_deactivation_rate &&
           left.response_threshold_fraction == right.response_threshold_fraction;
}

} // namespace

void validate_intracellular_response_profile(const IntracellularResponseProfile& profile) {
    const auto& reference = profile.reference;
    if (profile.schema_version != intracellular_response_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        !same_kinetics(profile.ode.kinetics, profile.ssa.kinetics) ||
        reference.stream_prefix.empty() || reference.population_size < 2 ||
        reference.population_size > 1'000'000 ||
        !valid_fraction(reference.expected_ode_messenger_fraction) ||
        !valid_fraction(reference.expected_ode_effector_fraction) ||
        !positive_finite(reference.expected_ode_response_seconds) ||
        reference.expected_ode_response_seconds >
            std::chrono::duration<double>{reference.request.observation_time}.count() ||
        !positive_finite(reference.ode_fraction_tolerance) ||
        !positive_finite(reference.ode_time_tolerance_seconds) ||
        !positive_finite(reference.maximum_ssa_messenger_mean_error) ||
        !positive_finite(reference.maximum_ssa_effector_mean_error) ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Intracellular response profile is incomplete or inconsistent");
    }
    const IntracellularOdeModel ode{profile.ode};
    const IntracellularSsaModel ssa{profile.ssa};
    static_cast<void>(ode.evaluate(reference.request));
    core::RandomStream validation{reference.master_seed, reference.stream_prefix + ".validation"};
    static_cast<void>(ssa.evaluate(reference.request, validation));

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Intracellular response sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Intracellular response limitations must not be empty");
        }
    }
}

IntracellularResponseProfile
load_intracellular_response_profile(const IntracellularResponseProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "intracellular response schema");
    try {
        const auto schema = jsoncons::jsonschema::make_json_schema(schema_document);
        const auto profile_document =
            read_json(request.profile_path, "intracellular response profile");
        schema.validate(profile_document);
        auto profile = decode(profile_document);
        validate_intracellular_response_profile(profile);
        return profile;
    } catch (const core::MehlissaError&) {
        throw;
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Intracellular response profile validation failed: " + std::string{error.what()});
    }
}

IntracellularPopulationComparison
compare_intracellular_ode_ssa(const IntracellularResponseProfile& profile) {
    validate_intracellular_response_profile(profile);
    const IntracellularOdeModel ode{profile.ode};
    const IntracellularSsaModel ssa{profile.ssa};
    const auto deterministic = ode.evaluate(profile.reference.request);
    std::vector<double> messenger;
    std::vector<double> effector;
    messenger.reserve(profile.reference.population_size);
    effector.reserve(profile.reference.population_size);
    std::size_t responding = 0;
    std::size_t events = 0;
    std::uint64_t draws = 0;
    for (std::size_t index = 0; index < profile.reference.population_size; ++index) {
        core::RandomStream random{profile.reference.master_seed, profile.reference.stream_prefix +
                                                                     ".cell." +
                                                                     std::to_string(index)};
        const auto response = ssa.evaluate(profile.reference.request, random);
        messenger.push_back(response.final_active_messenger_fraction);
        effector.push_back(response.final_active_effector_fraction);
        responding += response.response_threshold_reached ? 1U : 0U;
        events += response.reaction_events;
        draws += response.random_draws;
    }
    auto moments = [](const std::vector<double>& values) {
        double mean = 0.0;
        for (const auto value : values) {
            mean += value;
        }
        mean /= static_cast<double>(values.size());
        double variance = 0.0;
        for (const auto value : values) {
            const auto difference = value - mean;
            variance += difference * difference;
        }
        return std::pair{mean, variance / static_cast<double>(values.size())};
    };
    const auto messenger_moments = moments(messenger);
    const auto effector_moments = moments(effector);
    return {deterministic,
            profile.reference.population_size,
            messenger_moments.first,
            messenger_moments.second,
            effector_moments.first,
            effector_moments.second,
            responding,
            events,
            draws};
}

} // namespace mehlissa::models::cell

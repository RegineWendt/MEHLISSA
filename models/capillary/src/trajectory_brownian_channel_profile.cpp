// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/trajectory_brownian_channel_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::capillary {
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
                "Invalid Brownian trajectory schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Brownian trajectory profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] BrownianBoundaryConfig decode_boundary(const Json& boundary) {
    if (boundary.at("kind").as<std::string>() == "unbounded") {
        return {};
    }
    const auto& extents = boundary.at("half_extents_m");
    return {BrownianBoundaryKind::reflecting_box,
            {core::meters(extents.at(0).as<double>()), core::meters(extents.at(1).as<double>()),
             core::meters(extents.at(2).as<double>())}};
}

[[nodiscard]] TrajectoryBrownianChannelProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& retention = implementation.at("trajectory_retention");
    const auto& refinement = document.at("refinement");
    const auto& gate = document.at("verification_gate");
    const auto& validity = document.at("validity");
    TrajectoryBrownianChannelProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        document.at("compatible_analytical_profile_id").as<std::string>(),
        {implementation.at("model_id").as<std::string>(),
         core::square_meters_per_second(
             implementation.at("diffusion_coefficient_m2_s").as<double>()),
         core::per_second(implementation.at("first_order_degradation_rate_s_1").as<double>()),
         implementation.at("sample_count").as<std::uint64_t>(),
         refinement.at("refined_step_count").as<std::uint64_t>(),
         implementation.at("experiment_seed").as<std::uint64_t>(),
         implementation.at("random_stream_name").as<std::string>(),
         decode_boundary(implementation.at("boundary")),
         retention.at("particle_count").as<std::uint64_t>(),
         retention.at("maximum_point_count").as<std::size_t>()},
        refinement.at("coarse_step_count").as<std::uint64_t>(),
        {gate.at("minimum_receiver_observation_count").as<std::uint64_t>(),
         gate.at("maximum_absolute_reference_standardized_error").as<double>(),
         gate.at("maximum_relative_fraction_error").as<double>(),
         gate.at("maximum_refinement_standardized_difference").as<double>(),
         gate.at("maximum_relative_mean_squared_displacement_error").as<double>()},
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

[[nodiscard]] bool approximately_equal(const double left, const double right) noexcept {
    constexpr double relative_tolerance = 1.0e-12;
    return std::abs(left - right) <= relative_tolerance * std::max(std::abs(left), std::abs(right));
}

} // namespace

void validate_trajectory_brownian_channel_profile(const TrajectoryBrownianChannelProfile& profile) {
    if (profile.schema_version != trajectory_brownian_channel_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != brownian_trajectory_3d_kind ||
        profile.compatible_analytical_profile_id.empty() || profile.coarse_step_count == 0 ||
        profile.coarse_step_count >= profile.refined_channel.step_count ||
        profile.refined_channel.step_count % profile.coarse_step_count != 0 ||
        profile.verification_gate.minimum_receiver_observation_count == 0 ||
        !std::isfinite(profile.verification_gate.maximum_absolute_reference_standardized_error) ||
        profile.verification_gate.maximum_absolute_reference_standardized_error <= 0.0 ||
        !std::isfinite(profile.verification_gate.maximum_relative_fraction_error) ||
        profile.verification_gate.maximum_relative_fraction_error <= 0.0 ||
        profile.verification_gate.maximum_relative_fraction_error > 1.0 ||
        !std::isfinite(profile.verification_gate.maximum_refinement_standardized_difference) ||
        profile.verification_gate.maximum_refinement_standardized_difference <= 0.0 ||
        !std::isfinite(
            profile.verification_gate.maximum_relative_mean_squared_displacement_error) ||
        profile.verification_gate.maximum_relative_mean_squared_displacement_error <= 0.0 ||
        profile.verification_gate.maximum_relative_mean_squared_displacement_error > 1.0 ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Brownian trajectory profile metadata, refinement, or gate is invalid");
    }
    if (profile.verification_gate.minimum_receiver_observation_count >=
        profile.refined_channel.sample_count) {
        invalid(core::ErrorCode::data_invalid,
                "Brownian trajectory minimum observation count must be below sample count");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Brownian trajectory sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Brownian trajectory limitations must not be empty");
        }
    }

    static_cast<void>(TrajectoryBrownianChannel{profile.refined_channel});
    auto coarse_config = profile.refined_channel;
    coarse_config.model_id += ".coarse";
    coarse_config.step_count = profile.coarse_step_count;
    static_cast<void>(TrajectoryBrownianChannel{std::move(coarse_config)});
}

TrajectoryBrownianChannelProfile load_trajectory_brownian_channel_profile(
    const TrajectoryBrownianChannelProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "Brownian trajectory schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "Brownian trajectory profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_trajectory_brownian_channel_profile(profile);
    return profile;
}

TrajectoryBrownianChannel
make_trajectory_brownian_channel(const TrajectoryBrownianChannelProfile& trajectory_profile,
                                 const MolecularChannelProfile& analytical_profile,
                                 const bool coarse_resolution) {
    validate_trajectory_brownian_channel_profile(trajectory_profile);
    validate_molecular_channel_profile(analytical_profile);
    if (trajectory_profile.compatible_analytical_profile_id != analytical_profile.profile_id ||
        !approximately_equal(
            core::in_square_meters_per_second(
                trajectory_profile.refined_channel.diffusion_coefficient),
            core::in_square_meters_per_second(analytical_profile.channel.diffusion_coefficient)) ||
        !approximately_equal(
            core::in_per_second(trajectory_profile.refined_channel.first_order_degradation_rate),
            core::in_per_second(analytical_profile.channel.first_order_degradation_rate))) {
        invalid(core::ErrorCode::data_invalid,
                "Trajectory and analytical channel profiles do not describe the same propagation "
                "case");
    }

    auto config = trajectory_profile.refined_channel;
    if (coarse_resolution) {
        config.model_id += ".coarse";
        config.step_count = trajectory_profile.coarse_step_count;
    }
    return TrajectoryBrownianChannel{std::move(config)};
}

} // namespace mehlissa::models::capillary

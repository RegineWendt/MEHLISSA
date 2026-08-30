// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/radial_finite_volume_channel_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
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
                "Invalid radial finite-volume schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Radial finite-volume profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] RadialFiniteVolumeChannelProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& refinement = document.at("refinement");
    const auto& gate = document.at("verification_gate");
    const auto& validity = document.at("validity");
    RadialFiniteVolumeChannelProfile result{
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
         refinement.at("refined_radial_cell_count").as<std::uint64_t>(),
         core::meters(implementation.at("radial_domain_radius_m").as<double>()),
         implementation.at("cfl_safety_factor").as<double>()},
        refinement.at("coarse_radial_cell_count").as<std::uint64_t>(),
        {gate.at("maximum_coarse_relative_reference_error").as<double>(),
         gate.at("maximum_refined_relative_reference_error").as<double>(),
         gate.at("maximum_relative_refinement_difference").as<double>(),
         gate.at("maximum_absolute_conservation_residual").as<double>(),
         gate.at("maximum_escaped_amount_fraction").as<double>()},
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

void validate_radial_finite_volume_channel_profile(
    const RadialFiniteVolumeChannelProfile& profile) {
    if (profile.schema_version != radial_finite_volume_channel_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != radial_finite_volume_diffusion_3d_kind ||
        profile.compatible_analytical_profile_id.empty() || profile.coarse_radial_cell_count < 16 ||
        profile.coarse_radial_cell_count >= profile.refined_channel.radial_cell_count ||
        profile.refined_channel.radial_cell_count % profile.coarse_radial_cell_count != 0 ||
        !std::isfinite(profile.verification_gate.maximum_coarse_relative_reference_error) ||
        profile.verification_gate.maximum_coarse_relative_reference_error <= 0.0 ||
        profile.verification_gate.maximum_coarse_relative_reference_error > 1.0 ||
        !std::isfinite(profile.verification_gate.maximum_refined_relative_reference_error) ||
        profile.verification_gate.maximum_refined_relative_reference_error <= 0.0 ||
        profile.verification_gate.maximum_refined_relative_reference_error >
            profile.verification_gate.maximum_coarse_relative_reference_error ||
        !std::isfinite(profile.verification_gate.maximum_relative_refinement_difference) ||
        profile.verification_gate.maximum_relative_refinement_difference <= 0.0 ||
        profile.verification_gate.maximum_relative_refinement_difference > 1.0 ||
        !std::isfinite(profile.verification_gate.maximum_absolute_conservation_residual) ||
        profile.verification_gate.maximum_absolute_conservation_residual <= 0.0 ||
        !std::isfinite(profile.verification_gate.maximum_escaped_amount_fraction) ||
        profile.verification_gate.maximum_escaped_amount_fraction < 0.0 ||
        profile.verification_gate.maximum_escaped_amount_fraction > 1.0 ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Radial finite-volume profile metadata, refinement, or gate is invalid");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Radial finite-volume sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Radial finite-volume limitations must not be empty");
        }
    }

    static_cast<void>(RadialFiniteVolumeChannel{profile.refined_channel});
    auto coarse_config = profile.refined_channel;
    coarse_config.model_id += ".coarse";
    coarse_config.radial_cell_count = profile.coarse_radial_cell_count;
    static_cast<void>(RadialFiniteVolumeChannel{std::move(coarse_config)});
}

RadialFiniteVolumeChannelProfile load_radial_finite_volume_channel_profile(
    const RadialFiniteVolumeChannelProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "radial finite-volume schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "radial finite-volume profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_radial_finite_volume_channel_profile(profile);
    return profile;
}

RadialFiniteVolumeChannel
make_radial_finite_volume_channel(const RadialFiniteVolumeChannelProfile& field_profile,
                                  const MolecularChannelProfile& analytical_profile,
                                  const bool coarse_resolution) {
    validate_radial_finite_volume_channel_profile(field_profile);
    validate_molecular_channel_profile(analytical_profile);
    if (field_profile.compatible_analytical_profile_id != analytical_profile.profile_id ||
        !approximately_equal(
            core::in_square_meters_per_second(field_profile.refined_channel.diffusion_coefficient),
            core::in_square_meters_per_second(analytical_profile.channel.diffusion_coefficient)) ||
        !approximately_equal(
            core::in_per_second(field_profile.refined_channel.first_order_degradation_rate),
            core::in_per_second(analytical_profile.channel.first_order_degradation_rate))) {
        invalid(core::ErrorCode::data_invalid,
                "Radial finite-volume and analytical profiles do not describe the same propagation "
                "case");
    }

    auto config = field_profile.refined_channel;
    if (coarse_resolution) {
        config.model_id += ".coarse";
        config.radial_cell_count = field_profile.coarse_radial_cell_count;
    }
    return RadialFiniteVolumeChannel{std::move(config)};
}

} // namespace mehlissa::models::capillary

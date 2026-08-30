// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/axial_advection_reaction_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numbers>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>

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
                "Invalid axial advection-reaction schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Axial advection-reaction profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] AxialAdvectionReactionProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& context = document.at("context");
    const auto& implementation = document.at("implementation");
    const auto& geometry = document.at("geometry");
    const auto& propagation = document.at("propagation");
    const auto& particle = document.at("particle_resolution");
    const auto& field = document.at("field_resolution");
    const auto& gate = document.at("verification_gate");
    const auto& validity = document.at("validity");
    AxialAdvectionReactionProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        context.at("compatible_capillary_definition_id").as<std::string>(),
        context.at("capillary_region_id").as<std::string>(),
        {implementation.at("model_id").as<std::string>(),
         core::meters(geometry.at("domain_length_m").as<double>()),
         core::meters(geometry.at("source_position_m").as<double>()),
         core::meters(geometry.at("receiver_center_m").as<double>()),
         core::meters(geometry.at("receiver_width_m").as<double>()),
         core::meters(geometry.at("lumen_radius_m").as<double>()),
         core::meters_per_second(propagation.at("mean_advection_speed_m_s").as<double>()),
         core::square_meters_per_second(propagation.at("diffusion_coefficient_m2_s").as<double>()),
         core::per_second(propagation.at("bulk_reaction_rate_s_1").as<double>()),
         core::meters_per_second(propagation.at("wall_interaction_velocity_m_s").as<double>()),
         core::seconds(document.at("observation_time_s").as<double>())},
        {particle.at("sample_count").as<std::uint64_t>(),
         particle.at("experiment_seed").as<std::uint64_t>(),
         particle.at("random_stream_name").as<std::string>()},
        field.at("coarse_cell_count").as<std::uint64_t>(),
        {field.at("refined_cell_count").as<std::uint64_t>(),
         field.at("cfl_safety_factor").as<double>()},
        {gate.at("minimum_particle_receiver_count").as<std::uint64_t>(),
         gate.at("maximum_particle_standardized_error").as<double>(),
         gate.at("maximum_coarse_relative_reference_error").as<double>(),
         gate.at("maximum_refined_relative_reference_error").as<double>(),
         gate.at("maximum_relative_refinement_difference").as<double>(),
         gate.at("maximum_relative_active_fraction_error").as<double>(),
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
    return std::abs(left - right) <=
           relative_tolerance * std::max({std::abs(left), std::abs(right), 1.0e-30});
}

void validate_capillary_binding(const AxialAdvectionReactionProfile& profile,
                                const CapillaryBedDefinition& definition) {
    if (definition.definition_id != profile.compatible_capillary_definition_id) {
        invalid(core::ErrorCode::data_invalid,
                "Axial advection-reaction profile targets a different capillary definition");
    }
    const auto region =
        std::ranges::find_if(definition.model.regions, [&](const CapillaryRegion& candidate) {
            return candidate.id == profile.capillary_region_id;
        });
    if (region == definition.model.regions.end() ||
        region->kind != CapillaryRegionKind::capillary) {
        invalid(core::ErrorCode::data_invalid,
                "Axial advection-reaction profile requires its named capillary region");
    }
    const auto radius = core::in_meters(region->diameter) / 2.0;
    const auto single_area = std::numbers::pi * radius * radius;
    const auto expected_speed =
        core::in_cubic_meters_per_second(definition.model.volume_flow_rate) /
        (single_area * static_cast<double>(region->parallel_vessel_count));
    if (!approximately_equal(core::in_meters(profile.channel.lumen_radius), radius) ||
        !approximately_equal(core::in_meters_per_second(profile.channel.mean_advection_speed),
                             expected_speed) ||
        core::in_meters(profile.channel.domain_length) > core::in_meters(region->length)) {
        invalid(core::ErrorCode::data_invalid,
                "Axial advection-reaction geometry or speed does not close against the capillary "
                "definition");
    }
}

} // namespace

void validate_axial_advection_reaction_profile(const AxialAdvectionReactionProfile& profile) {
    const auto positive_finite = [](const double value) noexcept {
        return std::isfinite(value) && value > 0.0;
    };
    const auto& gate = profile.verification_gate;
    if (profile.schema_version != axial_advection_reaction_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != axial_advection_reaction_case_kind ||
        profile.compatible_capillary_definition_id.empty() || profile.capillary_region_id.empty() ||
        profile.particle.sample_count < 10'000 || profile.particle.sample_count > 10'000'000 ||
        profile.particle.random_stream_name.empty() || profile.coarse_field_cell_count < 16 ||
        profile.coarse_field_cell_count > 4'096 || profile.refined_field.cell_count > 4'096 ||
        profile.coarse_field_cell_count >= profile.refined_field.cell_count ||
        profile.refined_field.cell_count % profile.coarse_field_cell_count != 0 ||
        !positive_finite(profile.refined_field.cfl_safety_factor) ||
        profile.refined_field.cfl_safety_factor > 0.5 ||
        gate.minimum_particle_receiver_count == 0 ||
        gate.minimum_particle_receiver_count > profile.particle.sample_count ||
        !positive_finite(gate.maximum_particle_standardized_error) ||
        !positive_finite(gate.maximum_coarse_relative_reference_error) ||
        gate.maximum_coarse_relative_reference_error > 1.0 ||
        !positive_finite(gate.maximum_refined_relative_reference_error) ||
        gate.maximum_refined_relative_reference_error >
            gate.maximum_coarse_relative_reference_error ||
        !positive_finite(gate.maximum_relative_refinement_difference) ||
        gate.maximum_relative_refinement_difference > 1.0 ||
        !positive_finite(gate.maximum_relative_active_fraction_error) ||
        gate.maximum_relative_active_fraction_error > 1.0 ||
        !positive_finite(gate.maximum_absolute_conservation_residual) ||
        !std::isfinite(gate.maximum_escaped_amount_fraction) ||
        gate.maximum_escaped_amount_fraction < 0.0 || gate.maximum_escaped_amount_fraction > 1.0 ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.evidence_class != "software_test_surrogate" ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Axial advection-reaction profile metadata or refinement is invalid");
    }
    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Axial advection-reaction sources must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Axial advection-reaction limitations must not be empty");
        }
    }
    static_cast<void>(evaluate_axial_advection_reaction_reference(profile.channel));
}

AxialAdvectionReactionProfile
load_axial_advection_reaction_profile(const AxialAdvectionReactionProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "axial advection-reaction schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document =
        read_json(request.profile_path, "axial advection-reaction profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_axial_advection_reaction_profile(profile);
    return profile;
}

AxialAdvectionReactionVerificationResult
verify_axial_advection_reaction_profile(const AxialAdvectionReactionProfile& profile,
                                        const CapillaryBedDefinition& definition) {
    validate_axial_advection_reaction_profile(profile);
    validate_capillary_binding(profile, definition);
    const AxialFieldConfig coarse_field{profile.coarse_field_cell_count,
                                        profile.refined_field.cfl_safety_factor};
    return verify_axial_advection_reaction_case(profile.channel, profile.particle, coarse_field,
                                                profile.refined_field, profile.verification_gate);
}

} // namespace mehlissa::models::capillary

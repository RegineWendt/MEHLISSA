// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/molecular_channel_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <memory>
#include <numbers>
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
                "Invalid molecular-channel schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular-channel profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] MolecularChannelProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& implementation = document.at("implementation");
    const auto& reference = document.at("reference_case");
    const auto& validity = document.at("validity");
    MolecularChannelProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        implementation.at("kind").as<std::string>(),
        {implementation.at("model_id").as<std::string>(),
         core::square_meters_per_second(
             implementation.at("diffusion_coefficient_m2_s").as<double>()),
         core::per_second(implementation.at("first_order_degradation_rate_s_1").as<double>()),
         implementation.at("maximum_receiver_radius_to_separation_ratio").as<double>()},
        {reference.at("request_id").as<std::string>(), reference.at("signal_id").as<std::string>(),
         reference.at("compatible_capillary_definition_id").as<std::string>(),
         reference.at("capillary_region_id").as<std::string>(),
         reference.at("separation_fraction_of_diameter").as<double>(),
         reference.at("receiver_radius_fraction_of_separation").as<double>(),
         core::moles(reference.at("emitted_amount_mol").as<double>()),
         reference.at("observation_time_mode").as<std::string>()},
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

} // namespace

void validate_molecular_channel_profile(const MolecularChannelProfile& profile) {
    const auto diffusion = core::in_square_meters_per_second(profile.channel.diffusion_coefficient);
    const auto degradation = core::in_per_second(profile.channel.first_order_degradation_rate);
    const auto amount = core::in_moles(profile.reference_case.emitted_amount);
    if (profile.schema_version != molecular_channel_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.implementation_kind != analytical_free_diffusion_3d_kind ||
        profile.channel.model_id.empty() || !std::isfinite(diffusion) || diffusion <= 0.0 ||
        !std::isfinite(degradation) || degradation < 0.0 ||
        !std::isfinite(profile.channel.maximum_receiver_radius_to_separation_ratio) ||
        profile.channel.maximum_receiver_radius_to_separation_ratio <= 0.0 ||
        profile.channel.maximum_receiver_radius_to_separation_ratio > 0.25 ||
        profile.reference_case.request_id.empty() || profile.reference_case.signal_id.empty() ||
        profile.reference_case.compatible_capillary_definition_id.empty() ||
        profile.reference_case.capillary_region_id.empty() ||
        !std::isfinite(profile.reference_case.separation_fraction_of_diameter) ||
        profile.reference_case.separation_fraction_of_diameter <= 0.0 ||
        profile.reference_case.separation_fraction_of_diameter > 1.0 ||
        !std::isfinite(profile.reference_case.receiver_radius_fraction_of_separation) ||
        profile.reference_case.receiver_radius_fraction_of_separation <= 0.0 ||
        profile.reference_case.receiver_radius_fraction_of_separation >
            profile.channel.maximum_receiver_radius_to_separation_ratio ||
        !std::isfinite(amount) || amount <= 0.0 ||
        profile.reference_case.observation_time_mode != "analytical_peak" ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular-channel profile metadata or numerical configuration is invalid");
    }
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid, "Molecular-channel evidence class is unsupported");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Molecular-channel source metadata must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Molecular-channel limitations must not be empty");
        }
    }

    static_cast<void>(AnalyticalDiffusionChannel{profile.channel});
}

MolecularChannelProfile
load_molecular_channel_profile(const MolecularChannelProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "molecular-channel schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "molecular-channel profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_molecular_channel_profile(profile);
    return profile;
}

std::unique_ptr<MolecularChannel> make_molecular_channel(const MolecularChannelProfile& profile) {
    validate_molecular_channel_profile(profile);
    return std::make_unique<AnalyticalDiffusionChannel>(profile.channel);
}

MolecularChannelRequest
make_capillary_reference_request(const MolecularChannelProfile& profile,
                                 const CapillaryBedDefinition& capillary_definition) {
    validate_molecular_channel_profile(profile);
    if (profile.reference_case.compatible_capillary_definition_id !=
        capillary_definition.definition_id) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular-channel profile is incompatible with the capillary definition");
    }

    const CapillaryRegion* selected_region = nullptr;
    for (const auto& region : capillary_definition.model.regions) {
        if (region.id == profile.reference_case.capillary_region_id) {
            selected_region = &region;
            break;
        }
    }
    if (selected_region == nullptr || selected_region->kind != CapillaryRegionKind::capillary) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular-channel reference requires a named capillary region");
    }

    const auto separation =
        selected_region->diameter * profile.reference_case.separation_fraction_of_diameter;
    const auto receiver_radius =
        separation * profile.reference_case.receiver_radius_fraction_of_separation;
    const auto receiver_volume = core::cubic_meters(
        4.0 * std::numbers::pi * std::pow(core::in_meters(receiver_radius), 3.0) / 3.0);
    const AnalyticalDiffusionChannel channel{profile.channel};
    const auto observation_time = channel.peak_observation_time(separation);

    const auto single_area =
        std::numbers::pi * std::pow(core::in_meters(selected_region->diameter), 2.0) / 4.0;
    const auto regional_volume = single_area *
                                 static_cast<double>(selected_region->parallel_vessel_count) *
                                 core::in_meters(selected_region->length);
    const auto transit_seconds = regional_volume / core::in_cubic_meters_per_second(
                                                       capillary_definition.model.volume_flow_rate);
    if (std::chrono::duration<double>{observation_time}.count() > transit_seconds) {
        invalid(core::ErrorCode::data_invalid,
                "Molecular-channel peak lies outside the capillary residence window");
    }

    return {
        profile.reference_case.request_id,
        profile.reference_case.signal_id,
        capillary_definition.model.model_id,
        selected_region->id,
        profile.reference_case.emitted_amount,
        separation,
        receiver_volume,
        observation_time,
    };
}

} // namespace mehlissa::models::capillary

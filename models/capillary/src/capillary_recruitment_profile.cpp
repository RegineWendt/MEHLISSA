// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_recruitment_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
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
                "Invalid capillary-recruitment schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-recruitment profile does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] core::SimulationClock::Duration duration_from_seconds(const double seconds) {
    constexpr double nanoseconds_per_second = 1'000'000'000.0;
    const auto nanoseconds_as_double = seconds * nanoseconds_per_second;
    const auto maximum =
        static_cast<double>(std::numeric_limits<core::SimulationClock::Duration::rep>::max());
    if (!std::isfinite(nanoseconds_as_double) || nanoseconds_as_double < 0.0 ||
        nanoseconds_as_double > maximum) {
        invalid(core::ErrorCode::numeric_overflow,
                "Capillary-recruitment state time is not representable");
    }
    return core::SimulationClock::Duration{
        static_cast<core::SimulationClock::Duration::rep>(std::llround(nanoseconds_as_double))};
}

[[nodiscard]] CapillaryBoundaryCondition decode_boundary_condition(const std::string_view value) {
    if (value == "fixed_total_flow") {
        return CapillaryBoundaryCondition::fixed_total_flow;
    }
    if (value == "fixed_pressure_drop") {
        return CapillaryBoundaryCondition::fixed_pressure_drop;
    }
    invalid(core::ErrorCode::data_invalid, "Unsupported capillary-recruitment boundary condition");
}

[[nodiscard]] CapillaryRecruitmentProfile decode(const Json& document) {
    const auto& identity = document.at("profile");
    const auto& validity = document.at("validity");
    CapillaryRecruitmentProfile result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        document.at("compatible_model_id").as<std::string>(),
        decode_boundary_condition(document.at("boundary_condition").as<std::string_view>()),
        {},
        {},
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& group : document.at("sphincter_groups").array_range()) {
        result.sphincter_groups.push_back(
            {group.at("id").as<std::string>(), group.at("path_count").as<std::uint64_t>()});
    }
    for (const auto& state : document.at("states").array_range()) {
        CapillaryRecruitmentState decoded_state{
            state.at("id").as<std::string>(),
            duration_from_seconds(state.at("effective_at_s").as<double>()),
            {},
        };
        for (const auto& group_id : state.at("open_sphincter_group_ids").array_range()) {
            decoded_state.open_sphincter_group_ids.push_back(group_id.as<std::string>());
        }
        result.states.push_back(std::move(decoded_state));
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

} // namespace

std::string_view to_string(const CapillaryBoundaryCondition condition) noexcept {
    switch (condition) {
    case CapillaryBoundaryCondition::fixed_total_flow:
        return "fixed_total_flow";
    case CapillaryBoundaryCondition::fixed_pressure_drop:
        return "fixed_pressure_drop";
    }
    return "unknown";
}

void validate_capillary_recruitment_profile(const CapillaryRecruitmentProfile& profile) {
    if (profile.schema_version != capillary_recruitment_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.compatible_model_id.empty() || profile.sphincter_groups.empty() ||
        profile.states.empty() || profile.validity.population.empty() ||
        profile.validity.physiological_state.empty() || profile.validity.evidence_class.empty() ||
        profile.validity.description.empty() || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-recruitment profile metadata is incomplete or invalid");
    }
    if (profile.boundary_condition != CapillaryBoundaryCondition::fixed_total_flow &&
        profile.boundary_condition != CapillaryBoundaryCondition::fixed_pressure_drop) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-recruitment boundary condition is invalid");
    }
    if (profile.validity.evidence_class != "software_test_surrogate" &&
        profile.validity.evidence_class != "literature_parameterized" &&
        profile.validity.evidence_class != "externally_derived") {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-recruitment evidence class is unsupported");
    }

    std::unordered_set<std::string> group_ids;
    for (const auto& group : profile.sphincter_groups) {
        if (group.id.empty() || group.path_count == 0 || !group_ids.insert(group.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Precapillary sphincter groups require unique IDs and positive path counts");
        }
    }

    std::unordered_set<std::string> state_ids;
    auto previous_time = core::SimulationClock::Duration::zero();
    for (std::size_t index{}; index < profile.states.size(); ++index) {
        const auto& state = profile.states[index];
        if (state.id.empty() || !state_ids.insert(state.id).second ||
            state.open_sphincter_group_ids.empty() || state.effective_at.count() < 0 ||
            (index == 0 && state.effective_at != core::SimulationClock::Duration::zero()) ||
            (index > 0 && state.effective_at <= previous_time)) {
            invalid(core::ErrorCode::data_invalid,
                    "Recruitment states require unique IDs, non-empty open groups, and strictly "
                    "increasing times beginning at zero");
        }
        std::unordered_set<std::string> open_groups;
        for (const auto& group_id : state.open_sphincter_group_ids) {
            if (!group_ids.contains(group_id) || !open_groups.insert(group_id).second) {
                invalid(core::ErrorCode::data_invalid,
                        "Recruitment state '" + state.id +
                            "' contains an unknown or duplicate sphincter group");
            }
        }
        previous_time = state.effective_at;
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.location.empty() ||
            source.license.empty() || source.role.empty() || !source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-recruitment source metadata must be complete and unique");
        }
    }
    for (const auto& limitation : profile.limitations) {
        if (limitation.empty()) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-recruitment limitations must not be empty");
        }
    }
}

CapillaryRecruitmentProfile
load_capillary_recruitment_profile(const CapillaryRecruitmentProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "capillary-recruitment schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "capillary-recruitment profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_capillary_recruitment_profile(profile);
    return profile;
}

} // namespace mehlissa::models::capillary

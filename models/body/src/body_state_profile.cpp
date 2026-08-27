// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/body_state_profile.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mehlissa::models::body {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

[[noreturn]] void invalid(const std::string& message) {
    throw VascularGraphError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw VascularGraphError{core::ErrorCode::input_unreadable,
                                 "Cannot open " + std::string{role} + ": " + path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::json_invalid,
                                 "Invalid JSON in " + std::string{role} + " '" + path.string() +
                                     "': " + error.what()};
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::schema_invalid, "Invalid body-state schema '" +
                                                                      path.string() +
                                                                      "': " + error.what()};
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::data_invalid,
                                 "Body-state profile does not satisfy its schema '" +
                                     path.string() + "': " + error.what()};
    }
}

[[nodiscard]] std::vector<Transition> decode_transitions(const Json& document) {
    std::vector<Transition> result;
    result.reserve(document.size());
    for (const auto& transition : document.array_range()) {
        result.push_back({transition.at("successor_id").as<std::string>(),
                          transition.at("probability").as<double>()});
    }
    return result;
}

[[nodiscard]] BodyStateProfile decode(const Json& document) {
    const auto& profile_document = document.at("profile");
    const auto& compatibility = document.at("compatible_model");
    const auto& validity = document.at("validity");
    const auto& cardiac_output = document.at("cardiac_output");

    BodyStateProfile profile{};
    profile.schema_version = document.at("schema_version").as<std::string>();
    profile.profile_id = profile_document.at("id").as<std::string>();
    profile.profile_version = profile_document.at("version").as<std::string>();
    profile.title = profile_document.at("title").as<std::string>();
    profile.compatible_model_id = compatibility.at("id").as<std::string>();
    profile.compatible_model_version = compatibility.at("version").as<std::string>();
    profile.validity = {validity.at("population").as<std::string>(),
                        validity.at("physiological_state").as<std::string>(),
                        validity.at("description").as<std::string>()};
    profile.cardiac_output_anchor_segment_id =
        cardiac_output.at("anchor_segment_id").as<std::string>();
    profile.cardiac_output_multiplier = cardiac_output.at("multiplier").as<double>();

    for (const auto& item : document.at("transition_overrides").array_range()) {
        profile.transition_overrides.push_back(
            {item.at("segment_id").as<std::string>(), decode_transitions(item.at("transitions"))});
    }
    for (const auto& source : document.at("sources").array_range()) {
        profile.sources.push_back({source.at("id").as<std::string>(),
                                   source.at("citation").as<std::string>(),
                                   source.at("license").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        profile.limitations.push_back(limitation.as<std::string>());
    }
    return profile;
}

void validate_profile(const BodyStateProfile& profile) {
    if (profile.schema_version != supported_body_state_profile_schema_version ||
        profile.profile_id.empty() || profile.profile_version.empty() || profile.title.empty() ||
        profile.compatible_model_id.empty() || profile.compatible_model_version.empty() ||
        profile.validity.population.empty() || profile.validity.physiological_state.empty() ||
        profile.validity.description.empty() || profile.cardiac_output_anchor_segment_id.empty() ||
        !std::isfinite(profile.cardiac_output_multiplier) ||
        profile.cardiac_output_multiplier <= 0.0 || profile.sources.empty() ||
        profile.limitations.empty()) {
        invalid("Body-state profile metadata is incomplete or invalid");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : profile.sources) {
        if (source.id.empty() || source.citation.empty() || source.license.empty() ||
            !source_ids.emplace(source.id).second) {
            invalid("Body-state profile source metadata must be complete and unique");
        }
    }
    std::unordered_set<std::string> overridden_segments;
    for (const auto& override : profile.transition_overrides) {
        if (override.segment_id.empty() ||
            !overridden_segments.emplace(override.segment_id).second ||
            override.transitions.empty()) {
            invalid("Body-state transition overrides must have unique segment IDs");
        }
        std::unordered_set<std::string> successors;
        double sum{};
        for (const auto& transition : override.transitions) {
            if (transition.successor_id.empty() ||
                !successors.emplace(transition.successor_id).second ||
                !std::isfinite(transition.probability) || transition.probability <= 0.0 ||
                transition.probability > 1.0) {
                invalid("Body-state transition override is invalid for segment '" +
                        override.segment_id + "'");
            }
            sum += transition.probability;
        }
        if (std::abs(sum - 1.0) > 1.0e-12) {
            invalid("Body-state transition probabilities do not sum to one for segment '" +
                    override.segment_id + "'");
        }
    }
}

[[nodiscard]] std::vector<double> solve_stationary_flows(const VascularGraph& graph,
                                                         const std::size_t anchor_index,
                                                         const double anchor_flow) {
    const auto size = graph.segments.size();
    std::unordered_map<std::string, std::size_t> indices;
    indices.reserve(size);
    for (std::size_t index = 0; index < size; ++index) {
        indices.emplace(graph.segments[index].id, index);
    }

    std::vector<std::vector<double>> matrix(size, std::vector<double>(size + 1));
    for (std::size_t target = 0; target < size; ++target) {
        matrix[target][target] = 1.0;
    }
    for (std::size_t source = 0; source < size; ++source) {
        for (const auto& transition : graph.segments[source].transitions) {
            matrix[indices.at(transition.successor_id)][source] -= transition.probability;
        }
    }
    std::ranges::fill(matrix[anchor_index], 0.0);
    matrix[anchor_index][anchor_index] = 1.0;
    matrix[anchor_index][size] = anchor_flow;

    constexpr double pivot_tolerance = 1.0e-12;
    for (std::size_t column = 0; column < size; ++column) {
        auto pivot = column;
        for (std::size_t row = column + 1; row < size; ++row) {
            if (std::abs(matrix[row][column]) > std::abs(matrix[pivot][column])) {
                pivot = row;
            }
        }
        if (std::abs(matrix[pivot][column]) <= pivot_tolerance) {
            invalid("Body-state flow system is singular; check graph connectivity and anchor");
        }
        if (pivot != column) {
            std::swap(matrix[pivot], matrix[column]);
        }
        const auto divisor = matrix[column][column];
        for (std::size_t entry = column; entry <= size; ++entry) {
            matrix[column][entry] /= divisor;
        }
        for (std::size_t row = 0; row < size; ++row) {
            if (row == column) {
                continue;
            }
            const auto factor = matrix[row][column];
            if (factor == 0.0) {
                continue;
            }
            for (std::size_t entry = column; entry <= size; ++entry) {
                matrix[row][entry] -= factor * matrix[column][entry];
            }
        }
    }

    std::vector<double> result;
    result.reserve(size);
    for (const auto& row : matrix) {
        const auto flow = row[size];
        if (!std::isfinite(flow) || flow <= 0.0) {
            invalid("Body-state flow solution contains a non-positive or non-finite value");
        }
        result.push_back(flow);
    }
    return result;
}

} // namespace

BodyStateProfile load_body_state_profile(const BodyStateProfileLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "body-state schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto profile_document = read_json(request.profile_path, "body-state profile");
    validate_document(profile_document, schema, request.profile_path);
    auto profile = decode(profile_document);
    validate_profile(profile);
    return profile;
}

VascularGraph apply_body_state_profile(const VascularGraph& base_graph,
                                       const BodyStateProfile& profile) {
    validate_vascular_graph(base_graph);
    validate_profile(profile);
    if (base_graph.model_id != profile.compatible_model_id ||
        base_graph.model_version != profile.compatible_model_version) {
        invalid("Body-state profile '" + profile.profile_id + "' requires model " +
                profile.compatible_model_id + " version " + profile.compatible_model_version);
    }

    auto result = base_graph;
    std::unordered_map<std::string, std::size_t> indices;
    indices.reserve(result.segments.size());
    for (std::size_t index = 0; index < result.segments.size(); ++index) {
        indices.emplace(result.segments[index].id, index);
    }
    const auto anchor = indices.find(profile.cardiac_output_anchor_segment_id);
    if (anchor == indices.end()) {
        invalid("Body-state cardiac-output anchor is unknown: " +
                profile.cardiac_output_anchor_segment_id);
    }

    for (const auto& override : profile.transition_overrides) {
        const auto segment = indices.find(override.segment_id);
        if (segment == indices.end()) {
            invalid("Body-state override references unknown segment: " + override.segment_id);
        }
        const auto& original = result.segments[segment->second].transitions;
        if (original.size() != override.transitions.size()) {
            invalid("Body-state override changes successor count for segment '" +
                    override.segment_id + "'");
        }
        for (const auto& transition : override.transitions) {
            if (std::ranges::find(original, transition.successor_id, &Transition::successor_id) ==
                original.end()) {
                invalid("Body-state override changes topology for segment '" + override.segment_id +
                        "'");
            }
        }
        result.segments[segment->second].transitions = override.transitions;
    }

    const auto base_anchor_flow = core::in_cubic_meters_per_second(
        base_graph.segments[anchor->second].hemodynamics.flow_rate);
    const auto flows = solve_stationary_flows(result, anchor->second,
                                              base_anchor_flow * profile.cardiac_output_multiplier);

    std::unordered_set<std::string> source_ids;
    for (const auto& source : result.sources) {
        source_ids.emplace(source.id);
    }
    for (const auto& source : profile.sources) {
        if (!source_ids.emplace(source.id).second) {
            invalid("Body-state source duplicates base-model source ID: " + source.id);
        }
        result.sources.push_back(source);
    }

    for (std::size_t index = 0; index < result.segments.size(); ++index) {
        auto& segment = result.segments[index];
        segment.hemodynamics.flow_rate = core::cubic_meters_per_second(flows[index]);
        segment.hemodynamics.mean_velocity = core::meters_per_second(
            flows[index] / core::in_square_meters(segment.geometry.cross_section_area));
        segment.evidence.flow = EvidenceQuality::derived;
        for (const auto& source : profile.sources) {
            segment.source_refs.push_back(source.id);
        }
    }

    result.model_id += "--" + profile.profile_id;
    result.model_version = profile.profile_version;
    result.title += " — " + profile.title;
    result.validity = profile.validity;
    for (const auto& limitation : profile.limitations) {
        result.validity.description += " Limitation: " + limitation;
    }
    validate_vascular_graph(result);
    return result;
}

} // namespace mehlissa::models::body

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/vascular_graph.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numbers>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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
        throw VascularGraphError{core::ErrorCode::schema_invalid,
                                 "Invalid vascular-graph schema '" + path.string() +
                                     "': " + error.what()};
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::data_invalid,
                                 "Vascular graph '" + path.string() +
                                     "' does not satisfy its schema: " + error.what()};
    }
}

[[nodiscard]] core::Position3D decode_position(const Json& values) {
    return {
        core::meters(values.at(0).as<double>()),
        core::meters(values.at(1).as<double>()),
        core::meters(values.at(2).as<double>()),
    };
}

[[nodiscard]] VesselType decode_vessel_type(const std::string_view value) {
    if (value == "artery") {
        return VesselType::artery;
    }
    if (value == "vein") {
        return VesselType::vein;
    }
    return VesselType::organ_bed;
}

[[nodiscard]] EvidenceQuality decode_quality(const std::string_view value) {
    if (value == "measured") {
        return EvidenceQuality::measured;
    }
    if (value == "literature") {
        return EvidenceQuality::literature;
    }
    if (value == "derived") {
        return EvidenceQuality::derived;
    }
    if (value == "schematic") {
        return EvidenceQuality::schematic;
    }
    if (value == "assumed") {
        return EvidenceQuality::assumed;
    }
    return EvidenceQuality::unknown;
}

[[nodiscard]] std::optional<double> optional_number(const Json& object,
                                                    const std::string_view key) {
    if (!object.contains(key)) {
        return std::nullopt;
    }
    return object.at(key).as<double>();
}

[[nodiscard]] VascularGraph decode(const Json& document) {
    VascularGraph graph;
    graph.schema_version = document.at("schema_version").as<std::string>();
    const auto& model = document.at("model");
    graph.model_id = model.at("id").as<std::string>();
    graph.model_version = model.at("version").as<std::string>();
    graph.title = model.at("title").as<std::string>();

    const auto& coordinate_system = document.at("coordinate_system");
    graph.coordinate_system = {
        coordinate_system.at("id").as<std::string>(),
        coordinate_system.at("description").as<std::string>(),
        coordinate_system.at("handedness").as<std::string>(),
    };

    const auto& validity = document.at("validity");
    graph.validity = {
        validity.at("population").as<std::string>(),
        validity.at("physiological_state").as<std::string>(),
        validity.at("description").as<std::string>(),
    };

    for (const auto& source : document.at("sources").array_range()) {
        graph.sources.push_back({
            source.at("id").as<std::string>(),
            source.at("citation").as<std::string>(),
            source.at("license").as<std::string>(),
        });
    }

    for (const auto& segment_document : document.at("segments").array_range()) {
        const auto& geometry = segment_document.at("geometry");
        const auto& hemodynamics = segment_document.at("hemodynamics");
        const auto& evidence = segment_document.at("evidence");
        const auto& uncertainty = segment_document.at("relative_uncertainty");
        VascularSegment segment{
            segment_document.at("id").as<std::string>(),
            decode_vessel_type(segment_document.at("type").as<std::string_view>()),
            {
                decode_position(geometry.at("start_m")),
                decode_position(geometry.at("end_m")),
                core::meters(geometry.at("length_m").as<double>()),
                core::meters(geometry.at("diameter_m").as<double>()),
                core::square_meters(geometry.at("cross_section_area_m2").as<double>()),
                core::cubic_meters(geometry.at("volume_m3").as<double>()),
            },
            {
                core::cubic_meters_per_second(hemodynamics.at("flow_rate_m3_s").as<double>()),
                core::meters_per_second(hemodynamics.at("mean_velocity_m_s").as<double>()),
            },
            {},
            {},
            {
                decode_quality(evidence.at("geometry").as<std::string_view>()),
                decode_quality(evidence.at("diameter").as<std::string_view>()),
                decode_quality(evidence.at("flow").as<std::string_view>()),
            },
            {
                optional_number(uncertainty, "geometry"),
                optional_number(uncertainty, "diameter"),
                optional_number(uncertainty, "flow"),
            },
        };

        for (const auto& transition : segment_document.at("transitions").array_range()) {
            segment.transitions.push_back({
                transition.at("successor_id").as<std::string>(),
                transition.at("probability").as<double>(),
            });
        }
        for (const auto& source_ref : segment_document.at("source_refs").array_range()) {
            segment.source_refs.push_back(source_ref.as<std::string>());
        }
        graph.segments.push_back(std::move(segment));
    }
    return graph;
}

[[nodiscard]] std::string_view encode_vessel_type(const VesselType type) noexcept {
    switch (type) {
    case VesselType::artery:
        return "artery";
    case VesselType::vein:
        return "vein";
    case VesselType::organ_bed:
        return "organ_bed";
    }
    return "organ_bed";
}

[[nodiscard]] std::string_view encode_quality(const EvidenceQuality quality) noexcept {
    switch (quality) {
    case EvidenceQuality::measured:
        return "measured";
    case EvidenceQuality::literature:
        return "literature";
    case EvidenceQuality::derived:
        return "derived";
    case EvidenceQuality::schematic:
        return "schematic";
    case EvidenceQuality::assumed:
        return "assumed";
    case EvidenceQuality::unknown:
        return "unknown";
    }
    return "unknown";
}

[[nodiscard]] Json encode_position(const core::Position3D& position) {
    Json document = Json::array();
    document.push_back(core::in_meters(position.x));
    document.push_back(core::in_meters(position.y));
    document.push_back(core::in_meters(position.z));
    return document;
}

[[nodiscard]] Json encode(const VascularGraph& graph) {
    Json source_documents = Json::array();
    for (const auto& source : graph.sources) {
        source_documents.push_back(Json{jsoncons::json_object_arg,
                                        {
                                            {"id", source.id},
                                            {"citation", source.citation},
                                            {"license", source.license},
                                        }});
    }

    Json segment_documents = Json::array();
    for (const auto& segment : graph.segments) {
        Json transition_documents = Json::array();
        for (const auto& transition : segment.transitions) {
            transition_documents.push_back(Json{jsoncons::json_object_arg,
                                                {
                                                    {"successor_id", transition.successor_id},
                                                    {"probability", transition.probability},
                                                }});
        }
        Json source_refs = Json::array();
        for (const auto& source_ref : segment.source_refs) {
            source_refs.push_back(source_ref);
        }
        Json uncertainty{jsoncons::json_object_arg};
        if (segment.relative_uncertainty.geometry.has_value()) {
            uncertainty["geometry"] = *segment.relative_uncertainty.geometry;
        }
        if (segment.relative_uncertainty.diameter.has_value()) {
            uncertainty["diameter"] = *segment.relative_uncertainty.diameter;
        }
        if (segment.relative_uncertainty.flow.has_value()) {
            uncertainty["flow"] = *segment.relative_uncertainty.flow;
        }

        segment_documents.push_back(Json{
            jsoncons::json_object_arg,
            {
                {"id", segment.id},
                {"type", encode_vessel_type(segment.type)},
                {"geometry", Json{jsoncons::json_object_arg,
                                  {
                                      {"start_m", encode_position(segment.geometry.start)},
                                      {"end_m", encode_position(segment.geometry.end)},
                                      {"length_m", core::in_meters(segment.geometry.length)},
                                      {"diameter_m", core::in_meters(segment.geometry.diameter)},
                                      {"cross_section_area_m2",
                                       core::in_square_meters(segment.geometry.cross_section_area)},
                                      {"volume_m3", core::in_cubic_meters(segment.geometry.volume)},
                                  }}},
                {"hemodynamics",
                 Json{jsoncons::json_object_arg,
                      {
                          {"flow_rate_m3_s",
                           core::in_cubic_meters_per_second(segment.hemodynamics.flow_rate)},
                          {"mean_velocity_m_s",
                           core::in_meters_per_second(segment.hemodynamics.mean_velocity)},
                      }}},
                {"transitions", std::move(transition_documents)},
                {"source_refs", std::move(source_refs)},
                {"evidence", Json{jsoncons::json_object_arg,
                                  {
                                      {"geometry", encode_quality(segment.evidence.geometry)},
                                      {"diameter", encode_quality(segment.evidence.diameter)},
                                      {"flow", encode_quality(segment.evidence.flow)},
                                  }}},
                {"relative_uncertainty", std::move(uncertainty)},
            }});
    }

    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", graph.schema_version},
            {"model", Json{jsoncons::json_object_arg,
                           {
                               {"id", graph.model_id},
                               {"version", graph.model_version},
                               {"title", graph.title},
                           }}},
            {"coordinate_system", Json{jsoncons::json_object_arg,
                                       {
                                           {"id", graph.coordinate_system.id},
                                           {"description", graph.coordinate_system.description},
                                           {"handedness", graph.coordinate_system.handedness},
                                           {"length_unit", "m"},
                                       }}},
            {"validity", Json{jsoncons::json_object_arg,
                              {
                                  {"population", graph.validity.population},
                                  {"physiological_state", graph.validity.physiological_state},
                                  {"description", graph.validity.description},
                              }}},
            {"sources", std::move(source_documents)},
            {"segments", std::move(segment_documents)},
        }};
}

[[nodiscard]] bool approximately_equal(const double left, const double right) noexcept {
    constexpr double absolute_tolerance = 1.0e-12;
    constexpr double relative_tolerance = 1.0e-9;
    return std::abs(left - right) <=
           std::max(absolute_tolerance,
                    relative_tolerance * std::max(std::abs(left), std::abs(right)));
}

[[nodiscard]] bool same_position(const core::Position3D& left,
                                 const core::Position3D& right) noexcept {
    return approximately_equal(core::in_meters(left.x), core::in_meters(right.x)) &&
           approximately_equal(core::in_meters(left.y), core::in_meters(right.y)) &&
           approximately_equal(core::in_meters(left.z), core::in_meters(right.z));
}

void require_finite_positive(const double value, const std::string& field,
                             const std::string& segment_id) {
    if (!std::isfinite(value) || value <= 0.0) {
        invalid("Segment '" + segment_id + "' has invalid " + field);
    }
}

void require_uncertainty(const std::optional<double> value, const std::string_view field,
                         const std::string& segment_id) {
    if (value.has_value() && (!std::isfinite(*value) || *value < 0.0)) {
        invalid("Segment '" + segment_id + "' has invalid relative " + std::string{field} +
                " uncertainty");
    }
}

void validate_graph_connectivity(
    const VascularGraph& graph,
    const std::unordered_map<std::string, std::size_t>& segment_indices) {
    const auto visit = [&](const bool reverse) {
        std::vector<bool> visited(graph.segments.size());
        std::vector<std::size_t> pending{0};
        while (!pending.empty()) {
            const auto index = pending.back();
            pending.pop_back();
            if (visited.at(index)) {
                continue;
            }
            visited.at(index) = true;
            if (!reverse) {
                for (const auto& transition : graph.segments.at(index).transitions) {
                    pending.push_back(segment_indices.at(transition.successor_id));
                }
            } else {
                const auto& current_id = graph.segments.at(index).id;
                for (std::size_t candidate = 0; candidate < graph.segments.size(); ++candidate) {
                    const auto& transitions = graph.segments.at(candidate).transitions;
                    if (std::ranges::any_of(transitions, [&](const Transition& transition) {
                            return transition.successor_id == current_id;
                        })) {
                        pending.push_back(candidate);
                    }
                }
            }
        }
        return std::ranges::all_of(visited, [](const bool reached) { return reached; });
    };

    if (!visit(false) || !visit(true)) {
        invalid("Vascular graph must form one strongly connected closed circulation");
    }
}

void validate_flow_conservation(const VascularGraph& graph) {
    for (const auto& junction : graph.segments) {
        double inflow{};
        double outflow{};
        for (const auto& segment : graph.segments) {
            if (same_position(segment.geometry.end, junction.geometry.start)) {
                inflow += core::in_cubic_meters_per_second(segment.hemodynamics.flow_rate);
            }
            if (same_position(segment.geometry.start, junction.geometry.start)) {
                outflow += core::in_cubic_meters_per_second(segment.hemodynamics.flow_rate);
            }
        }
        if (!approximately_equal(inflow, outflow)) {
            invalid("Flow is not conserved at the start junction of segment '" + junction.id + "'");
        }
    }
}

} // namespace

const VascularSegment* VascularGraph::find_segment(const std::string_view id) const noexcept {
    const auto result = std::ranges::find(segments, id, &VascularSegment::id);
    return result == segments.end() ? nullptr : &*result;
}

void validate_vascular_graph(const VascularGraph& graph) {
    if (graph.schema_version != supported_vascular_graph_schema_version || graph.model_id.empty() ||
        graph.model_version.empty() || graph.title.empty()) {
        invalid("Vascular graph metadata is incomplete or unsupported");
    }
    if (graph.coordinate_system.id.empty() || graph.coordinate_system.description.empty() ||
        graph.coordinate_system.handedness.empty() || graph.validity.population.empty() ||
        graph.validity.physiological_state.empty() || graph.validity.description.empty()) {
        invalid("Vascular graph coordinate-system or validity metadata is incomplete");
    }
    if (graph.segments.empty()) {
        invalid("Vascular graph must contain at least one segment");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : graph.sources) {
        if (source.id.empty() || source.citation.empty() || source.license.empty()) {
            invalid("Vascular data-source metadata must be complete");
        }
        if (!source_ids.insert(source.id).second) {
            invalid("Duplicate vascular data-source ID: " + source.id);
        }
    }

    std::unordered_map<std::string, std::size_t> segment_indices;
    for (std::size_t index = 0; index < graph.segments.size(); ++index) {
        const auto& segment = graph.segments.at(index);
        if (segment.id.empty()) {
            invalid("Vascular segment ID must not be empty");
        }
        if (!segment_indices.emplace(segment.id, index).second) {
            invalid("Duplicate vascular segment ID: " + segment.id);
        }
    }

    for (const auto& segment : graph.segments) {
        const auto length = core::in_meters(segment.geometry.length);
        const auto diameter = core::in_meters(segment.geometry.diameter);
        const auto area = core::in_square_meters(segment.geometry.cross_section_area);
        const auto volume = core::in_cubic_meters(segment.geometry.volume);
        const auto flow = core::in_cubic_meters_per_second(segment.hemodynamics.flow_rate);
        const auto velocity = core::in_meters_per_second(segment.hemodynamics.mean_velocity);
        require_finite_positive(length, "length", segment.id);
        require_finite_positive(diameter, "diameter", segment.id);
        require_finite_positive(area, "cross-sectional area", segment.id);
        require_finite_positive(volume, "volume", segment.id);
        require_finite_positive(flow, "flow rate", segment.id);
        require_finite_positive(velocity, "mean velocity", segment.id);
        require_uncertainty(segment.relative_uncertainty.geometry, "geometry", segment.id);
        require_uncertainty(segment.relative_uncertainty.diameter, "diameter", segment.id);
        require_uncertainty(segment.relative_uncertainty.flow, "flow", segment.id);

        if (!approximately_equal(
                core::in_meters(core::distance(segment.geometry.start, segment.geometry.end)),
                length)) {
            invalid("Segment '" + segment.id + "' length does not match its endpoints");
        }
        const auto expected_area = std::numbers::pi * diameter * diameter / 4.0;
        if (!approximately_equal(area, expected_area)) {
            invalid("Segment '" + segment.id + "' area does not match its diameter");
        }
        if (!approximately_equal(volume, area * length)) {
            invalid("Segment '" + segment.id + "' volume does not match area times length");
        }
        if (!approximately_equal(flow, area * velocity)) {
            invalid("Segment '" + segment.id + "' flow does not match area times velocity");
        }

        std::unordered_set<std::string> successors;
        double probability_sum{};
        double successor_flow_sum{};
        if (segment.transitions.empty()) {
            invalid("Segment '" + segment.id + "' must have at least one successor");
        }
        for (const auto& transition : segment.transitions) {
            if (!std::isfinite(transition.probability) || transition.probability <= 0.0 ||
                transition.probability > 1.0) {
                invalid("Segment '" + segment.id + "' has an invalid transition probability");
            }
            if (!successors.insert(transition.successor_id).second) {
                invalid("Segment '" + segment.id +
                        "' has a duplicate successor: " + transition.successor_id);
            }
            const auto successor = segment_indices.find(transition.successor_id);
            if (successor == segment_indices.end()) {
                invalid("Segment '" + segment.id +
                        "' references an unknown successor: " + transition.successor_id);
            }
            const auto& successor_segment = graph.segments.at(successor->second);
            if (!same_position(segment.geometry.end, successor_segment.geometry.start)) {
                invalid("Segment '" + segment.id + "' is not geometrically connected to '" +
                        transition.successor_id + "'");
            }
            probability_sum += transition.probability;
            successor_flow_sum +=
                core::in_cubic_meters_per_second(successor_segment.hemodynamics.flow_rate);
        }
        if (!approximately_equal(probability_sum, 1.0)) {
            invalid("Transition probabilities of segment '" + segment.id + "' do not sum to one");
        }
        for (const auto& transition : segment.transitions) {
            const auto successor_flow = core::in_cubic_meters_per_second(
                graph.segments.at(segment_indices.at(transition.successor_id))
                    .hemodynamics.flow_rate);
            if (!approximately_equal(transition.probability, successor_flow / successor_flow_sum)) {
                invalid("Transition probability of segment '" + segment.id + "' to '" +
                        transition.successor_id + "' is inconsistent with successor flow");
            }
        }

        if (segment.source_refs.empty()) {
            invalid("Segment '" + segment.id + "' must reference at least one data source");
        }
        std::unordered_set<std::string> referenced_sources;
        for (const auto& source_ref : segment.source_refs) {
            if (!referenced_sources.insert(source_ref).second) {
                invalid("Segment '" + segment.id +
                        "' has a duplicate data-source reference: " + source_ref);
            }
            if (!source_ids.contains(source_ref)) {
                invalid("Segment '" + segment.id +
                        "' references an unknown data source: " + source_ref);
            }
        }
    }

    validate_graph_connectivity(graph, segment_indices);
    validate_flow_conservation(graph);
}

VascularGraph load_vascular_graph(const VascularGraphLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "vascular-graph schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto model_document = read_json(request.model_path, "vascular-graph model");
    validate_document(model_document, schema, request.model_path);
    try {
        auto graph = decode(model_document);
        validate_vascular_graph(graph);
        return graph;
    } catch (const VascularGraphError&) {
        throw;
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::data_invalid, "Cannot decode vascular graph '" +
                                                                    request.model_path.string() +
                                                                    "': " + error.what()};
    }
}

void write_vascular_graph(const VascularGraph& graph, const std::filesystem::path& output_path) {
    validate_vascular_graph(graph);
    std::error_code error;
    if (!output_path.parent_path().empty()) {
        std::filesystem::create_directories(output_path.parent_path(), error);
        if (error) {
            throw VascularGraphError{core::ErrorCode::output_unwritable,
                                     "Cannot create vascular-graph directory '" +
                                         output_path.parent_path().string() +
                                         "': " + error.message()};
        }
    }
    std::ofstream stream{output_path, std::ios::binary | std::ios::trunc};
    if (!stream) {
        throw VascularGraphError{core::ErrorCode::output_unwritable,
                                 "Cannot write vascular graph: " + output_path.string()};
    }
    encode(graph).dump_pretty(stream);
    stream.put('\n');
    if (!stream) {
        throw VascularGraphError{core::ErrorCode::output_unwritable,
                                 "Cannot complete vascular graph: " + output_path.string()};
    }
}

} // namespace mehlissa::models::body

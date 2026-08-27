// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/transport_observation_report.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace mehlissa::models::body {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

[[nodiscard]] std::string_view name(const MeasurementSiteKind kind) noexcept {
    switch (kind) {
    case MeasurementSiteKind::sample:
        return "sample";
    case MeasurementSiteKind::gateway:
        return "gateway";
    }
    return "unknown";
}

[[nodiscard]] std::string_view name(const TrajectorySelection selection) noexcept {
    switch (selection) {
    case TrajectorySelection::none:
        return "none";
    case TrajectorySelection::all:
        return "all";
    case TrajectorySelection::first_n:
        return "first_n";
    }
    return "unknown";
}

[[nodiscard]] std::string_view name(const TrajectoryAction action) noexcept {
    switch (action) {
    case TrajectoryAction::injected:
        return "injected";
    case TrajectoryAction::entered_segment:
        return "entered_segment";
    case TrajectoryAction::extracted:
        return "extracted";
    }
    return "unknown";
}

[[nodiscard]] Json encode_optional_count(const std::optional<std::uint64_t>& count) {
    return count.has_value() ? Json{count.value()} : Json::null();
}

[[nodiscard]] Json encode_populations(const std::vector<SegmentPopulation>& populations) {
    Json result = Json::array();
    for (const auto& population : populations) {
        result.push_back(Json{jsoncons::json_object_arg,
                              {{"segment_id", population.segment_id},
                               {"particle_count", population.particle_count}}});
    }
    return result;
}

[[nodiscard]] Json encode(const CompartmentTransport& transport) {
    Json sites = Json::array();
    for (const auto& site : transport.observation_config().measurement_sites) {
        sites.push_back(Json{jsoncons::json_object_arg,
                             {{"id", site.id},
                              {"segment_id", site.segment_id},
                              {"kind", name(site.kind)}}});
    }

    Json trajectories = Json::array();
    for (const auto& record : transport.trajectory_records()) {
        trajectories.push_back(Json{jsoncons::json_object_arg,
                                     {{"time_ns", record.time.count()},
                                      {"particle_id", record.particle_id},
                                      {"action", name(record.action)},
                                      {"segment_id", record.segment_id}}});
    }

    Json measurements = Json::array();
    for (const auto& record : transport.measurement_records()) {
        measurements.push_back(Json{jsoncons::json_object_arg,
                                    {{"time_ns", record.time.count()},
                                     {"site_id", record.site_id},
                                     {"segment_id", record.segment_id},
                                     {"kind", name(record.kind)},
                                     {"particle_id", record.particle_id}}});
    }

    Json measurement_counts = Json::array();
    for (const auto& count : transport.measurement_counts()) {
        measurement_counts.push_back(Json{jsoncons::json_object_arg,
                                           {{"site_id", count.site_id},
                                            {"segment_id", count.segment_id},
                                            {"kind", name(count.kind)},
                                            {"particle_count", count.particle_count}}});
    }

    Json snapshots = Json::array();
    for (const auto& snapshot : transport.population_snapshots()) {
        snapshots.push_back(Json{jsoncons::json_object_arg,
                                 {{"time_ns", snapshot.time.count()},
                                  {"segments", encode_populations(snapshot.populations)}}});
    }

    Json extractions = Json::array();
    for (const auto& extraction : transport.extraction_results()) {
        extractions.push_back(
            Json{jsoncons::json_object_arg,
                 {{"scheduled_time_ns", extraction.scheduled_time.count()},
                  {"processed_time_ns", extraction.processed_time.count()},
                  {"segment_id", extraction.segment_id},
                  {"requested_particle_count",
                   encode_optional_count(extraction.requested_particle_count)},
                  {"extracted_particle_count", extraction.extracted_particle_count}}});
    }

    const auto& config = transport.observation_config();
    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", supported_transport_observation_report_schema_version},
            {"model", Json{jsoncons::json_object_arg,
                            {{"id", transport.graph().model_id},
                             {"version", transport.graph().model_version}}}},
            {"summary", Json{jsoncons::json_object_arg,
                              {{"injected_particle_count",
                                transport.injected_particle_count()},
                               {"active_particle_count", transport.particle_count()},
                               {"extracted_particle_count",
                                transport.extracted_particle_count()},
                               {"transition_count", transport.transition_count()}}}},
            {"configuration",
             Json{jsoncons::json_object_arg,
                  {{"trajectory_selection", name(config.trajectory_selection)},
                   {"trajectory_particle_limit", config.trajectory_particle_limit},
                   {"maximum_trajectory_records", config.maximum_trajectory_records},
                   {"maximum_measurement_records", config.maximum_measurement_records},
                   {"aggregate_interval_ns", config.aggregate_interval.count()},
                   {"maximum_aggregate_records", config.maximum_aggregate_records},
                   {"measurement_sites", std::move(sites)}}}},
            {"truncation", Json{jsoncons::json_object_arg,
                                 {{"trajectories", transport.trajectories_truncated()},
                                  {"measurements", transport.measurements_truncated()},
                                  {"aggregates", transport.aggregates_truncated()}}}},
            {"measurement_counts", std::move(measurement_counts)},
            {"measurement_records", std::move(measurements)},
            {"trajectory_records", std::move(trajectories)},
            {"population_snapshots", std::move(snapshots)},
            {"extractions", std::move(extractions)},
        }};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path,
                             const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw core::MehlissaError{core::ErrorCode::input_unreadable,
                                  "Cannot open " + std::string{role} + ": " + path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw core::MehlissaError{core::ErrorCode::json_invalid,
                                  "Invalid JSON in " + std::string{role} + " '" +
                                      path.string() + "': " + error.what()};
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw core::MehlissaError{core::ErrorCode::schema_invalid,
                                  "Invalid transport observation schema '" + path.string() +
                                      "': " + error.what()};
    }
}

} // namespace

void write_transport_observation_report(
    const CompartmentTransport& transport,
    const TransportObservationReportWriteRequest& request) {
    const auto document = encode(transport);
    const auto schema_document = read_json(request.schema_path, "transport observation schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Transport observation report does not satisfy its schema: " +
                                      std::string{error.what()}};
    }

    std::error_code error;
    if (!request.output_path.parent_path().empty()) {
        std::filesystem::create_directories(request.output_path.parent_path(), error);
        if (error) {
            throw core::MehlissaError{core::ErrorCode::output_unwritable,
                                      "Cannot create transport observation directory: " +
                                          error.message()};
        }
    }
    std::ofstream output{request.output_path, std::ios::binary | std::ios::trunc};
    if (!output) {
        throw core::MehlissaError{core::ErrorCode::output_unwritable,
                                  "Cannot write transport observation report: " +
                                      request.output_path.string()};
    }
    document.dump_pretty(output);
    output.put('\n');
    if (!output) {
        throw core::MehlissaError{core::ErrorCode::output_unwritable,
                                  "Cannot complete transport observation report: " +
                                      request.output_path.string()};
    }
}

} // namespace mehlissa::models::body

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/bvs_reference.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/random_stream.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mehlissa::models::body {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

constexpr std::uint64_t nanoseconds_per_second = 1'000'000'000ULL;
constexpr std::uint64_t reference_particle_count = 6'359;
constexpr std::uint64_t large_particle_count = 63'590;
constexpr auto aorta_segment_id = "bvs95-001";
constexpr auto popliteal_segment_id = "bvs95-062";
constexpr auto transition_stream_name = "body.bvs-reference.transitions";
constexpr double random_resolution = 9'007'199'254'740'992.0;

constexpr double maximum_equilibrium_difference_percent = 5.0;
constexpr double maximum_injection_site_difference_percent = 5.0;
constexpr double maximum_population_scale_total_variation_percent = 2.0;
constexpr double maximum_mean_perfusion_target_error_percentage_points = 0.01;
constexpr double maximum_perfusion_target_error_percentage_points = 0.01;
constexpr double maximum_dissertation_difference_percentage_points = 0.5;

struct PublishedPerfusion final {
    std::string_view region;
    std::string_view segment_id;
    double target_percent{};
    double simulation_percent{};
};

constexpr PublishedPerfusion published_perfusion[] = {
    {"heart", "bvs95-095", 5.0, 4.89},           {"head", "bvs95-009", 15.0, 14.08},
    {"right shoulder", "bvs95-011", 1.25, 1.18}, {"right upper arm", "bvs95-015", 1.25, 1.19},
    {"left shoulder", "bvs95-017", 1.25, 1.13},  {"right elbow", "bvs95-021", 1.25, 1.37},
    {"left upper arm", "bvs95-023", 1.25, 0.94}, {"chest and back", "bvs95-024", 1.67, 1.67},
    {"right hand", "bvs95-027", 1.25, 1.27},     {"left elbow", "bvs95-029", 1.25, 1.01},
    {"stomach", "bvs95-030", 10.0, 9.70},        {"left hand", "bvs95-033", 1.25, 1.11},
    {"liver", "bvs95-036", 10.0, 9.37},          {"intestine", "bvs95-039", 15.0, 13.90},
    {"kidneys", "bvs95-040", 20.0, 18.65},       {"left pelvis", "bvs95-047", 1.67, 1.54},
    {"left hip", "bvs95-049", 1.67, 1.81},       {"right pelvis", "bvs95-051", 1.67, 1.54},
    {"right hip", "bvs95-052", 1.67, 1.31},      {"left knee", "bvs95-054", 1.67, 1.41},
    {"right knee", "bvs95-056", 1.67, 1.52},     {"left foot", "bvs95-059", 1.67, 1.31},
    {"right foot", "bvs95-060", 1.67, 1.44},
};

struct TransitionEvent final {
    std::uint64_t time_ns{};
    std::uint64_t particle_id{};
    std::size_t segment_index{};
};

struct EarlierEvent final {
    [[nodiscard]] bool operator()(const TransitionEvent& left,
                                  const TransitionEvent& right) const noexcept {
        if (left.time_ns != right.time_ns) {
            return left.time_ns > right.time_ns;
        }
        return left.particle_id > right.particle_id;
    }
};

struct EventRun final {
    std::vector<std::vector<std::uint64_t>> cumulative_particle_nanoseconds;
    std::uint64_t transition_count{};
    std::uint64_t final_population{};
};

struct MasterSeed final {
    std::uint64_t value{};
};

struct ParticleCount final {
    std::uint64_t value{};
};

[[noreturn]] void invalid(const std::string& message) {
    throw VascularGraphError{core::ErrorCode::data_invalid, message};
}

[[nodiscard]] std::uint64_t checked_product(const std::uint64_t left, const std::uint64_t right,
                                            const std::string_view role) {
    if (left != 0 && right > std::numeric_limits<std::uint64_t>::max() / left) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "BVS reference " + std::string{role} + " overflow"};
    }
    return left * right;
}

[[nodiscard]] std::uint64_t checked_sum(const std::uint64_t left, const std::uint64_t right,
                                        const std::string_view role) {
    if (right > std::numeric_limits<std::uint64_t>::max() - left) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "BVS reference " + std::string{role} + " overflow"};
    }
    return left + right;
}

[[nodiscard]] std::uint64_t transit_time_ns(const VascularSegment& segment) {
    const auto length = core::in_meters(segment.geometry.length);
    const auto velocity = core::in_meters_per_second(segment.hemodynamics.mean_velocity);
    const auto value = length / velocity * static_cast<double>(nanoseconds_per_second);
    if (!std::isfinite(value) || value <= 0.0 ||
        value > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) {
        throw core::MehlissaError{core::ErrorCode::numeric_overflow,
                                  "Segment '" + segment.id +
                                      "' has an invalid BVS reference transit time"};
    }
    return static_cast<std::uint64_t>(std::ceil(value));
}

[[nodiscard]] std::size_t choose_successor(const VascularGraph& graph,
                                           const std::vector<std::size_t>& first_successor_indices,
                                           const std::vector<std::size_t>& second_successor_indices,
                                           const std::size_t segment_index,
                                           core::RandomStream& random) {
    const auto& transitions = graph.segments.at(segment_index).transitions;
    if (transitions.size() == 1) {
        return first_successor_indices.at(segment_index);
    }
    const auto sample = random.next_u64() >> 11U;
    const auto unit = static_cast<double>(sample) / random_resolution;
    if (unit < transitions.front().probability) {
        return first_successor_indices.at(segment_index);
    }
    return second_successor_indices.at(segment_index);
}

[[nodiscard]] EventRun run_event_transport(const VascularGraph& graph, const MasterSeed master_seed,
                                           const ParticleCount particle_count,
                                           const std::string_view injection_segment_id,
                                           const std::vector<std::uint64_t>& boundaries_ns) {
    if (boundaries_ns.empty() || !std::ranges::is_sorted(boundaries_ns) ||
        boundaries_ns.front() != 0) {
        invalid("BVS reference boundaries must be sorted and start at zero");
    }

    std::unordered_map<std::string, std::size_t> indices;
    std::vector<std::uint64_t> transit_times;
    std::vector<std::size_t> first_successors;
    std::vector<std::size_t> second_successors;
    indices.reserve(graph.segments.size());
    transit_times.reserve(graph.segments.size());
    for (std::size_t index = 0; index < graph.segments.size(); ++index) {
        indices.emplace(graph.segments.at(index).id, index);
        transit_times.push_back(transit_time_ns(graph.segments.at(index)));
    }
    const auto injection = indices.find(std::string{injection_segment_id});
    if (injection == indices.end()) {
        invalid("BVS reference injection segment is unknown: " + std::string{injection_segment_id});
    }
    for (const auto& segment : graph.segments) {
        first_successors.push_back(indices.at(segment.transitions.front().successor_id));
        second_successors.push_back(segment.transitions.size() == 2
                                        ? indices.at(segment.transitions.back().successor_id)
                                        : first_successors.back());
    }

    const auto segment_count = graph.segments.size();
    std::vector<std::uint64_t> populations(segment_count);
    std::vector<std::uint64_t> accumulated(segment_count);
    std::vector<std::uint64_t> last_change_ns(segment_count);
    populations.at(injection->second) = particle_count.value;

    std::priority_queue<TransitionEvent, std::vector<TransitionEvent>, EarlierEvent> events;
    const auto initial_transition_time = transit_times.at(injection->second);
    for (std::uint64_t particle_id = 1; particle_id <= particle_count.value; ++particle_id) {
        events.push({initial_transition_time, particle_id, injection->second});
    }

    const auto maximum_time_ns = boundaries_ns.back();
    core::RandomStream random{master_seed.value, transition_stream_name};
    std::uint64_t transition_count{};

    const auto update_integral = [&](const std::size_t index, const std::uint64_t time_ns) {
        const auto elapsed = time_ns - last_change_ns.at(index);
        const auto contribution = checked_product(populations.at(index), elapsed, "integral");
        accumulated.at(index) =
            checked_sum(accumulated.at(index), contribution, "integral accumulation");
        last_change_ns.at(index) = time_ns;
    };

    EventRun result;
    result.cumulative_particle_nanoseconds.reserve(boundaries_ns.size());
    for (const auto boundary_ns : boundaries_ns) {
        while (!events.empty() && events.top().time_ns <= boundary_ns) {
            const auto event = events.top();
            events.pop();
            const auto successor = choose_successor(graph, first_successors, second_successors,
                                                    event.segment_index, random);
            update_integral(event.segment_index, event.time_ns);
            update_integral(successor, event.time_ns);
            --populations.at(event.segment_index);
            ++populations.at(successor);
            ++transition_count;

            const auto next_time =
                checked_sum(event.time_ns, transit_times.at(successor), "event time");
            if (next_time <= maximum_time_ns) {
                events.push({next_time, event.particle_id, successor});
            }
        }

        std::vector<std::uint64_t> snapshot;
        snapshot.reserve(segment_count);
        for (std::size_t index = 0; index < segment_count; ++index) {
            const auto pending = checked_product(
                populations.at(index), boundary_ns - last_change_ns.at(index), "snapshot integral");
            snapshot.push_back(
                checked_sum(accumulated.at(index), pending, "snapshot accumulation"));
        }
        result.cumulative_particle_nanoseconds.push_back(std::move(snapshot));
    }

    result.transition_count = transition_count;
    for (const auto population : populations) {
        result.final_population =
            checked_sum(result.final_population, population, "population sum");
    }
    return result;
}

[[nodiscard]] std::vector<double> window_average(const EventRun& run, const std::size_t start_index,
                                                 const std::size_t end_index,
                                                 const std::vector<std::uint64_t>& boundaries_ns) {
    const auto duration = boundaries_ns.at(end_index) - boundaries_ns.at(start_index);
    std::vector<double> result;
    const auto& start = run.cumulative_particle_nanoseconds.at(start_index);
    const auto& end = run.cumulative_particle_nanoseconds.at(end_index);
    result.reserve(start.size());
    for (std::size_t index = 0; index < start.size(); ++index) {
        result.push_back(static_cast<double>(end.at(index) - start.at(index)) /
                         static_cast<double>(duration));
    }
    return result;
}

[[nodiscard]] DistributionComparison compare_distributions(const std::vector<double>& left,
                                                           const std::vector<double>& right,
                                                           const std::uint64_t particle_count) {
    if (left.size() != right.size() || left.empty()) {
        invalid("BVS reference distributions have incompatible dimensions");
    }
    DistributionComparison comparison;
    for (std::size_t index = 0; index < left.size(); ++index) {
        const auto difference = std::abs(left.at(index) - right.at(index));
        comparison.mean_absolute_difference_per_segment += difference;
        comparison.maximum_absolute_difference =
            std::max(comparison.maximum_absolute_difference, difference);
    }
    comparison.mean_absolute_difference_per_segment /= static_cast<double>(left.size());
    const auto mean_population =
        static_cast<double>(particle_count) / static_cast<double>(left.size());
    comparison.normalized_mean_difference_percent =
        comparison.mean_absolute_difference_per_segment / mean_population * 100.0;
    return comparison;
}

[[nodiscard]] double total_variation_percent(const std::vector<double>& reference,
                                             const std::uint64_t reference_count,
                                             const std::vector<double>& candidate,
                                             const std::uint64_t candidate_count) {
    if (reference.size() != candidate.size() || reference.empty()) {
        invalid("BVS population-scale distributions have incompatible dimensions");
    }
    double total{};
    for (std::size_t index = 0; index < reference.size(); ++index) {
        total += std::abs(reference.at(index) / static_cast<double>(reference_count) -
                          candidate.at(index) / static_cast<double>(candidate_count));
    }
    return total * 50.0;
}

[[nodiscard]] Json encode_comparison(const DistributionComparison& comparison) {
    return Json{
        jsoncons::json_object_arg,
        {
            {"mean_absolute_difference_per_segment",
             comparison.mean_absolute_difference_per_segment},
            {"normalized_mean_difference_percent", comparison.normalized_mean_difference_percent},
            {"maximum_absolute_difference", comparison.maximum_absolute_difference},
        }};
}

[[nodiscard]] Json encode(const BvsReferenceReport& report) {
    Json perfusion = Json::array();
    for (const auto& row : report.perfusion) {
        perfusion.push_back(
            Json{jsoncons::json_object_arg,
                 {
                     {"region", row.region},
                     {"segment_id", row.segment_id},
                     {"literature_target_percent", row.literature_target_percent},
                     {"dissertation_simulation_percent", row.dissertation_simulation_percent},
                     {"model_percent", row.model_percent},
                 }});
    }
    return Json{
        jsoncons::json_object_arg,
        {
            {"schema_version", report.schema_version},
            {"model", Json{jsoncons::json_object_arg,
                           {
                               {"id", report.model_id},
                               {"version", report.model_version},
                           }}},
            {"experiment", Json{jsoncons::json_object_arg,
                                {
                                    {"master_seed", report.master_seed},
                                    {"reference_particle_count", report.reference_particle_count},
                                    {"large_particle_count", report.large_particle_count},
                                    {"aorta_segment_id", report.aorta_segment_id},
                                    {"popliteal_segment_id", report.popliteal_segment_id},
                                    {"averaging_window_seconds", 60},
                                    {"reference_duration_seconds", 7200},
                                }}},
            {"published_reference",
             Json{jsoncons::json_object_arg,
                  {
                      {"bvs_vessel_count", 94},
                      {"bvs_minute_7_mean_difference_per_vessel", 2.03},
                      {"bvs_mean_normalized_difference_percent", 3.11},
                      {"bvs_injection_site_difference_percent", 3.95},
                      {"description",
                       "BloodVoyagerS 2018 used a 94-vessel, coarse 1:1/1:3 model. The M2.4 "
                       "dynamic gates compare the claims, not an identical implementation."},
                  }}},
            {"acceptance_thresholds",
             Json{jsoncons::json_object_arg,
                  {
                      {"maximum_minute_7_equilibrium_difference_percent",
                       maximum_equilibrium_difference_percent},
                      {"maximum_injection_site_difference_percent",
                       maximum_injection_site_difference_percent},
                      {"maximum_population_scale_total_variation_percent",
                       maximum_population_scale_total_variation_percent},
                      {"maximum_mean_perfusion_target_error_percentage_points",
                       maximum_mean_perfusion_target_error_percentage_points},
                      {"maximum_perfusion_target_error_percentage_points",
                       maximum_perfusion_target_error_percentage_points},
                      {"maximum_mean_dissertation_difference_percentage_points",
                       maximum_dissertation_difference_percentage_points},
                  }}},
            {"dynamic_distribution",
             Json{
                 jsoncons::json_object_arg,
                 {
                     {"minute_1_vs_minute_120", encode_comparison(report.minute_1_vs_minute_120)},
                     {"minute_7_vs_minute_120", encode_comparison(report.minute_7_vs_minute_120)},
                     {"minute_15_vs_minute_120", encode_comparison(report.minute_15_vs_minute_120)},
                     {"aorta_vs_popliteal_at_minute_7",
                      encode_comparison(report.aorta_vs_popliteal_at_minute_7)},
                     {"population_scale_total_variation_percent",
                      report.population_scale_total_variation_percent},
                 }}},
            {"perfusion",
             Json{jsoncons::json_object_arg,
                  {
                      {"mean_error_vs_literature_percentage_points",
                       report.mean_perfusion_error_vs_literature_percentage_points},
                      {"maximum_error_vs_literature_percentage_points",
                       report.maximum_perfusion_error_vs_literature_percentage_points},
                      {"mean_difference_vs_dissertation_percentage_points",
                       report.mean_perfusion_difference_vs_dissertation_percentage_points},
                      {"regions", std::move(perfusion)},
                  }}},
            {"diagnostics",
             Json{jsoncons::json_object_arg,
                  {
                      {"aorta_reference_transitions", report.aorta_reference_transitions},
                      {"popliteal_transitions", report.popliteal_transitions},
                      {"large_population_transitions", report.large_population_transitions},
                  }}},
            {"gates", Json{jsoncons::json_object_arg,
                           {
                               {"population_conserved", report.population_conserved},
                               {"perfusion", report.perfusion_gate_passed},
                               {"equilibrium", report.equilibrium_gate_passed},
                               {"injection_site", report.injection_site_gate_passed},
                               {"population_scale", report.population_scale_gate_passed},
                               {"overall", report.overall_passed},
                           }}},
        }};
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
        throw VascularGraphError{core::ErrorCode::schema_invalid, "Invalid BVS reference schema '" +
                                                                      path.string() +
                                                                      "': " + error.what()};
    }
}

} // namespace

BvsReferenceReport run_bvs_reference(const VascularGraph& graph, const std::uint64_t master_seed) {
    validate_vascular_graph(graph);
    const std::vector<std::uint64_t> reference_boundaries{
        0,
        60 * nanoseconds_per_second,
        360 * nanoseconds_per_second,
        420 * nanoseconds_per_second,
        840 * nanoseconds_per_second,
        900 * nanoseconds_per_second,
        7140 * nanoseconds_per_second,
        7200 * nanoseconds_per_second,
    };
    const std::vector<std::uint64_t> minute_7_boundaries{
        0,
        360 * nanoseconds_per_second,
        420 * nanoseconds_per_second,
    };

    const auto aorta =
        run_event_transport(graph, MasterSeed{master_seed}, ParticleCount{reference_particle_count},
                            aorta_segment_id, reference_boundaries);
    const auto popliteal =
        run_event_transport(graph, MasterSeed{master_seed}, ParticleCount{reference_particle_count},
                            popliteal_segment_id, minute_7_boundaries);
    const auto large =
        run_event_transport(graph, MasterSeed{master_seed}, ParticleCount{large_particle_count},
                            aorta_segment_id, minute_7_boundaries);

    const auto minute_1 = window_average(aorta, 0, 1, reference_boundaries);
    const auto minute_7 = window_average(aorta, 2, 3, reference_boundaries);
    const auto minute_15 = window_average(aorta, 4, 5, reference_boundaries);
    const auto minute_120 = window_average(aorta, 6, 7, reference_boundaries);
    const auto popliteal_minute_7 = window_average(popliteal, 1, 2, minute_7_boundaries);
    const auto large_minute_7 = window_average(large, 1, 2, minute_7_boundaries);

    BvsReferenceReport report{};
    report.schema_version = supported_bvs_reference_report_schema_version;
    report.model_id = graph.model_id;
    report.model_version = graph.model_version;
    report.master_seed = master_seed;
    report.reference_particle_count = reference_particle_count;
    report.large_particle_count = large_particle_count;
    report.aorta_segment_id = aorta_segment_id;
    report.popliteal_segment_id = popliteal_segment_id;
    report.minute_1_vs_minute_120 =
        compare_distributions(minute_1, minute_120, reference_particle_count);
    report.minute_7_vs_minute_120 =
        compare_distributions(minute_7, minute_120, reference_particle_count);
    report.minute_15_vs_minute_120 =
        compare_distributions(minute_15, minute_120, reference_particle_count);
    report.aorta_vs_popliteal_at_minute_7 =
        compare_distributions(minute_7, popliteal_minute_7, reference_particle_count);
    report.population_scale_total_variation_percent = total_variation_percent(
        minute_7, reference_particle_count, large_minute_7, large_particle_count);

    const auto* cardiac_output = graph.find_segment("bvs95-002");
    if (cardiac_output == nullptr) {
        invalid("BVS reference graph does not contain cardiac-output segment bvs95-002");
    }
    const auto total_flow =
        core::in_cubic_meters_per_second(cardiac_output->hemodynamics.flow_rate);
    for (const auto& published : published_perfusion) {
        const auto* segment = graph.find_segment(published.segment_id);
        if (segment == nullptr) {
            invalid("BVS reference graph lacks perfusion segment " +
                    std::string{published.segment_id});
        }
        const auto model_percent =
            core::in_cubic_meters_per_second(segment->hemodynamics.flow_rate) / total_flow * 100.0;
        report.perfusion.push_back({std::string{published.region},
                                    std::string{published.segment_id}, published.target_percent,
                                    published.simulation_percent, model_percent});
        const auto target_error = std::abs(model_percent - published.target_percent);
        report.mean_perfusion_error_vs_literature_percentage_points += target_error;
        report.maximum_perfusion_error_vs_literature_percentage_points =
            std::max(report.maximum_perfusion_error_vs_literature_percentage_points, target_error);
        report.mean_perfusion_difference_vs_dissertation_percentage_points +=
            std::abs(model_percent - published.simulation_percent);
    }
    report.mean_perfusion_error_vs_literature_percentage_points /=
        static_cast<double>(report.perfusion.size());
    report.mean_perfusion_difference_vs_dissertation_percentage_points /=
        static_cast<double>(report.perfusion.size());

    report.aorta_reference_transitions = aorta.transition_count;
    report.popliteal_transitions = popliteal.transition_count;
    report.large_population_transitions = large.transition_count;
    report.population_conserved = aorta.final_population == reference_particle_count &&
                                  popliteal.final_population == reference_particle_count &&
                                  large.final_population == large_particle_count;
    report.perfusion_gate_passed =
        report.mean_perfusion_error_vs_literature_percentage_points <=
            maximum_mean_perfusion_target_error_percentage_points &&
        report.maximum_perfusion_error_vs_literature_percentage_points <=
            maximum_perfusion_target_error_percentage_points &&
        report.mean_perfusion_difference_vs_dissertation_percentage_points <=
            maximum_dissertation_difference_percentage_points;
    report.equilibrium_gate_passed =
        report.minute_7_vs_minute_120.normalized_mean_difference_percent <=
        maximum_equilibrium_difference_percent;
    report.injection_site_gate_passed =
        report.aorta_vs_popliteal_at_minute_7.normalized_mean_difference_percent <=
        maximum_injection_site_difference_percent;
    report.population_scale_gate_passed = report.population_scale_total_variation_percent <=
                                          maximum_population_scale_total_variation_percent;
    report.overall_passed = report.population_conserved && report.perfusion_gate_passed &&
                            report.equilibrium_gate_passed && report.injection_site_gate_passed &&
                            report.population_scale_gate_passed;
    return report;
}

void write_bvs_reference_report(const BvsReferenceReport& report,
                                const BvsReferenceReportWriteRequest& request) {
    const auto document = encode(report);
    const auto schema_document = read_json(request.schema_path, "BVS reference schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw VascularGraphError{core::ErrorCode::data_invalid,
                                 "BVS reference report does not satisfy its schema: " +
                                     std::string{error.what()}};
    }

    std::error_code error;
    if (!request.output_path.parent_path().empty()) {
        std::filesystem::create_directories(request.output_path.parent_path(), error);
        if (error) {
            throw VascularGraphError{core::ErrorCode::output_unwritable,
                                     "Cannot create BVS reference directory: " + error.message()};
        }
    }
    std::ofstream output{request.output_path, std::ios::binary | std::ios::trunc};
    if (!output) {
        throw VascularGraphError{core::ErrorCode::output_unwritable,
                                 "Cannot write BVS reference report: " +
                                     request.output_path.string()};
    }
    document.dump_pretty(output);
    output.put('\n');
    if (!output) {
        throw VascularGraphError{core::ErrorCode::output_unwritable,
                                 "Cannot complete BVS reference report: " +
                                     request.output_path.string()};
    }
}

} // namespace mehlissa::models::body

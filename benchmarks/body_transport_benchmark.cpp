// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/experiment/provenance.hpp>
#include <mehlissa/models/body/body_state_profile.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/transport_observation_report.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
// clang-format off: psapi.h requires the Windows base declarations first.
#include <windows.h>
#include <psapi.h>
// clang-format on
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;
using Duration = mehlissa::core::SimulationClock::Duration;
using Clock = std::chrono::steady_clock;

constexpr std::string_view supported_manifest_schema_version = "1.0.0";
constexpr std::string_view supported_report_schema_version = "1.0.0";

struct CommandLine final {
    std::filesystem::path manifest_path;
    std::filesystem::path manifest_schema_path;
    std::filesystem::path result_schema_path;
    std::filesystem::path observation_schema_path;
    std::filesystem::path output_path;
    std::filesystem::path observation_output_path;
};

struct VersionedInput final {
    std::filesystem::path path;
    std::filesystem::path schema_path;
    std::string expected_id;
    std::string expected_version;
};

struct Injection final {
    Duration time;
    std::string segment_id;
    std::uint64_t particle_count{};
};

struct Observation final {
    std::string policy_id;
    mehlissa::models::body::TransportObservationConfig config;
    Json document;
};

struct Manifest final {
    std::string benchmark_id;
    std::string machine_label;
    VersionedInput model;
    std::optional<VersionedInput> state_profile;
    std::uint64_t master_seed{};
    Duration duration;
    Injection injection;
    Observation observation;
};

struct PlatformMetadata final {
    std::string cpu_model;
    std::uint64_t logical_processor_count{};
    std::uint64_t physical_memory_bytes{};
    std::uint64_t peak_resident_set_bytes{};
};

[[noreturn]] void invalid(const std::string& message) {
    throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::manifest_invalid, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::input_unreadable,
                                            "Cannot open " + std::string{role} + ": " +
                                                path.string()};
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::json_invalid,
                                            "Invalid JSON in " + std::string{role} + " '" +
                                                path.string() + "': " + error.what()};
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document, const std::filesystem::path& path,
                                            const std::string_view role) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::schema_invalid,
                                            "Invalid " + std::string{role} + " '" + path.string() +
                                                "': " + error.what()};
    }
}

void validate_document(const Json& document, const std::filesystem::path& schema_path,
                       const std::string_view role) {
    const auto schema_document = read_json(schema_path, std::string{role} + " schema");
    const auto schema = compile_schema(schema_document, schema_path, role);
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::manifest_invalid,
                                            std::string{role} +
                                                " does not satisfy its schema: " + error.what()};
    }
}

[[nodiscard]] std::filesystem::path resolve_input_path(const std::filesystem::path& base,
                                                       const std::string& configured) {
    auto path = std::filesystem::path{configured};
    if (path.is_relative()) {
        path = base / path;
    }
    return std::filesystem::absolute(path).lexically_normal();
}

[[nodiscard]] std::filesystem::path absolute_path(const std::filesystem::path& path) {
    return std::filesystem::absolute(path).lexically_normal();
}

[[nodiscard]] std::size_t checked_size(const std::uint64_t value, const std::string_view field) {
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        invalid(std::string{field} + " exceeds the platform size limit");
    }
    return static_cast<std::size_t>(value);
}

[[nodiscard]] Duration checked_duration(const std::uint64_t value, const std::string_view field) {
    if (value > static_cast<std::uint64_t>(std::numeric_limits<Duration::rep>::max())) {
        invalid(std::string{field} + " exceeds the supported nanosecond range");
    }
    return Duration{static_cast<Duration::rep>(value)};
}

[[nodiscard]] VersionedInput decode_versioned_input(const Json& document,
                                                    const std::filesystem::path& base) {
    return VersionedInput{resolve_input_path(base, document.at("path").as<std::string>()),
                          resolve_input_path(base, document.at("schema_path").as<std::string>()),
                          document.at("expected_id").as<std::string>(),
                          document.at("expected_version").as<std::string>()};
}

[[nodiscard]] mehlissa::models::body::TrajectorySelection
decode_trajectory_selection(const std::string_view selection) {
    using Selection = mehlissa::models::body::TrajectorySelection;
    if (selection == "none") {
        return Selection::none;
    }
    if (selection == "all") {
        return Selection::all;
    }
    if (selection == "first_n") {
        return Selection::first_n;
    }
    invalid("Unsupported trajectory selection");
}

[[nodiscard]] mehlissa::models::body::MeasurementSiteKind
decode_site_kind(const std::string_view kind) {
    if (kind == "sample") {
        return mehlissa::models::body::MeasurementSiteKind::sample;
    }
    if (kind == "gateway") {
        return mehlissa::models::body::MeasurementSiteKind::gateway;
    }
    invalid("Unsupported measurement-site kind");
}

void validate_policy(const Observation& observation) {
    using Selection = mehlissa::models::body::TrajectorySelection;
    const auto& config = observation.config;
    const auto no_trajectories = config.trajectory_selection == Selection::none &&
                                 config.trajectory_particle_limit == 0 &&
                                 config.maximum_trajectory_records == 0;
    const auto no_aggregates =
        config.aggregate_interval == Duration::zero() && config.maximum_aggregate_records == 0;
    const auto aggregates_enabled =
        config.aggregate_interval > Duration::zero() && config.maximum_aggregate_records > 0;
    const auto sites_enabled = !config.measurement_sites.empty();

    if (observation.policy_id == "O0") {
        if (!no_trajectories || !no_aggregates || sites_enabled ||
            config.maximum_measurement_records != 0) {
            invalid("O0 must disable trajectories, sites, measurement records, and aggregates");
        }
        return;
    }
    if (observation.policy_id == "O1") {
        if (!no_trajectories || !aggregates_enabled || !sites_enabled ||
            config.maximum_measurement_records != 0) {
            invalid(
                "O1 requires aggregates and passive counters, with individual records disabled");
        }
        return;
    }
    if (observation.policy_id == "O2") {
        if (config.trajectory_selection != Selection::first_n ||
            config.trajectory_particle_limit == 0 || config.maximum_trajectory_records == 0 ||
            !aggregates_enabled || !sites_enabled || config.maximum_measurement_records == 0) {
            invalid(
                "O2 requires bounded first-N trajectories, aggregates, and measurement records");
        }
        return;
    }
    if (observation.policy_id == "O3") {
        if (config.trajectory_selection != Selection::all ||
            config.trajectory_particle_limit != 0 || config.maximum_trajectory_records == 0 ||
            !aggregates_enabled || !sites_enabled || config.maximum_measurement_records == 0) {
            invalid("O3 requires complete trajectories, aggregates, and measurement records");
        }
        return;
    }
    invalid("Unsupported observation policy");
}

[[nodiscard]] Manifest decode_manifest(const Json& document,
                                       const std::filesystem::path& manifest_path) {
    if (document.at("schema_version").as<std::string_view>() != supported_manifest_schema_version) {
        invalid("Unsupported body-transport benchmark manifest schema version");
    }

    const auto base = manifest_path.parent_path();
    const auto& injection_document = document.at("injection");
    const auto& observation_document = document.at("observation");
    const auto trajectory_limit =
        observation_document.at("trajectory_particle_limit").as<std::uint64_t>();
    const auto trajectory_records =
        observation_document.at("maximum_trajectory_records").as<std::uint64_t>();
    const auto measurement_records =
        observation_document.at("maximum_measurement_records").as<std::uint64_t>();
    const auto aggregate_interval =
        observation_document.at("aggregate_interval_ns").as<std::uint64_t>();
    const auto aggregate_records =
        observation_document.at("maximum_aggregate_records").as<std::uint64_t>();

    mehlissa::models::body::TransportObservationConfig config;
    config.trajectory_selection = decode_trajectory_selection(
        observation_document.at("trajectory_selection").as<std::string_view>());
    config.trajectory_particle_limit = trajectory_limit;
    config.maximum_trajectory_records =
        checked_size(trajectory_records, "maximum_trajectory_records");
    config.maximum_measurement_records =
        checked_size(measurement_records, "maximum_measurement_records");
    config.aggregate_interval = checked_duration(aggregate_interval, "aggregate_interval_ns");
    config.maximum_aggregate_records = checked_size(aggregate_records, "maximum_aggregate_records");

    std::unordered_set<std::string> site_ids;
    for (const auto& site : observation_document.at("measurement_sites").array_range()) {
        auto decoded = mehlissa::models::body::MeasurementSite{
            site.at("id").as<std::string>(), site.at("segment_id").as<std::string>(),
            decode_site_kind(site.at("kind").as<std::string_view>())};
        if (!site_ids.emplace(decoded.id).second) {
            invalid("Measurement-site IDs must be unique: " + decoded.id);
        }
        config.measurement_sites.push_back(std::move(decoded));
    }

    std::optional<VersionedInput> state_profile;
    if (!document.at("state_profile").is_null()) {
        state_profile = decode_versioned_input(document.at("state_profile"), base);
    }

    Manifest result{document.at("benchmark_id").as<std::string>(),
                    document.at("machine_label").as<std::string>(),
                    decode_versioned_input(document.at("model"), base),
                    std::move(state_profile),
                    document.at("master_seed").as<std::uint64_t>(),
                    checked_duration(document.at("duration_ns").as<std::uint64_t>(), "duration_ns"),
                    Injection{checked_duration(injection_document.at("time_ns").as<std::uint64_t>(),
                                               "injection.time_ns"),
                              injection_document.at("segment_id").as<std::string>(),
                              injection_document.at("particle_count").as<std::uint64_t>()},
                    Observation{observation_document.at("policy_id").as<std::string>(),
                                std::move(config), observation_document}};

    if (result.injection.time > result.duration) {
        invalid("Injection time must not exceed benchmark duration");
    }
    validate_policy(result.observation);
    return result;
}

[[nodiscard]] CommandLine parse_command_line(const int argc, char** argv) {
    if (argc == 2 && std::string_view{argv[1]} == "--help") {
        std::cout << "Usage: mehlissa_body_transport_benchmark --manifest FILE "
                     "--manifest-schema FILE --result-schema FILE --observation-schema FILE "
                     "--output FILE --observation-output FILE\n";
        std::exit(0);
    }

    CommandLine result;
    std::unordered_set<std::string> seen;
    for (int index = 1; index < argc; index += 2) {
        if (index + 1 >= argc) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Every benchmark option requires a value"};
        }
        const std::string option{argv[index]};
        if (!seen.emplace(option).second) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Duplicate benchmark option: " + option};
        }
        auto assign = [&](std::filesystem::path& destination) { destination = argv[index + 1]; };
        if (option == "--manifest") {
            assign(result.manifest_path);
        } else if (option == "--manifest-schema") {
            assign(result.manifest_schema_path);
        } else if (option == "--result-schema") {
            assign(result.result_schema_path);
        } else if (option == "--observation-schema") {
            assign(result.observation_schema_path);
        } else if (option == "--output") {
            assign(result.output_path);
        } else if (option == "--observation-output") {
            assign(result.observation_output_path);
        } else {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::command_line_invalid,
                                                "Unknown benchmark option: " + option};
        }
    }
    if (result.manifest_path.empty() || result.manifest_schema_path.empty() ||
        result.result_schema_path.empty() || result.observation_schema_path.empty() ||
        result.output_path.empty() || result.observation_output_path.empty()) {
        throw mehlissa::core::MehlissaError{
            mehlissa::core::ErrorCode::command_line_invalid,
            "All benchmark paths are required; use --help for usage"};
    }
    result.manifest_path = absolute_path(result.manifest_path);
    result.manifest_schema_path = absolute_path(result.manifest_schema_path);
    result.result_schema_path = absolute_path(result.result_schema_path);
    result.observation_schema_path = absolute_path(result.observation_schema_path);
    result.output_path = absolute_path(result.output_path);
    result.observation_output_path = absolute_path(result.observation_output_path);
    return result;
}

[[nodiscard]] std::uint64_t elapsed_ns(const Clock::time_point started,
                                       const Clock::time_point completed) {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(completed - started).count());
}

[[nodiscard]] std::string path_string(const std::filesystem::path& path) {
    return path.generic_string();
}

[[nodiscard]] std::string
population_hash(const std::vector<mehlissa::models::body::SegmentPopulation>& populations) {
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    auto hash = offset;
    auto consume = [&](const std::uint8_t value) {
        hash ^= value;
        hash *= prime;
    };
    for (const auto& population : populations) {
        for (const auto character : population.segment_id) {
            consume(static_cast<std::uint8_t>(character));
        }
        consume(0);
        for (int shift = 56; shift >= 0; shift -= 8) {
            consume(static_cast<std::uint8_t>((population.particle_count >> shift) & 0xffU));
        }
    }
    std::ostringstream encoded;
    encoded << std::hex << std::setfill('0') << std::setw(16) << hash;
    return encoded.str();
}

[[nodiscard]] std::string cpu_model() {
#if defined(_WIN32)
    std::array<wchar_t, 512> buffer{};
    DWORD bytes = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
    const auto status =
        RegGetValueW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     L"ProcessorNameString", RRF_RT_REG_SZ, nullptr, buffer.data(), &bytes);
    if (status == ERROR_SUCCESS) {
        const auto length = static_cast<int>(wcsnlen_s(buffer.data(), buffer.size()));
        const auto utf8_size =
            WideCharToMultiByte(CP_UTF8, 0, buffer.data(), length, nullptr, 0, nullptr, nullptr);
        if (utf8_size > 0) {
            std::string result(static_cast<std::size_t>(utf8_size), '\0');
            WideCharToMultiByte(CP_UTF8, 0, buffer.data(), length, result.data(), utf8_size,
                                nullptr, nullptr);
            return result;
        }
    }
#elif defined(__linux__)
    std::ifstream input{"/proc/cpuinfo"};
    std::string line;
    while (std::getline(input, line)) {
        constexpr std::string_view key = "model name";
        if (line.starts_with(key)) {
            const auto separator = line.find(':');
            if (separator != std::string::npos) {
                const auto first = line.find_first_not_of(" \t", separator + 1);
                return first == std::string::npos ? "unknown" : line.substr(first);
            }
        }
    }
#endif
    return "unknown";
}

[[nodiscard]] std::uint64_t physical_memory_bytes() noexcept {
#if defined(_WIN32)
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status) != 0) {
        return status.ullTotalPhys;
    }
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
    const auto pages = sysconf(_SC_PHYS_PAGES);
    const auto page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0) {
        return static_cast<std::uint64_t>(pages) * static_cast<std::uint64_t>(page_size);
    }
#endif
    return 0;
}

[[nodiscard]] std::uint64_t peak_resident_set_bytes() noexcept {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
        return static_cast<std::uint64_t>(counters.PeakWorkingSetSize);
    }
#elif defined(__unix__) || defined(__APPLE__)
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
        return static_cast<std::uint64_t>(usage.ru_maxrss);
#else
        return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024U;
#endif
    }
#endif
    return 0;
}

[[nodiscard]] PlatformMetadata platform_metadata() {
    return PlatformMetadata{
        cpu_model(),
        std::max<std::uint64_t>(1, static_cast<std::uint64_t>(std::thread::hardware_concurrency())),
        physical_memory_bytes(), peak_resident_set_bytes()};
}

[[nodiscard]] Json
encode_measurement_counts(const std::vector<mehlissa::models::body::MeasurementCount>& counts) {
    Json result = Json::array();
    for (const auto& count : counts) {
        const auto kind = count.kind == mehlissa::models::body::MeasurementSiteKind::sample
                              ? "sample"
                              : "gateway";
        result.push_back(Json{jsoncons::json_object_arg,
                              {{"site_id", count.site_id},
                               {"segment_id", count.segment_id},
                               {"kind", kind},
                               {"particle_count", count.particle_count}}});
    }
    return result;
}

[[nodiscard]] Json
encode_random_streams(const std::vector<mehlissa::core::RandomStreamState>& streams) {
    Json result = Json::array();
    for (const auto& stream : streams) {
        result.push_back(Json{jsoncons::json_object_arg,
                              {{"name", stream.name}, {"draw_count", stream.draw_count}}});
    }
    return result;
}

struct ValidatedReportWriteRequest final {
    std::filesystem::path schema_path;
    std::filesystem::path output_path;
};

void write_validated_report(const Json& report, const ValidatedReportWriteRequest& request) {
    if (report.at("schema_version").as<std::string_view>() != supported_report_schema_version) {
        throw mehlissa::core::MehlissaError{
            mehlissa::core::ErrorCode::internal_failure,
            "Benchmark report encoder selected an unsupported schema version"};
    }
    validate_document(report, request.schema_path, "body-transport benchmark report");

    std::error_code error;
    if (!request.output_path.parent_path().empty()) {
        std::filesystem::create_directories(request.output_path.parent_path(), error);
        if (error) {
            throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::output_unwritable,
                                                "Cannot create benchmark report directory: " +
                                                    error.message()};
        }
    }
    std::ofstream output{request.output_path, std::ios::binary | std::ios::trunc};
    if (!output) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::output_unwritable,
                                            "Cannot write benchmark report: " +
                                                request.output_path.string()};
    }
    report.dump_pretty(output);
    output.put('\n');
    if (!output) {
        throw mehlissa::core::MehlissaError{mehlissa::core::ErrorCode::output_unwritable,
                                            "Cannot complete benchmark report: " +
                                                request.output_path.string()};
    }
}

[[nodiscard]] int run(const CommandLine& command_line) {
    const auto manifest_document = read_json(command_line.manifest_path, "benchmark manifest");
    validate_document(manifest_document, command_line.manifest_schema_path,
                      "body-transport benchmark manifest");
    const auto manifest = decode_manifest(manifest_document, command_line.manifest_path);

    auto graph = mehlissa::models::body::load_vascular_graph(
        {manifest.model.path, manifest.model.schema_path});
    if (graph.model_id != manifest.model.expected_id ||
        graph.model_version != manifest.model.expected_version) {
        invalid("Loaded vascular graph identity does not match the manifest expectation");
    }
    if (graph.find_segment(manifest.injection.segment_id) == nullptr) {
        invalid("Injection segment does not exist in the selected vascular graph: " +
                manifest.injection.segment_id);
    }
    for (const auto& site : manifest.observation.config.measurement_sites) {
        if (graph.find_segment(site.segment_id) == nullptr) {
            invalid("Measurement-site segment does not exist in the selected vascular graph: " +
                    site.segment_id);
        }
    }

    Json state_profile_identity = Json::null();
    Json state_profile_schema_identity = Json::null();
    if (manifest.state_profile.has_value()) {
        const auto& input = manifest.state_profile.value();
        const auto profile =
            mehlissa::models::body::load_body_state_profile({input.path, input.schema_path});
        if (profile.profile_id != input.expected_id ||
            profile.profile_version != input.expected_version) {
            invalid("Loaded body-state profile identity does not match the manifest expectation");
        }
        graph = mehlissa::models::body::apply_body_state_profile(graph, profile);
        state_profile_identity =
            Json{jsoncons::json_object_arg,
                 {{"id", profile.profile_id},
                  {"version", profile.profile_version},
                  {"input", Json{jsoncons::json_object_arg,
                                 {{"path", path_string(input.path)},
                                  {"sha256", mehlissa::experiment::sha256_file(input.path)}}}}}};
        state_profile_schema_identity =
            Json{jsoncons::json_object_arg,
                 {{"path", path_string(input.schema_path)},
                  {"sha256", mehlissa::experiment::sha256_file(input.schema_path)}}};
    }

    auto transport = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        std::move(graph),
        std::vector<mehlissa::models::body::InjectionEvent>{{manifest.injection.time,
                                                             manifest.injection.segment_id,
                                                             manifest.injection.particle_count}},
        std::vector<mehlissa::models::body::ExtractionEvent>{}, manifest.observation.config);
    auto* transport_view = transport.get();
    mehlissa::core::ComponentHost host{manifest.master_seed};
    host.add(std::move(transport));

    const auto end_to_end_started = Clock::now();
    const auto initialization_started = Clock::now();
    host.initialize();
    const auto initialization_completed = Clock::now();

    const auto maximum_advance = transport_view->maximum_advance();
    if (maximum_advance <= Duration::zero()) {
        throw mehlissa::core::MehlissaError{
            mehlissa::core::ErrorCode::invariant_violated,
            "Body transport selected a non-positive maximum advance"};
    }
    std::uint64_t advance_count{};
    const auto simulation_started = Clock::now();
    while (host.context().clock().now() < manifest.duration) {
        const auto remaining = manifest.duration - host.context().clock().now();
        host.advance(std::min(maximum_advance, remaining));
        ++advance_count;
    }
    const auto simulation_completed = Clock::now();

    const auto finalization_started = Clock::now();
    host.finalize();
    const auto finalization_completed = Clock::now();

    const auto random_streams = host.context().random_stream_states();
    const auto final_populations = transport_view->segment_populations();
    const auto observation_metrics = mehlissa::models::body::write_transport_observation_report(
        *transport_view,
        {command_line.observation_output_path, command_line.observation_schema_path});
    const auto end_to_end_completed = Clock::now();

    const auto build = mehlissa::experiment::current_build_metadata();
    const auto platform = platform_metadata();
    const auto simulation_ns = elapsed_ns(simulation_started, simulation_completed);
    const auto throughput = simulation_ns == 0
                                ? 0.0
                                : static_cast<double>(transport_view->transition_count()) *
                                      1'000'000'000.0 / static_cast<double>(simulation_ns);

    const Json report{
        jsoncons::json_object_arg,
        {
            {"schema_version", supported_report_schema_version},
            {"benchmark",
             Json{jsoncons::json_object_arg,
                  {{"id", manifest.benchmark_id},
                   {"manifest", Json{jsoncons::json_object_arg,
                                     {{"path", path_string(command_line.manifest_path)},
                                      {"sha256", mehlissa::experiment::sha256_file(
                                                     command_line.manifest_path)}}}}}}},
            {"schemas",
             Json{
                 jsoncons::json_object_arg,
                 {{"manifest", Json{jsoncons::json_object_arg,
                                    {{"path", path_string(command_line.manifest_schema_path)},
                                     {"sha256", mehlissa::experiment::sha256_file(
                                                    command_line.manifest_schema_path)}}}},
                  {"result", Json{jsoncons::json_object_arg,
                                  {{"path", path_string(command_line.result_schema_path)},
                                   {"sha256", mehlissa::experiment::sha256_file(
                                                  command_line.result_schema_path)}}}},
                  {"observation", Json{jsoncons::json_object_arg,
                                       {{"path", path_string(command_line.observation_schema_path)},
                                        {"sha256", mehlissa::experiment::sha256_file(
                                                       command_line.observation_schema_path)}}}},
                  {"model", Json{jsoncons::json_object_arg,
                                 {{"path", path_string(manifest.model.schema_path)},
                                  {"sha256", mehlissa::experiment::sha256_file(
                                                 manifest.model.schema_path)}}}},
                  {"state_profile", std::move(state_profile_schema_identity)}}}},
            {"software", Json{jsoncons::json_object_arg,
                              {{"name", "MEHLISSA"},
                               {"version", build.software_version},
                               {"git_commit", build.git_commit},
                               {"git_dirty", build.git_dirty},
                               {"build_type", build.build_type},
                               {"compiler", Json{jsoncons::json_object_arg,
                                                 {{"id", build.compiler_id},
                                                  {"version", build.compiler_version}}}}}}},
            {"platform", Json{jsoncons::json_object_arg,
                              {{"operating_system", build.operating_system},
                               {"architecture", build.architecture},
                               {"machine_label", manifest.machine_label},
                               {"cpu_model", platform.cpu_model},
                               {"logical_processor_count", platform.logical_processor_count},
                               {"physical_memory_bytes", platform.physical_memory_bytes},
                               {"peak_resident_set_bytes", platform.peak_resident_set_bytes}}}},
            {"model",
             Json{jsoncons::json_object_arg,
                  {{"id", transport_view->graph().model_id},
                   {"version", transport_view->graph().model_version},
                   {"input",
                    Json{jsoncons::json_object_arg,
                         {{"path", path_string(manifest.model.path)},
                          {"sha256", mehlissa::experiment::sha256_file(manifest.model.path)}}}},
                   {"state_profile", std::move(state_profile_identity)}}}},
            {"configuration",
             Json{jsoncons::json_object_arg,
                  {{"master_seed", manifest.master_seed},
                   {"duration_ns", manifest.duration.count()},
                   {"injection", Json{jsoncons::json_object_arg,
                                      {{"time_ns", manifest.injection.time.count()},
                                       {"segment_id", manifest.injection.segment_id},
                                       {"particle_count", manifest.injection.particle_count}}}},
                   {"observation_policy", manifest.observation.document},
                   {"maximum_advance_ns", maximum_advance.count()},
                   {"advance_count", advance_count}}}},
            {"timing",
             Json{jsoncons::json_object_arg,
                  {{"initialization_ns",
                    elapsed_ns(initialization_started, initialization_completed)},
                   {"simulation_ns", simulation_ns},
                   {"finalization_ns", elapsed_ns(finalization_started, finalization_completed)},
                   {"observation_encoding_ns", observation_metrics.document_encoding_ns},
                   {"observation_schema_validation_ns", observation_metrics.schema_validation_ns},
                   {"observation_serialization_and_write_ns",
                    observation_metrics.serialization_and_write_ns},
                   {"end_to_end_ns", elapsed_ns(end_to_end_started, end_to_end_completed)}}}},
            {"summary",
             Json{jsoncons::json_object_arg,
                  {{"injected_particle_count", transport_view->injected_particle_count()},
                   {"active_particle_count", transport_view->particle_count()},
                   {"extracted_particle_count", transport_view->extracted_particle_count()},
                   {"transition_count", transport_view->transition_count()},
                   {"transition_throughput_per_second", throughput},
                   {"final_population_hash", Json{jsoncons::json_object_arg,
                                                  {{"algorithm", "fnv1a64"},
                                                   {"value", population_hash(final_populations)}}}},
                   {"random_streams", encode_random_streams(random_streams)}}}},
            {"observation",
             Json{
                 jsoncons::json_object_arg,
                 {{"output", Json{jsoncons::json_object_arg,
                                  {{"path", path_string(command_line.observation_output_path)},
                                   {"sha256", mehlissa::experiment::sha256_file(
                                                  command_line.observation_output_path)},
                                   {"bytes", observation_metrics.output_bytes}}}},
                  {"record_counts",
                   Json{jsoncons::json_object_arg,
                        {{"trajectory_records", transport_view->trajectory_records().size()},
                         {"measurement_records", transport_view->measurement_records().size()},
                         {"population_snapshots", transport_view->population_snapshots().size()}}}},
                  {"truncation", Json{jsoncons::json_object_arg,
                                      {{"trajectories", transport_view->trajectories_truncated()},
                                       {"measurements", transport_view->measurements_truncated()},
                                       {"aggregates", transport_view->aggregates_truncated()}}}},
                  {"measurement_counts",
                   encode_measurement_counts(transport_view->measurement_counts())}}}},
        }};

    write_validated_report(report, {command_line.result_schema_path, command_line.output_path});
    std::cout << "benchmark_id=" << manifest.benchmark_id
              << " policy=" << manifest.observation.policy_id
              << " population=" << manifest.injection.particle_count
              << " transitions=" << transport_view->transition_count()
              << " result=" << path_string(command_line.output_path) << '\n';
    return 0;
}

} // namespace

int main(const int argc, char** argv) {
    try {
        return run(parse_command_line(argc, argv));
    } catch (const mehlissa::core::MehlissaError& error) {
        std::cerr << '[' << error.code_id() << "] " << error.what() << '\n';
        return 3;
    } catch (const std::exception& error) {
        std::cerr << "[MEHLISSA-E9001] " << error.what() << '\n';
        return 4;
    }
}

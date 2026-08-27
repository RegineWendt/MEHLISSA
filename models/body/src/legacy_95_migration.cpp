// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/body/legacy_95_migration.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <numbers>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mehlissa::models::body {
namespace {

constexpr std::size_t expected_segment_count = 95;
constexpr std::size_t expected_transition_count = 23;
constexpr double centimeters_to_meters = 0.01;
constexpr double reference_cardiac_output_m3_s = 0.0001; // 6.0 L/min

struct LegacySegment final {
    int id{};
    int type{};
    std::array<double, 3> start{};
    std::array<double, 3> end{};
};

[[noreturn]] void migration_error(const std::string& message) {
    throw VascularGraphError{core::ErrorCode::data_invalid,
                             "Cannot migrate legacy 95-segment data: " + message};
}

[[nodiscard]] std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

[[nodiscard]] std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream{line};
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }
    if (!line.empty() && line.back() == ',') {
        fields.emplace_back();
    }
    return fields;
}

[[nodiscard]] int parse_integer(const std::string& field, const std::string_view role,
                                const std::size_t line_number) {
    try {
        std::size_t consumed{};
        const auto value = std::stoi(field, &consumed);
        if (consumed != field.size()) {
            migration_error(std::string{role} + " line " + std::to_string(line_number) +
                            " contains a non-integer field");
        }
        return value;
    } catch (const VascularGraphError&) {
        throw;
    } catch (const std::exception&) {
        migration_error(std::string{role} + " line " + std::to_string(line_number) +
                        " contains an invalid integer");
    }
}

[[nodiscard]] double parse_number(const std::string& field, const std::string_view role,
                                  const std::size_t line_number) {
    try {
        std::size_t consumed{};
        const auto value = std::stod(field, &consumed);
        if (consumed != field.size() || !std::isfinite(value)) {
            migration_error(std::string{role} + " line " + std::to_string(line_number) +
                            " contains a non-finite or malformed number");
        }
        return value;
    } catch (const VascularGraphError&) {
        throw;
    } catch (const std::exception&) {
        migration_error(std::string{role} + " line " + std::to_string(line_number) +
                        " contains an invalid number");
    }
}

[[nodiscard]] std::ifstream open_input(const std::filesystem::path& path,
                                       const std::string_view role) {
    std::ifstream input{path, std::ios::binary};
    if (!input) {
        throw VascularGraphError{core::ErrorCode::input_unreadable,
                                 "Cannot open legacy " + std::string{role} + ": " + path.string()};
    }
    return input;
}

[[nodiscard]] std::vector<LegacySegment> read_segments(const std::filesystem::path& path) {
    auto input = open_input(path, "vasculature file");
    std::vector<LegacySegment> segments;
    std::string line;
    std::size_t line_number{};
    while (std::getline(input, line)) {
        ++line_number;
        if (trim(line).empty()) {
            migration_error("vasculature line " + std::to_string(line_number) + " is empty");
        }
        const auto fields = split_csv_line(line);
        if (fields.size() != 10 || fields.at(8) != "0" || !fields.at(9).empty()) {
            migration_error("vasculature line " + std::to_string(line_number) +
                            " does not have the documented ten-field legacy form");
        }
        LegacySegment segment;
        segment.id = parse_integer(fields.at(0), "vasculature", line_number);
        segment.type = parse_integer(fields.at(1), "vasculature", line_number);
        for (std::size_t axis = 0; axis < 3; ++axis) {
            segment.start.at(axis) = parse_number(fields.at(axis + 2), "vasculature", line_number);
            segment.end.at(axis) = parse_number(fields.at(axis + 5), "vasculature", line_number);
        }
        if (segment.type < 0 || segment.type > 2) {
            migration_error("vasculature line " + std::to_string(line_number) +
                            " has an unknown vessel type");
        }
        segments.push_back(segment);
    }
    if (segments.size() != expected_segment_count) {
        migration_error("expected 95 vasculature rows, found " + std::to_string(segments.size()));
    }
    std::ranges::sort(segments, {}, &LegacySegment::id);
    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (segments.at(index).id != static_cast<int>(index + 1)) {
            migration_error("legacy vessel IDs must be unique and cover 1 through 95");
        }
    }
    return segments;
}

using SplitProbabilities = std::array<double, 2>;

[[nodiscard]] std::unordered_map<int, SplitProbabilities>
read_probabilities(const std::filesystem::path& path) {
    auto input = open_input(path, "transition file");
    std::unordered_map<int, SplitProbabilities> probabilities;
    std::string line;
    std::size_t line_number{};
    while (std::getline(input, line)) {
        ++line_number;
        if (trim(line).empty()) {
            continue;
        }
        const auto fields = split_csv_line(line);
        const bool three_fields = fields.size() == 3;
        const bool trailing_empty_field = fields.size() == 4 && fields.at(3).empty();
        if (!three_fields && !trailing_empty_field) {
            migration_error("transition line " + std::to_string(line_number) +
                            " does not have the documented legacy form");
        }
        const auto id = parse_integer(fields.at(0), "transition", line_number);
        const SplitProbabilities split{
            parse_number(fields.at(1), "transition", line_number),
            parse_number(fields.at(2), "transition", line_number),
        };
        if (split.at(0) <= 0.0 || split.at(1) <= 0.0 ||
            std::abs(split.at(0) + split.at(1) - 1.0) > 1.0e-12) {
            migration_error("transition line " + std::to_string(line_number) +
                            " does not contain two positive probabilities summing to one");
        }
        if (!probabilities.emplace(id, split).second) {
            migration_error("duplicate transition row for legacy vessel " + std::to_string(id));
        }
    }
    if (probabilities.size() != expected_transition_count) {
        migration_error("expected 23 explicit transition rows, found " +
                        std::to_string(probabilities.size()));
    }
    return probabilities;
}

[[nodiscard]] bool same_point(const std::array<double, 3>& left,
                              const std::array<double, 3>& right) noexcept {
    return left == right;
}

[[nodiscard]] std::vector<std::vector<std::size_t>>
derive_successors(const std::vector<LegacySegment>& segments) {
    std::vector<std::vector<std::size_t>> successors(segments.size());
    for (std::size_t source = 0; source < segments.size(); ++source) {
        for (std::size_t candidate = 0; candidate < segments.size(); ++candidate) {
            if (same_point(segments.at(source).end, segments.at(candidate).start)) {
                successors.at(source).push_back(candidate);
            }
        }
        if (successors.at(source).empty() || successors.at(source).size() > 2) {
            migration_error("legacy vessel " + std::to_string(segments.at(source).id) +
                            " must have one or two coordinate-derived successors");
        }
    }
    return successors;
}

[[nodiscard]] std::vector<SplitProbabilities>
build_splits(const std::vector<LegacySegment>& segments,
             const std::vector<std::vector<std::size_t>>& successors,
             const std::unordered_map<int, SplitProbabilities>& explicit_probabilities) {
    std::vector<SplitProbabilities> splits(segments.size(), {1.0, 0.0});
    std::size_t branch_count{};
    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (successors.at(index).size() == 1) {
            continue;
        }
        ++branch_count;
        const auto id = segments.at(index).id;
        if (id == 9) {
            // Stoquart-ElSankari et al. (2009), cohort means in healthy young
            // adults: left IJV 161 mL/min, right IJV 399 mL/min. The legacy
            // successor order is 81 (left), 83 (right).
            splits.at(index) = {161.0 / 560.0, 399.0 / 560.0};
            continue;
        }
        const auto found = explicit_probabilities.find(id);
        if (found == explicit_probabilities.end()) {
            migration_error("missing transition probabilities for branching legacy vessel " +
                            std::to_string(id));
        }
        splits.at(index) = found->second;
    }
    if (branch_count != 24) {
        migration_error("expected 24 coordinate-derived branches, found " +
                        std::to_string(branch_count));
    }
    return splits;
}

[[nodiscard]] std::vector<double>
derive_flows(const std::vector<LegacySegment>& segments,
             const std::vector<std::vector<std::size_t>>& successors,
             const std::vector<SplitProbabilities>& splits) {
    const auto heart_index = std::size_t{1};          // legacy vessel 2
    const auto venous_return_index = std::size_t{93}; // legacy vessel 94
    std::vector<std::size_t> unprocessed_predecessors(segments.size());
    for (std::size_t source = 0; source < segments.size(); ++source) {
        for (const auto successor : successors.at(source)) {
            const bool closes_circuit = source == venous_return_index && successor == heart_index;
            if (!closes_circuit) {
                ++unprocessed_predecessors.at(successor);
            }
        }
    }

    std::queue<std::size_t> ready;
    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (unprocessed_predecessors.at(index) == 0) {
            ready.push(index);
        }
    }
    if (ready.size() != 1 || ready.front() != heart_index) {
        migration_error("the circulation cannot be linearized at legacy vessel 2");
    }

    std::vector<double> flows(segments.size());
    flows.at(heart_index) = reference_cardiac_output_m3_s;
    std::size_t processed{};
    while (!ready.empty()) {
        const auto source = ready.front();
        ready.pop();
        ++processed;
        for (std::size_t branch = 0; branch < successors.at(source).size(); ++branch) {
            const auto successor = successors.at(source).at(branch);
            if (source == venous_return_index && successor == heart_index) {
                continue;
            }
            flows.at(successor) += flows.at(source) * splits.at(source).at(branch);
            auto& predecessor_count = unprocessed_predecessors.at(successor);
            --predecessor_count;
            if (predecessor_count == 0) {
                ready.push(successor);
            }
        }
    }
    if (processed != segments.size()) {
        migration_error("coordinate graph contains an unexpected cycle");
    }
    if (std::abs(flows.at(venous_return_index) - reference_cardiac_output_m3_s) > 1.0e-12) {
        migration_error("derived flow does not return the full cardiac output to vessel 2");
    }
    return flows;
}

[[nodiscard]] std::string canonical_id(const int legacy_id) {
    std::ostringstream result;
    result << "bvs95-" << std::setfill('0') << std::setw(3) << legacy_id;
    return result.str();
}

[[nodiscard]] VesselType vessel_type(const int legacy_type) {
    if (legacy_type == 0) {
        return VesselType::artery;
    }
    if (legacy_type == 1) {
        return VesselType::vein;
    }
    return VesselType::organ_bed;
}

[[nodiscard]] double reference_velocity(const LegacySegment& segment) noexcept {
    if (segment.id == 2 || segment.id == 58) {
        return 0.05;
    }
    if (segment.type == 0) {
        return 0.10;
    }
    if (segment.type == 1) {
        return 0.037;
    }
    return 0.01;
}

[[nodiscard]] core::Position3D si_position(const std::array<double, 3>& point) {
    return {
        core::meters(point.at(0) * centimeters_to_meters),
        core::meters(point.at(1) * centimeters_to_meters),
        core::meters(point.at(2) * centimeters_to_meters),
    };
}

} // namespace

VascularGraph migrate_legacy_95(const Legacy95MigrationRequest& request) {
    const auto legacy_segments = read_segments(request.vasculature_path);
    const auto explicit_probabilities = read_probabilities(request.transitions_path);
    const auto successors = derive_successors(legacy_segments);
    const auto splits = build_splits(legacy_segments, successors, explicit_probabilities);
    const auto flows = derive_flows(legacy_segments, successors, splits);

    VascularGraph graph{
        supported_vascular_graph_schema_version,
        "bvs95-dissertation-rest",
        "1.0.0",
        "BVS/MEHLISSA 95-segment dissertation rest profile",
        {
            "bvs95-schematic-cartesian",
            "Legacy schematic Cartesian centimetre coordinates converted exactly to metres; "
            "arteries and veins remain on the historical z=+/-0.02 m planes.",
            "not_applicable",
        },
        {
            "historical BVS/MEHLISSA reference woman (1.72 m, 69 kg)",
            "rest; posture not represented; jugular correction uses supine evidence",
            "Reproduction and regression profile. Relative organ perfusion follows the released "
            "dissertation transition table; 6.0 L/min is an explicit normalization assumption. "
            "This profile is not a patient-specific or anatomically validated circulation.",
        },
        {
            {
                "legacy-95-release",
                "MEHLISSA/BloodVoyagerS 95_vasculature.csv and 95_transitions.csv; "
                "released by the project rights holder on 2026-08-27.",
                "CC-BY-4.0",
            },
            {
                "wendt-dissertation-2024",
                "Regine Wendt, Einsatz von Nanotechnologien in der Praezisionsmedizin, "
                "Universitaet zu Luebeck, 2024, Section 4.3.1 and Appendix A.1.",
                "citation-only",
            },
            {
                "jugular-flow-mri-2009",
                "Stoquart-ElSankari et al., A phase-contrast MRI study of physiologic cerebral "
                "venous flow, J Cereb Blood Flow Metab 29 (2009), doi:10.1038/jcbfm.2009.29.",
                "citation-only",
            },
            {
                "bvs95-migration-method",
                "MEHLISSA Next M2.2 deterministic SI migration method, version 1.0.0.",
                "CC-BY-4.0",
            },
        },
        {},
    };

    graph.segments.reserve(legacy_segments.size());
    for (std::size_t index = 0; index < legacy_segments.size(); ++index) {
        const auto& legacy = legacy_segments.at(index);
        const auto start = si_position(legacy.start);
        const auto end = si_position(legacy.end);
        const auto length = core::distance(start, end);
        const auto velocity = reference_velocity(legacy);
        const auto flow = flows.at(index);
        const auto area = flow / velocity;
        const auto diameter = std::sqrt(4.0 * area / std::numbers::pi);
        VascularSegment segment{
            canonical_id(legacy.id),
            vessel_type(legacy.type),
            {
                start,
                end,
                length,
                core::meters(diameter),
                core::square_meters(area),
                core::cubic_meters(area * core::in_meters(length)),
            },
            {
                core::cubic_meters_per_second(flow),
                core::meters_per_second(velocity),
            },
            {},
            {"legacy-95-release", "wendt-dissertation-2024", "bvs95-migration-method"},
            {
                EvidenceQuality::schematic,
                EvidenceQuality::derived,
                EvidenceQuality::derived,
            },
            {},
        };
        if (legacy.id == 9) {
            segment.source_refs.push_back("jugular-flow-mri-2009");
        }
        for (std::size_t branch = 0; branch < successors.at(index).size(); ++branch) {
            segment.transitions.push_back({
                canonical_id(legacy_segments.at(successors.at(index).at(branch)).id),
                splits.at(index).at(branch),
            });
        }
        graph.segments.push_back(std::move(segment));
    }

    validate_vascular_graph(graph);
    return graph;
}

} // namespace mehlissa::models::body

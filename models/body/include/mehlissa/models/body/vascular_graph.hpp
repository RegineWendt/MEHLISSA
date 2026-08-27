// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_VASCULAR_GRAPH_HPP
#define MEHLISSA_MODELS_BODY_VASCULAR_GRAPH_HPP

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/position.hpp>
#include <mehlissa/core/quantity.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::body {

inline constexpr auto supported_vascular_graph_schema_version = "1.0.0";

enum class VesselType : std::uint8_t { artery, vein, organ_bed };
enum class EvidenceQuality : std::uint8_t {
    measured,
    literature,
    derived,
    schematic,
    assumed,
    unknown
};

struct DataSource final {
    std::string id;
    std::string citation;
    std::string license;
};

struct ModelValidity final {
    std::string population;
    std::string physiological_state;
    std::string description;
};

struct CoordinateSystem final {
    std::string id;
    std::string description;
    std::string handedness;
};

struct RelativeUncertainty final {
    std::optional<double> geometry;
    std::optional<double> diameter;
    std::optional<double> flow;
};

struct SegmentGeometry final {
    core::Position3D start;
    core::Position3D end;
    core::Length length;
    core::Length diameter;
    core::Area cross_section_area;
    core::Volume volume;
};

struct SegmentHemodynamics final {
    core::FlowRate flow_rate;
    core::Speed mean_velocity;
};

struct Transition final {
    std::string successor_id;
    double probability{};
};

struct SegmentEvidence final {
    EvidenceQuality geometry{EvidenceQuality::unknown};
    EvidenceQuality diameter{EvidenceQuality::unknown};
    EvidenceQuality flow{EvidenceQuality::unknown};
};

struct VascularSegment final {
    std::string id;
    VesselType type{VesselType::artery};
    SegmentGeometry geometry;
    SegmentHemodynamics hemodynamics;
    std::vector<Transition> transitions;
    std::vector<std::string> source_refs;
    SegmentEvidence evidence;
    RelativeUncertainty relative_uncertainty;
};

struct VascularGraph final {
    std::string schema_version;
    std::string model_id;
    std::string model_version;
    std::string title;
    CoordinateSystem coordinate_system;
    ModelValidity validity;
    std::vector<DataSource> sources;
    std::vector<VascularSegment> segments;

    [[nodiscard]] const VascularSegment* find_segment(std::string_view id) const noexcept;
};

struct VascularGraphLoadRequest final {
    std::filesystem::path model_path;
    std::filesystem::path schema_path;
};

class VascularGraphError final : public core::MehlissaError {
  public:
    using core::MehlissaError::MehlissaError;
};

[[nodiscard]] VascularGraph load_vascular_graph(const VascularGraphLoadRequest& request);
void validate_vascular_graph(const VascularGraph& graph);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_VASCULAR_GRAPH_HPP

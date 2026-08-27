// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_BVS_REFERENCE_HPP
#define MEHLISSA_MODELS_BODY_BVS_REFERENCE_HPP

#include <mehlissa/models/body/vascular_graph.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::body {

inline constexpr auto supported_bvs_reference_report_schema_version = "1.0.0";

struct DistributionComparison final {
    double mean_absolute_difference_per_segment{};
    double normalized_mean_difference_percent{};
    double maximum_absolute_difference{};
};

struct PerfusionComparison final {
    std::string region;
    std::string segment_id;
    double literature_target_percent{};
    double dissertation_simulation_percent{};
    double model_percent{};
};

struct BvsReferenceReport final {
    std::string schema_version;
    std::string model_id;
    std::string model_version;
    std::uint64_t master_seed{};
    std::uint64_t reference_particle_count{};
    std::uint64_t large_particle_count{};
    std::string aorta_segment_id;
    std::string popliteal_segment_id;
    DistributionComparison minute_1_vs_minute_120;
    DistributionComparison minute_7_vs_minute_120;
    DistributionComparison minute_15_vs_minute_120;
    DistributionComparison aorta_vs_popliteal_at_minute_7;
    double population_scale_total_variation_percent{};
    double mean_perfusion_error_vs_literature_percentage_points{};
    double maximum_perfusion_error_vs_literature_percentage_points{};
    double mean_perfusion_difference_vs_dissertation_percentage_points{};
    std::vector<PerfusionComparison> perfusion;
    std::uint64_t aorta_reference_transitions{};
    std::uint64_t popliteal_transitions{};
    std::uint64_t large_population_transitions{};
    bool population_conserved{};
    bool perfusion_gate_passed{};
    bool equilibrium_gate_passed{};
    bool injection_site_gate_passed{};
    bool population_scale_gate_passed{};
    bool overall_passed{};
};

struct BvsReferenceReportWriteRequest final {
    std::filesystem::path output_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] BvsReferenceReport run_bvs_reference(const VascularGraph& graph,
                                                   std::uint64_t master_seed = 2'018);
void write_bvs_reference_report(const BvsReferenceReport& report,
                                const BvsReferenceReportWriteRequest& request);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_BVS_REFERENCE_HPP

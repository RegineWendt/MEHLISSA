// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_COUPLER_HPP
#define MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_COUPLER_HPP

#include <mehlissa/models/cell/receptor_ligand_model.hpp>
#include <mehlissa/models/coupling/extracellular_signal.hpp>

#include <string>
#include <unordered_set>

namespace mehlissa::models::cosimulation {

struct CapillaryCellSignalCouplerConfig final {
    std::string profile_id;
    std::string source_model_id;
    std::string source_compartment_id;
    std::string signal_id;
    core::Volume represented_volume{};
    std::string target_cell_model_id;
    std::string ligand_id;
    std::string target_compartment_id;
    core::SimulationClock::Duration exposure_duration{};
    double initial_bound_fraction{};
};

struct CapillaryCellSignalEvaluation final {
    std::string profile_id;
    coupling::ExtracellularSignalSample source_sample;
    cell::ReceptorLigandResponse cell_response;
};

void validate_capillary_cell_signal_coupler_config(const CapillaryCellSignalCouplerConfig& config);

class CapillaryCellSignalCoupler final {
  public:
    explicit CapillaryCellSignalCoupler(CapillaryCellSignalCouplerConfig config);

    [[nodiscard]] CapillaryCellSignalEvaluation
    evaluate(const coupling::ExtracellularSignalSource& source,
             const cell::ReceptorLigandModel& cell_model, std::string sample_id,
             core::SimulationClock::Duration observed_at);

    [[nodiscard]] std::size_t completed_sample_count() const noexcept;

  private:
    CapillaryCellSignalCouplerConfig config_;
    std::unordered_set<std::string> completed_sample_ids_;
};

} // namespace mehlissa::models::cosimulation

#endif // MEHLISSA_MODELS_COSIMULATION_CAPILLARY_CELL_SIGNAL_COUPLER_HPP

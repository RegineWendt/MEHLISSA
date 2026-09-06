// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cosimulation/dynamic_capillary_tissue_cell_model.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

namespace cosim = mehlissa::models::cosimulation;
namespace core = mehlissa::core;

void apply_variation(cosim::DynamicCapillaryTissueCellParameters& parameters,
                     cosim::DynamicCapillaryTissueCellInitialState& initial_state,
                     const std::string& name, const double factor) {
    if (!std::isfinite(factor) || factor < 0.0) {
        throw std::invalid_argument{"variation factor must be finite and nonnegative"};
    }
    if (name == "zero_flux") {
        parameters.blood_to_endothelium = core::per_second(0.0);
        parameters.endothelium_to_blood = core::per_second(0.0);
        parameters.endothelium_to_interstitium = core::per_second(0.0);
        parameters.interstitium_to_endothelium = core::per_second(0.0);
        parameters.blood_outlet = core::per_second(0.0);
        parameters.interstitial_clearance = core::per_second(0.0);
    } else if (name == "constant_reservoir") {
        parameters.blood_to_endothelium = core::per_second(0.0);
        parameters.endothelium_to_blood = core::per_second(0.0);
        parameters.endothelium_to_interstitium = core::per_second(0.0);
        parameters.interstitium_to_endothelium = core::per_second(0.0);
        parameters.blood_outlet = core::per_second(0.0);
        parameters.interstitial_clearance = core::per_second(0.0);
        parameters.internalization = core::per_second(0.0);
        parameters.degradation = core::per_second(0.0);
        initial_state.blood_free = core::moles(0.0);
        initial_state.interstitium_free = initial_state.declared_initial_amount;
    } else if (name == "blood_to_endothelium") {
        parameters.blood_to_endothelium =
            core::per_second(core::in_per_second(parameters.blood_to_endothelium) * factor);
    } else if (name == "endothelium_to_blood") {
        parameters.endothelium_to_blood =
            core::per_second(core::in_per_second(parameters.endothelium_to_blood) * factor);
    } else if (name == "endothelium_to_interstitium") {
        parameters.endothelium_to_interstitium =
            core::per_second(core::in_per_second(parameters.endothelium_to_interstitium) * factor);
    } else if (name == "interstitium_to_endothelium") {
        parameters.interstitium_to_endothelium =
            core::per_second(core::in_per_second(parameters.interstitium_to_endothelium) * factor);
    } else if (name == "blood_outlet") {
        parameters.blood_outlet =
            core::per_second(core::in_per_second(parameters.blood_outlet) * factor);
    } else if (name == "interstitial_clearance") {
        parameters.interstitial_clearance =
            core::per_second(core::in_per_second(parameters.interstitial_clearance) * factor);
    } else if (name == "association") {
        parameters.association = core::cubic_meters_per_mole_second(
            core::in_cubic_meters_per_mole_second(parameters.association) * factor);
    } else if (name == "dissociation") {
        parameters.dissociation =
            core::per_second(core::in_per_second(parameters.dissociation) * factor);
    } else if (name == "internalization") {
        parameters.internalization =
            core::per_second(core::in_per_second(parameters.internalization) * factor);
    } else if (name == "degradation") {
        parameters.degradation =
            core::per_second(core::in_per_second(parameters.degradation) * factor);
    } else if (name == "receptor_capacity") {
        parameters.receptor_capacity =
            core::moles(core::in_moles(parameters.receptor_capacity) * factor);
    } else if (name == "feedback_gain") {
        parameters.feedback_gain *= factor;
    } else if (name == "nrp1_facilitation") {
        parameters.nrp1_mode = cosim::Nrp1StructuralMode::facilitated_binding_assumption;
        parameters.nrp1_association_multiplier = factor;
    } else if (name == "nrp1_excluded") {
        parameters.nrp1_mode = cosim::Nrp1StructuralMode::excluded;
        parameters.coreceptor_id.clear();
        parameters.nrp1_site_fraction = 0.0;
        parameters.nrp1_association_multiplier = 1.0;
    } else {
        throw std::invalid_argument{"unknown DCCQ variation: " + name};
    }
}

void write_csv(const std::filesystem::path& path,
               const std::vector<cosim::DynamicCapillaryTissueCellSnapshot>& samples) {
    std::ofstream stream{path, std::ios::binary};
    if (!stream.good()) {
        throw std::runtime_error{"cannot open DCCQ output CSV"};
    }
    stream << "time_s,initial_mol,cumulative_inlet_mol,blood_free_mol,"
              "endothelium_free_mol,interstitium_free_mol,receptor_bound_mol,"
              "internalized_mol,cleared_or_degraded_mol,cumulative_outlet_mol,"
              "occupancy_fraction,applied_feedback_multiplier,"
              "scheduled_feedback_multiplier,balance_error_mol\n"
           << std::setprecision(17);
    for (const auto& sample : samples) {
        const auto& ledger = sample.ledger;
        stream << core::in_seconds(sample.time) << ',' << core::in_moles(ledger.initial_amount)
               << ',' << core::in_moles(ledger.cumulative_inlet) << ','
               << core::in_moles(ledger.blood_free) << ','
               << core::in_moles(ledger.endothelium_free) << ','
               << core::in_moles(ledger.interstitium_free) << ','
               << core::in_moles(ledger.receptor_bound) << ','
               << core::in_moles(ledger.internalized) << ','
               << core::in_moles(ledger.cleared_or_degraded) << ','
               << core::in_moles(ledger.cumulative_outlet) << ','
               << sample.receptor_occupancy_fraction << ',' << sample.applied_feedback_multiplier
               << ',' << sample.scheduled_feedback_multiplier << ','
               << cosim::dynamic_balance_error_moles(ledger) << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 5 && argc != 7) {
            std::cerr << "usage: mehlissa_dccq1_qualification_runner output.csv "
                         "internal_step_s synchronization_s duration_s [variation factor]\n";
            return EXIT_FAILURE;
        }
        auto parameters = cosim::dccq1_reference_parameters();
        auto initial_state = cosim::dccq1_reference_initial_state();
        parameters.internal_step = core::seconds(std::stod(argv[2]));
        parameters.synchronization_interval = core::seconds(std::stod(argv[3]));
        if (argc == 7) {
            apply_variation(parameters, initial_state, argv[5], std::stod(argv[6]));
        }
        cosim::DynamicCapillaryTissueCellModel model{parameters, initial_state};
        const auto samples = model.run(core::seconds(std::stod(argv[4])));
        write_csv(std::filesystem::path{argv[1]}, samples);
        std::cout << "model=" << parameters.model_id << " points=" << samples.size()
                  << " max_balance_error_mol=";
        double maximum_error = 0.0;
        for (const auto& sample : samples) {
            maximum_error =
                std::max(maximum_error, cosim::dynamic_balance_error_moles(sample.ledger));
        }
        std::cout << std::setprecision(17) << maximum_error << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "DCCQ-1 qualification run failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

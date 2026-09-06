// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/qualified_cd95_apoptosis_model.hpp>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

namespace cell = mehlissa::models::cell;

[[nodiscard]] cell::KallenbergerCase source_case(const std::string& value) {
    if (value == "523") {
        return cell::KallenbergerCase::cd95_hela;
    }
    if (value == "524") {
        return cell::KallenbergerCase::wild_type_hela;
    }
    throw std::invalid_argument{"source case must be 523 or 524"};
}

[[nodiscard]] cell::KallenbergerMinimalParameters
varied_parameters(cell::KallenbergerMinimalParameters parameters, const std::string& name,
                  const double factor) {
    const std::array entries{
        std::pair{"kon_FADD", &cell::KallenbergerMinimalParameters::kon_fadd},
        std::pair{"koff_FADD", &cell::KallenbergerMinimalParameters::koff_fadd},
        std::pair{"kDISC", &cell::KallenbergerMinimalParameters::kdisc},
        std::pair{"kD216", &cell::KallenbergerMinimalParameters::kd216},
        std::pair{"kD374trans_p55", &cell::KallenbergerMinimalParameters::kd374trans_p55},
        std::pair{"kD374trans_p43", &cell::KallenbergerMinimalParameters::kd374trans_p43},
        std::pair{"kdiss_p18", &cell::KallenbergerMinimalParameters::kdiss_p18},
        std::pair{"kBid", &cell::KallenbergerMinimalParameters::kbid},
        std::pair{"kD374probe", &cell::KallenbergerMinimalParameters::kd374probe},
        std::pair{"KDR", &cell::KallenbergerMinimalParameters::kdr},
        std::pair{"KDL", &cell::KallenbergerMinimalParameters::kdl}};
    for (const auto& [candidate, member] : entries) {
        if (candidate == name) {
            parameters.*member *= factor;
            return parameters;
        }
    }
    throw std::invalid_argument{"unknown parameter variation"};
}

void write_csv(const std::filesystem::path& path,
               const std::vector<cell::QualifiedCd95Sample>& samples) {
    std::ofstream stream{path, std::ios::binary};
    if (!stream.good()) {
        throw std::runtime_error{"cannot open output CSV"};
    }
    stream << "model_time";
    for (std::size_t i = 0; i < cell::kallenberger_species_count; ++i) {
        stream << ',' << cell::species_id(static_cast<cell::KallenbergerSpecies>(i));
    }
    stream << '\n' << std::setprecision(17);
    for (const auto& sample : samples) {
        stream << sample.model_time.value;
        for (const auto value : sample.state.values) {
            stream << ',' << value;
        }
        stream << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 4 && argc != 6) {
            std::cerr << "usage: mehlissa_bcq1_qualification_runner 523|524 step output.csv "
                         "[parameter factor]\n";
            return EXIT_FAILURE;
        }
        const auto selected_case = source_case(argv[1]);
        const auto step = std::stod(argv[2]);
        const auto output = std::filesystem::path{argv[3]};
        const auto& definition = cell::kallenberger_minimal_definition(selected_case);
        auto parameters = definition.parameters;
        if (argc == 6) {
            parameters = varied_parameters(parameters, argv[4], std::stod(argv[5]));
        }
        const cell::KallenbergerMinimalMechanism mechanism{parameters};
        std::size_t integration_steps = 0;
        const auto samples = mechanism.integrate(definition.initial_state, {240.0}, {0.25}, {step},
                                                 integration_steps);
        write_csv(output, samples);
        std::cout << "accession=" << definition.source.accession << " points=" << samples.size()
                  << " integration_steps=" << integration_steps << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "BCQ-1 qualification run failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

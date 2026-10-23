/*
 * Copyright (c) 2025 Universität zu Lübeck [WENDT] and Technische Universität
 * Berlin [DEBUS] This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Author: Regine Wendt <regine.wendt@uni-luebeck.de>
 * Author: Lisa Y. Debus <debus@ccs-labs.org>
 */

#include "../bloodcircuit/BloodCircuit.h"
#include "../utils/Logger.h"
#include "Simulator.h"
#include <boost/program_options.hpp>
#include <iostream>

using namespace std;
using namespace experiments;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    try {
        clock_t start, finish;

        int simulationDuration;
        double simStep;

        int numOfParticles;
        int numOfCollectors;
        int numOfLocators;
        int numCancerCells;
        int numCarTCells;
        int numTCells;
        // Print Mode -
        // Defaul: 0, will output detailed information per particle
        // Set mode to one if you want to simulate ldl particles,
        // and to two if you want so simulate liquid biopsy.
        // set to 3 for vessel summaries
        int printMode;
        int printInterval;

        int injectionVessel;
        double injectionTime;
        int detectionVessel;

        bool isDeterministic;
        string networkFile;
        string transitionsFile;
        string fingerprintFile;
        int parallel;
        int logLevel;
        string simFile;
        string gwFile;
        int seed;

        po::options_description desc("Allowed options");
        desc.add_options()("help",
                           "produce help message") // TODO: add help message
            ("simulationStep", po::value<double>(&simStep)->default_value(1),
             "simulationStep") //
            ("simulationDuration",
             po::value<int>(&simulationDuration)->default_value(10),
             "simulationDuration") //
            ("numOfParticles",
             po::value<int>(&numOfParticles)->default_value(0),
             "numOfParticles") //
            ("numOfCollectors",
             po::value<int>(&numOfCollectors)->default_value(0),
             "numOfCollectors") //
            ("numOfLocators", po::value<int>(&numOfLocators)->default_value(0),
             "numOfLocators") //
            ("numCancerCells",
             po::value<int>(&numCancerCells)->default_value(0),
             "numCancerCells") //
            ("numCarTCells", po::value<int>(&numCarTCells)->default_value(0),
             "numCarTCells") //
            ("numTCells", po::value<int>(&numTCells)->default_value(0),
             "numTCells") //
            ("printMode", po::value<int>(&printMode)->default_value(0),
             "printMode") //
            ("printInterval", po::value<int>(&printInterval)->default_value(60),
             "printInterval") //
            ("injectionVessel",
             po::value<int>(&injectionVessel)->default_value(1),
             "injectionVessel") //
            ("injectionTime",
             po::value<double>(&injectionTime)->default_value(10),
             "injectionTime") //
            ("detectionVessel",
             po::value<int>(&detectionVessel)->default_value(23),
             "detectionVessel") //
            ("isDeterministic",
             po::value<bool>(&isDeterministic)->default_value(true),
             "isDeterministic") //
            ("parallel", po::value<int>(&parallel)->default_value(1),
             "parallel") //
            ("simFile",
             po::value<string>(&simFile)->default_value(
                 "../output/csvnano.csv"),
             "simFile") //
            ("gwFile",
             po::value<string>(&gwFile)->default_value(
                 "../output/gwDetect.csv"),
             "gwFile") //
            ("networkFile",
             po::value<string>(&networkFile)
                 ->default_value("../data/95_vasculature.csv"),
             "networkFile") //
            ("transitionsFile",
             po::value<string>(&transitionsFile)
                 ->default_value("../data/95_transitions.csv"),
             "transitionsFile") //
            ("fingerprintFile",
             po::value<string>(&fingerprintFile)
                 ->default_value("../data/95_fingerprints.csv"),
             "fingerprintFile") //
            ("logLevel", po::value<int>(&logLevel)->default_value(2),
             "logLevel")                                               //
            ("seed", po::value<int>(&seed)->default_value(-1), "seed") //
            ;
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            Logger::LogInfo(
                "\n============================================================"
                "================"
                "====================================\n\n"
                "MEHLISSA simulates the global movement of objects through the "
                "human "
                "circulatory system.\nCreated by Universität zu Lübeck and "
                "Technische Universität Berlin.\n"
                "\nYou can define the simulation via the following command "
                "line parameters:\n\n"
                "\tsimulationStep (TYPE double, default_value 1)\n"
                "\tsimulationDuration (TYPE int, DEFAULT 10)\n"
                "\tnumOfParticles (TYPE int, DEFAULT 0)\n"
                "\tnumOfCollectors (TYPE int, DEFAULT 0)\n"
                "\tnumOfLocators (TYPE int, DEFAULT 0)\n"
                "\tnumCancerCells (TYPE int, DEFAULT 0)\n"
                "\tnumCarTCells (TYPE int, DEFAULT 0)\n"
                "\tnumTCells (TYPE int, DEFAULT 0)\n"
                "\tinjectionVessel (TYPE int, DEFAULT 1)\n"
                "\tinjectionTime (TYPE double, DEFAULT 10)\n"
                "\tdetectionVessel (TYPE int, DEFAULT 23)\n"
                "\tisDeterministic (TYPE bool, DEFAULT true)\n"
                "\tparallel (TYPE int, DEFAULT 1)\n"
                "\tlog level (TYPE int, DEFAULT 2)\n"
                "\tseed (TYPE int, DEFAULT -1, overrides isDetermnistic = "
                "true)\n"
                "\tprintMode (TYPE int, DEFAULT 0)\n"
                "\tprintInterval (TYPE int, DEFAULT 60)\n"
                "\tsimFile (TYPE string, DEFAULT '../output/csvnano.csv')\n"
                "\tgwFile (TYPE string, DEFAULT '../output/gwDetect.csv')\n"
                "\tnetworkFile (TYPE string, DEFAULT "
                "'../data/95_vasculature.csv')\n"
                "\ttransitionsFile (TYPE string, DEFAULT "
                "'../data/95_transitions.csv')\n"
                "\tfingerprintFile (TYPE string, DEFAULT "
                "'../data/95_fingerprints.csv')\n"
                "\nExample programm call:\n\t../bin/Mehlissa "
                "--simulationDuration=43200 "
                "--networkFile='../data/example_vasculature.csv' \n"
                "\n============================================================"
                "================"
                "===================================="
                "");
            return 1;
        }

        Randomizer::SetSeed(seed);
        Logger::SetLogLevel(logLevel);
        BloodCircuit::SetVasculature(networkFile, transitionsFile,
                                     fingerprintFile);
        shared_ptr<BloodCircuit> circuit = BloodCircuit::CreateBloodCircuit(
            isDeterministic, printMode, printInterval, simFile, gwFile,
            injectionTime, injectionVessel, detectionVessel, simulationDuration,
            numOfParticles, numOfCollectors, numOfLocators, numCancerCells,
            numCarTCells, numTCells);

        Simulator simulator(parallel, simStep, circuit);
        start = clock();
        simulator.Simulate(simulationDuration);
        finish = clock();
        Logger::LogInfo("======================================================"
                        "==============");
        Logger::LogInfo(string("Time total: ") +
                        to_string((finish - start) / 1000000) + "s");
        Logger::LogInfo(
            string("Simulation Parameters: \n\t") + to_string(numOfParticles) +
            " Particle, \n\t" + to_string(numOfLocators) +
            " Nanolocators, \n\t" + to_string(numOfCollectors) +
            " Nanocollectors, \n\t" + to_string(numCancerCells) +
            " Cancer Cells, \n\t" + to_string(numCarTCells) +
            " CAR-T Cells, \n\t" + to_string(numTCells) + " T Cells" +
            "\n+ Injection Vessel: " + to_string(injectionVessel) +
            "\n+ Injection Time: " + to_string(injectionTime) +
            "\n+ Detection vessel: " + to_string(detectionVessel) +
            " s \nSimulated Time: " + to_string(simulationDuration) + " s");
        Logger::LogInfo("======================================================"
                        "==============");

    } catch (const exception &e) {
        Logger::LogError(string("Exception ") + e.what() + "");
        return 1;
    }
}

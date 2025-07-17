/*
 * Copyright (c) 2025 Universität zu Lübeck [WENDT] and Technische Universität Berlin [DEBUS]
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
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

#include "Simulator.h"
#include "../bloodcircuit/BloodCircuit.h"
#include <iostream>
//#include "../libs/boost_1_82_0/boost/program_options.hpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace experiments;
namespace po = boost::program_options;



int
main (int argc, char *argv[])
{
    try {
        clock_t start, finish;
        // default values
        int numCancerCells;
        int numCarTCells;
        int numTCells;
        int simulationDuration;
        double simStep;
        double injectionTime;
        int injectionVessel;
        int detectionVessel;
        bool isDeterministic;
        int parallel;
        string simFile;
        string gwFile;
        string networkFile;
        string transitionsFile;
        string fingerprintFile;

        po::options_description desc("Allowed options");
        desc.add_options()
            //("help", "produce help message") // TODO: add help message
            ("numCancerCells", po::value<int>(&numCancerCells)->default_value(100), "numCancerCells")
            ("numCarTCells", po::value<int>(&numCarTCells)->default_value(100), "numCarTCells")
            ("numTCells", po::value<int>(&numTCells)->default_value(100), "numTCells")
            ("simulationStep", po::value<double>(&simStep)->default_value(1), "simulationStep")
            ("simulationDuration", po::value<int>(&simulationDuration)->default_value(100), "simulationDuration")
            ("injectionTime", po::value<double>(&injectionTime)->default_value(20), "injectionTime")
            ("injectionVessel", po::value<int>(&injectionVessel)->default_value(29), "injectionVessel")
            ("detectionVessel", po::value<int>(&detectionVessel)->default_value(23), "detectionVessel")
            ("isDeterministic", po::value<bool>(&isDeterministic)->default_value(true), "isDeterministic")
            ("parallel", po::value<int>(&parallel)->default_value(1), "parallel")
            ("simFile", po::value<string>(&simFile)->default_value("csvnano.csv"), "simFile")
            ("gwFile", po::value<string>(&gwFile)->default_value("gwDetect.csv"), "gwFile")
            ("networkFile", po::value<string>(&networkFile)->default_value("../data/95_vasculature.csv"), "networkFile")
            ("transitionsFile", po::value<string>(&transitionsFile)->default_value("../data/95_transitions.csv"), "transitionsFile")
            ("fingerprintFile", po::value<string>(&fingerprintFile)->default_value("../data/95_fingerprint.csv"), "fingerprintFile")
        ;
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        BloodCircuit::SetVasculature(networkFile, transitionsFile, fingerprintFile);
        shared_ptr<BloodCircuit> circuit =  BloodCircuit::CancerSimulation(numCancerCells,
                                                               numCarTCells,
                                                               numTCells,
                                                               simulationDuration,
                                                               injectionTime,
                                                               injectionVessel, 
                                                               detectionVessel,
                                                               isDeterministic,
                                                               simFile,
                                                               gwFile);

        Simulator simulator(parallel, simStep, circuit);
        start = clock();
        simulator.Simulate(simulationDuration);
        finish = clock();
        cout << "Time total: " << simulationDuration << "s " << endl;
        cout << numCancerCells << " cancer cells, "
             << numCarTCells << " CAR-T cells, "
             << numTCells  << " T cells -> " << (finish - start) / 1000000
             << "s ------------------------" << endl;
        cout << "Injection Vessel: " << injectionVessel << endl;
        cout << "Injection Time: " << injectionTime << endl;
  
    } catch (const exception &e) {
        cout << "Exception " << e.what() << endl;
        return 1;
    }
}


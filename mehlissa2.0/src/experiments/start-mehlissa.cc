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


int main (int argc, char *argv[])
{
    try {
        clock_t start, finish;
        
        int numOfParticles;
        int simulationDuration;
        int injectionVessel;
        int numOfCollectors;
        int numOfLocators;
        //Particle Mode - Set mode to one if you want to simulate ldl particles,
        //and to two if you want so simulate liquid biopsy.
        //See BloodCircuit.cc Line 197ff for the setup of both scenarios
        int particleMode;
        bool isDeterministic;
        string networkFile;
        string transitionsFile;
        string fingerprintFile;
        double simStep;
        int injectionVessel;
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
            ("simulationStep", po::value<double>(&simStep)->default_value(1), "simulationStep")
            ("simulationDuration", po::value<int>(&simulationDuration)->default_value(10), "simulationDuration")
            ("numOfParticles", po::value<int>(&numOfParticles)->default_value(10), "numOfParticles")
            ("numOfCollectors", po::value<int>(&numOfCollectors)->default_value(100), "numOfCollectors")
            ("numOfLocators", po::value<int>(&numOfLocators)->default_value(0), "numOfLocators")
            ("particleMode", po::value<int>(&particleMode)->default_value(0), "particleMode")
            ("injectionVessel", po::value<int>(&injectionVessel)->default_value(1), "injectionVessel")
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
        shared_ptr<BloodCircuit> circuit =  BloodCircuit::BeginSimulation(simulationDuration,
                                                                          numOfParticles,
                                                                          injectionVessel,
                                                                          numOfCollectors,
                                                                          numOfLocators,
                                                                          particleMode,
                                                                          isDeterministic);

        Simulator simulator(parallel, simStep, circuit);
        start = clock();
        simulator.Simulate(simulationDuration);
        finish = clock();
        cout << "Time total: " << simulationDuration << "s " << endl;
        cout << numOfParticles << " Particle, "
             << numOfLocators << " Nanolocators, "
             << numOfCollectors << " Nanocollectors, "
             << "  -> " << (finish - start) / 1000000
             << "s ------------------------" << endl;
        cout << "Injection Vessel: " << injectionVessel << endl;
  
    } catch (const exception &e) {
        cout << "Exception " << e.what() << endl;
        return 1;
    }
}

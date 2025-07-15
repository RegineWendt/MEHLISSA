/*
 * Copyright (c) 2024 Technische Universität Berlin [DEBUS]
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
 * Author: Lisa Y. Debus <debus@ccs-labs.org>
 */

#include "ns3/Bloodcircuit.h"

using namespace ns3;
using namespace std;

int
main (int argc, char *argv[])
{
    try {
          // default values
        int numCancerCells = 100;
        int numCarTCells = 100;
        int numTCells = 100;
        int simulationDuration = 100;
        int injectionTime = 20;
        int injectionVessel = 29;
        int detectionVessel = 23;
        bool isDeterministic = true;
        string simFile = "csvNano.csv";
        string gwFile = "gwDetect.csv";

        CommandLine cmd;
        cmd.AddValue ("numCancerCells", "numCancerCells", numCancerCells);
        cmd.AddValue ("numCarTCells", "numCarTCells", numCarTCells);
        cmd.AddValue ("numTCells", "numTCells", numTCells);
        cmd.AddValue ("simulationDuration", "simulationDuration", simulationDuration);
        cmd.AddValue ("injectionTime", "injectionTime", injectionTime);
        cmd.AddValue ("injectionVessel", "injectionVessel", injectionVessel);
        cmd.AddValue ("detectionVessel", "detectionVessel", detectionVessel);
        cmd.AddValue ("isDeterministic", "isDeterministic", isDeterministic);
        cmd.AddValue ("simFile", "simFile", simFile);
        cmd.AddValue ("gwFile", "gwFile", gwFile);

        cmd.Parse (argc, argv);
        return Bloodcircuit::CancerSimulation(numCancerCells,
                                            numCarTCells,
                                            numTCells,
                                            simulationDuration,
                                            injectionTime,
                                            injectionVessel, 
                                            detectionVessel,
                                            isDeterministic,
                                            simFile,
                                            gwFile);
    } catch (const exception &e) {
        cout << "Exception " << e.what() << endl;
        return 1;
    }
}


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

#ifndef CLASS_PRINTNANOBOT_
#define CLASS_PRINTNANOBOT_

#include "../bloodcircuit/Bloodstream.h"
#include <iostream> 
#include <fstream> 
#include <vector> 

using namespace std;
using namespace bloodcircuit;

namespace utils {

class Printer {
private:
    ofstream output;
    ofstream gwOutput;
    int particlePrintMode;

public:
    Printer(int particleMode);
    Printer(int particleMode, string simFile, string gwFile);

    ~Printer();

    /// Prints one nanobot to a csv file.
    void PrintParticle(shared_ptr<Particle> n, int vesselID);

    /// Prints transposed/translated nanobots in the BloodVessel to a csv file.
    void PrintParticles(list<shared_ptr<Particle>> nbl, int vesselID);

    void PrintGateway(int vesselID, int cancerCellNumber, int carTCellNumber);

    // Debug Function, currently not used
    void PrintInTerminal(vector<shared_ptr<Bloodstream>> streamsOfVessel,
                         int vesselIDl);
};
}; // namespace utils
#endif

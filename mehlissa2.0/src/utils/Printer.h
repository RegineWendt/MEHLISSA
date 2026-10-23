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

#ifndef CLASS_PRINTNANOBOT_
#define CLASS_PRINTNANOBOT_

#include "../bloodcircuit/Bloodstream.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace bloodcircuit;

namespace utils {

class Printer {
private:
    ofstream output;
    ofstream gwOutput;
    int particlePrintMode; // default 0
    int interval;          // printing interval [s]

public:
    Printer(int particleMode = 0, int printInterval = 60);
    Printer(string simFile, string gwFile, int particleMode = 0,
            int printInterval = 60);

    ~Printer();

    /// Prints one nanobot to a csv file.
    void PrintParticle(shared_ptr<Particle> n, int vesselID);

    /// Prints transposed/translated nanobots in the BloodVessel to a csv file.
    void PrintParticles(list<shared_ptr<Particle>> nbl, int vesselID);
    void PrintVessel(list<shared_ptr<Particle>> nbl, int vesselID);

    /// Prints the given outputString in the gateway file
    void PrintGateway(int vesselID, string outputString);

    /// Debug function, currently not used
    void PrintInTerminal(vector<shared_ptr<Bloodstream>> streamsOfVessel,
                         int vesselIDl);
    void CloseFile();
};
}; // namespace utils
#endif

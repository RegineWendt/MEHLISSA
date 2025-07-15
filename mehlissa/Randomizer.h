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

#ifndef H_RANDOMIZER_
#define H_RANDOMIZER_

#include "ns3/core-module.h"
#include "ns3/object.h"
#include <cstdint>
#include <random>

using namespace std;

namespace ns3 {
class Randomizer {
private:
    static uint32_t m_seed;
    static uint32_t m_run;
    static Ptr<UniformRandomVariable> stream_bool;
    static Ptr<UniformRandomVariable> stream_0_1;

public:
    static void InitRandomizer(bool isDeterministic);

    // Will return either true or false randomly
    static bool GetRandomBoolean();

    // Will return a random value between 0 and 1.
    static double GetRandomValue();

    static Ptr<UniformRandomVariable> GetNewRandomStream(double min,
                                                         double max);
};
}; // namespace ns3
#endif

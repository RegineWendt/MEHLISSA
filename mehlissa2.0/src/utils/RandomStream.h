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

#ifndef H_RANDOMSTREAM_
#define H_RANDOMSTREAM_

#include <iostream>
#include <cstdint>
#include <random>

using namespace std;

namespace utils {
class RandomStream {
private:
    double min;
    double max;
    unsigned int m_seed;
    mt19937 rnd_stream;
    uniform_real_distribution<> uni_stream;

public:
    RandomStream(unsigned int seed, double min, double max);
    ~RandomStream();
    
    double GetValue();
};
}; // namespace utils
#endif

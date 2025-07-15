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

#include "Randomizer.h"

namespace utils {
static unsigned int m_seed;
static mt19937 rnd_stream;
static uniform_real_distribution stream_bool;
static uniform_real_distribution stream_0_1;

void Randomizer::InitRandomizer(bool isDeterministic) {
    std::random_device rnd = std::random_device();
    if (isDeterministic == true)
        m_seed = 1;
    else
        m_seed = rnd();
    mt19937 rnd_stream(m_seed);    
    uniform_real_distribution stream_bool(0.0, 1.0);
    uniform_real_distribution stream_0_1(0.0, 1.0);
}

bool Randomizer::GetRandomBoolean() {
    double value = stream_bool(rnd_stream);
    return value >= 0.5;
}

double Randomizer::GetRandomValue() {
    double value = stream_0_1(rnd_stream);
    return value;
}

double Randomizer::GetRandomValue(double min, double max) {
    double value = stream_0_1(rnd_stream);
    value *= max;
    value += min;
    return value;
}

int Randomizer::GetRandomIntegerValue(int min, int max) {
    double value = stream_0_1(rnd_stream);
    if (value == 1) value = value - 0.01;
    value *= max + 1;
    value += min;
    return std::floor(value);
}

shared_ptr<RandomStream> Randomizer::GetNewRandomStream(double min, double max) {
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(m_seed, min, max);
    return rs;
}
} // namespace utils

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

#include "RandomStream.h"

namespace utils {

RandomStream::RandomStream(unsigned int seed, double min, double max) {
    this->min = min;
    this->max = max;
    this->m_seed = seed;
    mt19937 rnd_stream(m_seed);    
    uniform_real_distribution uni_stream(0.0, 1.0);
}

RandomStream::~RandomStream() {
}
    
double RandomStream::GetValue() {
    double value = uni_stream(rnd_stream);
    value *= max;
    value += min;
    return value;
}
} // namespace utils


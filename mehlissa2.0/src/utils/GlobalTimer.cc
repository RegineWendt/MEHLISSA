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

#include "GlobalTimer.h"

namespace utils {

static uint64_t m_time; // in seconds, so basically unixtime as simulationtime

GlobalTimer::GlobalTimer() {}
GlobalTimer::~GlobalTimer() {}

void GlobalTimer::ResetTimer() {
    m_time = 0;
}

void GlobalTimer::IncreaseTimer(double step) {
    m_time += (step * 1000);
}

double GlobalTimer::NowInSeconds() {
    return m_time/1000;
}
} // namespace utils


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

#include "Nanocollector.h"

namespace particles {

Nanocollector::Nanocollector() {
    particleType = NanocollectorType;
    m_length = 0.000001; // 10nm
    m_width = 0.000001;  // 10nm
    m_delay = 0.5;       // with Jorge 0.044 delay and 10nm x 10nm
    m_tissueDetected = false;
}

Nanocollector::~Nanocollector() {}

double Nanocollector::GetDelay() { return m_delay; }

void Nanocollector::SetDelay(double value) {
    if (value < 0)
        value = 0;
    m_delay = value;
}

int Nanocollector::GetTargetOrgan() { return m_targetOrgan; }

void Nanocollector::SetTargetOrgan(int target) { m_targetOrgan = target; }

bool Nanocollector::HasTissueDetected() { return m_tissueDetected; }

void Nanocollector::collectMessage() { this->m_tissueDetected = true; }

} // namespace particles

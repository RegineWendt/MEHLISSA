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

#include "Nanoparticle.h"

namespace particles {

    Nanoparticle::Nanoparticle() {
        particleType = NanoparticleType;
        m_length = 0.0000011; // 11 nm
        m_width = 0.00000055; // 5,5nm
        m_delay = 1;          // 4.71 for ctDNA, 2.32 for LDL
        m_got_detected = 0;
        m_detectionRadius = 0;
    }

    Nanoparticle::~Nanoparticle() {}

    double Nanoparticle::GetDelay() { return m_delay; }

    void Nanoparticle::SetDelay(double value) {
        if (value < 0)
            value = 0;
        m_delay = value;
    }

    double Nanoparticle::GetDetectionRadius() { return m_detectionRadius; }

    void Nanoparticle::SetDetectionRadius(double value) {
        if (value < 0)
            value = 0;
        m_detectionRadius = value;
    }

    int Nanoparticle::GotDetected() { return m_got_detected; }

    void Nanoparticle::GetsDetected() { this->m_got_detected++; }

} // namespace particles

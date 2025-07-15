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

#include "TCell.h"

namespace particles {

TCell::TCell() {
    particleType = TCellType;
    double default_nanobot_size = 0.000004702; // 47.02 nm
    m_length = 0.0011500; // 11.5 mum
    m_width = 0.0011500;  // 11.5 mum
    m_delay = 1; 
    m_got_detected = 0;
    m_detectionRadius = 0.0000001;
}

TCell::~TCell() {}

int TCell::WasDetected() { return m_got_detected > 0; }

void TCell::SetDelay(double value) {}

} // namespace particles

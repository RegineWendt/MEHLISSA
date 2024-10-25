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

#include "TCell.h"

namespace ns3 {

TCell::TCell() {
    m_type = TCellType;
    double default_nanobot_size = 0.000004702; // 47.02 nm
    // Jurkat
    m_length = 0.0011500; // 11.5 mum
    m_width = 0.0011500;  // 11.5 mum
    // HL60
    // m_length = 0.0012400;                        // 12.4 mum
    // m_width = 0.0012400;                         // 12.4 mum
    m_delay = 1; // default_nanobot_size / m_length;// calculated based on size
                                                    //  in relation to Nanobots

    m_got_detected = 0;                 // TODO
    m_detectionRadius = 0.0000001; // TODO
}

TCell::~TCell() {}

int TCell::WasDetected() { return m_got_detected > 0; }

void TCell::SetDelay(double value) {}

} // namespace ns3

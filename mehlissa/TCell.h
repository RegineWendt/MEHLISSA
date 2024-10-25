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

#ifndef CLASS_TCELL_
#define CLASS_TCELL_

#include "Nanoparticle.h"
#include "Randomizer.h"
#include <iostream>
#include <random>

namespace ns3 {
class TCell;
/**
 * \brief CarTCell is a mobile object that detects CancerCells.
 */

class TCell : public Nanoparticle {
private:
    double m_delay;     // factor for changed travelling  speed
    int m_got_detected; // counts up if particle was detected by nanobot
    double m_detectionRadius;

public:
    TCell();
    ~TCell();

    int WasDetected();

    /**
     * Setter method for the delay.
     * inherited from Parent but disabled for this class.
     * Only the delay calculated in the constructor is used.
     */
    void SetDelay(double value);
};
}; // namespace ns3
#endif


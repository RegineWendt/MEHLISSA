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

#ifndef CLASS_CANCERCELL_
#define CLASS_CANCERCELL_

#include "Nanoparticle.h"
#include <iostream>

namespace particles {
class CancerCell;
/**
 * \brief CancerCell is a Nanoparticle that can be detected and destroyed by
 * CarTCell objects.
 */

class CancerCell : public Nanoparticle {
private:
    double m_delay;     // factor for changed travelling  speed
    int m_got_detected; // counts up if particle was detected by nanobot
    double m_detectionRadius;

public:
    CancerCell();
    ~CancerCell();

    /**
     * Return true if the CancerCell was detected by a CarTCell and must be
     * destroyed
     */
    bool MustBeDeleted();

    /**
     * Setter method for the delay.
     * inherited from Parent but disabled for this class.
     * Only the delay calculated in the constructor is used.
     */
    void SetDelay(double value);
};
}; // namespace particles
#endif

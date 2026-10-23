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

#ifndef CLASS_NANOCOLLECTOR_
#define CLASS_NANOCOLLECTOR_

#include "Particle.h"
#include <iostream>

namespace particles {
class Nanocollector;
/**
 * \brief Nanocollector is a mobile Object.
 *
 * Each Nanocollector is a Particle with bigger dimensions, more capabilities and
 * less speed. To see in the csv output file if its a Nanocollector or not,
 * check the last column. False (0) is a Particle and true (1) a Nanocollector.
 */

class Nanocollector : public Particle {
private:
    double m_delay;        // factor for slower traveling speed.
    int m_targetOrgan;     // Marker for Organ whos message molecule can be
                           // collected
    bool m_tissueDetected; // true when message molecule is collected

public:
    /// Constructor to initialize values of all variables.
    Nanocollector();

    /// Destructor [does nothing].
    ~Nanocollector();

    /**
     * Getter method for the delay.
     * If the delay is greater than zero, it indicates that it is a
     * Nanocollector.
     *
     * @return The delay value.
     */
    double GetDelay();

    /**
     * Setter method for the delay.
     * @param value The delay value to set.
     */
    void SetDelay(double value);

    /**
     * Getter method for the target organ.
     * @return The target organ.
     */
    int GetTargetOrgan();

    /**
     * Setter method for the target organ.
     * @param target The target organ to set.
     */
    void SetTargetOrgan(int target);

    /**
     * Getter method for tissue detection.
     * @return True if tissue is detected, false otherwise.
     */
    bool HasTissueDetected();

    /**
     * Setter method for tissue detection.
     * This method is used to collect a message.
     */
    void collectMessage();
};
}; // namespace particles
#endif

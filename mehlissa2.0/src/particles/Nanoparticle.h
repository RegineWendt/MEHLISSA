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

#ifndef CLASS_NANOPARTICLE_
#define CLASS_NANOPARTICLE_

#include "Particle.h"
#include <iostream>
namespace particles {
class Nanoparticle;
/**
 * \brief Nanoparticle is a mobile Object.
 *
 * Each Nanoparticle is a small molecule or objekt with more speed than a
 * nanobot, due to its smaller size. To see in the csv output file if its a
 * Nanoparticle or not, check the last column. False (0) is a Particle and true
 * (1) a Nanoparticle.
 */

class Nanoparticle : public Particle {
private:
    double m_delay;     // factor for faster traveling speed.
    int m_got_detected; // counts up when particle was detected by nanobot
    double m_detectionRadius;

public:
    /// Constructor to initialize values of all variables.
    Nanoparticle();

    /// Destructor [does nothing].
    ~Nanoparticle();

    /**
     * Getter method for the delay.
     * If the delay is greater than zero, it indicates that it is a
     * Nanoparticle.
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
     * Getter method for detection status.
     * @return counts up if it was detected, false otherwise.
     */
    int GotDetected();

    /**
     * Setter method for detection status.
     * This method is used to signal a detection.
     */
    void GetsDetected();

    double GetDetectionRadius();

    void SetDetectionRadius(double value);
};
}; // namespace particles
#endif

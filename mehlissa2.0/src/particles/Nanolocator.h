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

#ifndef CLASS_NANOLOCATOR_
#define CLASS_NANOLOCATOR_

#include "Particle.h"
#include <iostream>

namespace particles {
class NanoLocator;
/**
 * \brief NanoLocator is a mobile Object.
 *
 * Each NanoLocator is a Particle with a specific tileset loaded, that is able to
 * form a message when in the right organ and a disease marker is present. To
 * see in the csv output file if its a NanoLocator or not, check the column
 * before last. (0) is a Particle or locator and (1) is a Nanocollector. And in
 * the last column a number higher than one indicates the organ the locator is
 * able to detect.
 */

class NanoLocator : public Particle {
private:
    bool m_hasFingerprint; // has fingerprint loaded in the beginning, after
                           // release false
    int m_targetOrgan;     // target organ of loaded fingerprint

public:
    /// Constructor to initialize values of all variables.
    // NanoLocator (int target);
    /// Constructor to initialize values of all variables.
    NanoLocator();

    /// Destructor [does nothing].
    ~NanoLocator();


    /**
     * This function returns the target organ.
     *
     * @return The target organ.
     */
    int GetTargetOrgan();

    /**
     * This function sets the target organ.
     *
     * @param[in] target The target organ to be set.
     */
    void SetTargetOrgan(int target);

    /**
     * This function checks if a fingerprint is loaded.
     *
     * @return True if a fingerprint is loaded, false otherwise.
     */
    bool HasFingerprintLoaded();

    /**
     * This function releases fingerprint tiles.
     */
    void releaseFingerprintTiles();
};
}; // namespace particles
#endif

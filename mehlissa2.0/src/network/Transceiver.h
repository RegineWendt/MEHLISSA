/*
 * Copyright (c) 2025 Universität zu Lübeck [WENDT] and Technische Universität
 * Berlin [DEBUS] This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
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

#ifndef CLASS_TRANSCEIVER
#define CLASS_TRANSCEIVER

#include "../bloodcircuit/BloodVessel.h"
#include "../particles/Particle.h"
#include "../utils/GlobalTimer.h"
#include "../utils/IDCounter.h"
#include "../utils/Logger.h"
#include "../utils/Position.h"
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

using namespace bloodcircuit;
using namespace particles;
using namespace utils;
using namespace std;

namespace network {
class Transceiver;

enum TransceiverType { AdrenalGlandType, HypothalamusType, PituitaryType };

class Transceiver {
protected:
    int m_id;                         // transceiver's id
    Position m_position;              // transceiver's position
    shared_ptr<BloodVessel> m_vessel; // transceiver's vessel

    std::vector<ParticleType>
        m_detectedParticles; // transceiver's detected particle types

    double m_p_detection;   // transceiver's probability of detection
    double m_p_absorption;  // transceiver's probability of absorption
    int m_detectionCounter; // transceiver's detected partciles (during the last
                            // interval)

public:
    TransceiverType type;

    Transceiver(TransceiverType t_type, int t_id, Position t_position,
                shared_ptr<BloodVessel> t_vessel,
                std::vector<ParticleType> t_detectedParticles,
                double t_p_detection, double t_p_absorption);

    /// Destructor [does nothing].
    ~Transceiver();

    /**
     * Compare transcievers based on their ID.
     * Returns true if the ID of v1 is smaller than v2's ID.
     */
    static bool Compare(shared_ptr<Transceiver> v1,
                        shared_ptr<Transceiver> v2) {
        return v1->m_id > v2->m_id;
    }

    /**
     * \returns the transceiver Id.
     */
    int GetTransceiverID();

    /**
     * \returns the position of the transceiver.
     */
    Position GetPosition();

    void SetVessel(shared_ptr<BloodVessel> vessel);

    shared_ptr<BloodVessel> GetVessel();

    /**
     * Runs the detection algorithm of the transceiver on the given nanobots.
     * \returns.An updated list of nanobots.
     */
    virtual vector<shared_ptr<Particle>>
    DetectParticles(vector<shared_ptr<Particle>> nanobots);

    virtual string GetGatewayOutput();
};
}; // namespace network
#endif // CLASS_TRANSCEIVER

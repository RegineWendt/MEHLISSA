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

#ifndef CLASS_BLOODSTREAM_
#define CLASS_BLOODSTREAM_

#include "../particles/Particle.h"
#include "../particles/Nanocollector.h"
#include "../particles/Nanolocator.h"
#include "../particles/Nanoparticle.h"
#include "../utils/Position.h"
#include <memory>
#include <cmath>
#include <omp.h>
#include <mutex>

using namespace std;
using namespace particles;
using namespace utils;

namespace bloodcircuit {

class Bloodstream {
private:
    int m_bloodvesselID; // unique ID, set in bloodcircuit.

    /// Stream settings
    int m_currentStream; // id of stream
    double m_velocity;   // velocity of the bloodvessel.
    int m_velocity_factor;
    // bool m_changeStreamSet; // true, if the nanobots should be able to change
    // their streams.
    double m_offset_x;
    double m_offset_y;
    double m_offset_z;
    list<shared_ptr<Particle>> m_nanobots;
    mutex m_addStepMutex;

public:
    Bloodstream(void);

    /// Destructor.
    ~Bloodstream(void);

    // initialize bloodstream
    /**
     * \param vesselId: Id of the Vessel
     * \param streamID: Id of the Stream
     * \param velocityfactor: 1 - 100 Factor which defines velocity compared to
     * the base velocity \param offsetX: relative offsetcoordinates \param
     * offsetY: relative offsetcoordinates \param angle: angle of the stream
     */
    void initBloodstream(int vesselId, int streamId, int velocityfactor,
                         double offsetX, double offsetY, double angle);

    void ClearStream();

    /**
     * \return number of nanobots inside of this stream
     */
    size_t CountParticles(void);

    int CountCarTCells();

    int CountCancerCells();
    /**
     * \param index: ParticleID
     */
    shared_ptr<Particle> GetParticle(int index);

    /**
     * \param index: ParticleID
     */
    shared_ptr<Particle> RemoveParticle(int index);

    /**
     * \param bot: pointer to bot
     */
    shared_ptr<Particle> RemoveParticle(shared_ptr<Particle> bot);

    /**
     * \param bot: pointer to bot
     */
    void AddParticle(shared_ptr<Particle> bot);

    /**
     * Sort the stream
     */
    void SortStream(void);

    /**
     * \return true if empty
     */
    bool IsEmpty(void);

    /**
     * \return velocity
     */
    double GetVelocity(void);

    /**
     * \parameter new velocity
     */
    void SetVelocity(double velocity);

    /**
     * \param offsetX: relative offsetcoordinates
     * \param offsetY: relative offsetcoordinates
     * \param angle: angle of the stream
     */
    void SetAngle(double angle, double offsetX, double offsetY);

}; //  Class End
}; // namespace bloodcircuit
#endif

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

#ifndef CLASS_PARTICLE_
#define CLASS_PARTICLE_

#include "../utils/GlobalTimer.h"
#include "../utils/Position.h"
#include <list>
#include <memory>
#include <cstdint>
#include <iostream>

using namespace utils;
using namespace std;

namespace particles {
class Particle;

/**
 * \brief Particle is a mobile Object.
 *
 * Each Particle has a dimension [length and width] and can be positioned by a
 * Position. The position of the Particle is the position of its node which it
 * owns. A Particle belongs to a particular stream of a specific BloodVessel. Its
 * current velocity depends on the stream it is in. Particles are managed by
 * bloodvessels.
 */
enum ParticleType {
    BaseParticleType,
    NanocollectorType,
    NanolocatorType,
    NanoparticleType,
    CancerCellType,
    CarTCellType,
    TCellType,
    ContainerParticleType,
    SwitchableParticleType
};

class Particle {
protected:
    int m_nanobotID; // nanobot's id
    double m_length; // nanobot's length.
    double m_width;  // nanobot's width.
    int m_stream_nb; // nanobot's stream.

    Position m_position;   // nanobot's position

    bool m_canAge;         // nanobot can age and die
    uint64_t m_maxAge;     // nanobot's maximum age [s]
    uint64_t m_ageCounter; // nanobot's age counter [s]

    bool m_shouldChange;  // nanobot's change stream setting
    uint64_t m_timeStep; // sim time of the last change of the nanobot

    bool m_willPerformMitosis;  // set if cell will perform mitosis in next step
    bool m_mitosisTime;         // nanobot's time to mitosis
    bool m_mitosisCounter;      // counter for mitosis

public:
    ParticleType particleType;

    /// Constructor to initialize values of all variables.
    /// Length an width are set to 100nm and the Particle does not change streams
    /// by default. The ParticleID is set in bloodcircuit, were the Particles are
    /// initialised.
    Particle();

    /// Destructor [does nothing].
    ~Particle();

    /// Compare is used for the purpose of sorting nanobots based on their ID in
    /// a list or a queue. returns true if the ID of v1 is smaller than v2's ID.
    static bool Compare(shared_ptr<Particle> v1, shared_ptr<Particle> v2);

    /// Getter and setter methods

    /**
     * \returns the Particle Id.
     */
    int GetParticleID();

    /**
     * \param value a Particle Id.
     *
     * A Particle has an unique Id.
     */
    void SetParticleID(int value);

    /**
     * \returns the length of the Particle.
     */
    double GetLength();

    /**
     * \param value the length of the Particle.
     */
    void SetLength(double value);

    /**
     * \returns the width of the Particle.
     */
    double GetWidth();

    /**
     * \param value the stream of the Particle.
     */
    void SetStream(int value);

    /**
     * \returns the stream of the Particle.
     */
    int GetStream();

    /**
     * \param value the width of the Particle.
     */
    void SetWidth(double value);

    /**
     * \returns true if the Particle should change
     */
    bool GetShouldChange();

    /**
     * \param bool if nanobot should change.
     */
    void SetShouldChange(bool value);

    /**
     * \returns the time of the last change of Particles position
     */
    uint64_t GetTimeStepInSeconds();

    /**
     * \sets the time of the last change in position.
     */
    void SetTimeStep();

    /**
     * \returns the position of Particle's Node which is located at the center
     * back of the Particle.
     */
    Position GetPosition();

    /**
     * \param value a position Position.
     *
     * This function sets the position of Particle's Node. Particle's position is
     * its Node's position. The position Position must point to the center back of
     * the Particle.
     */
    void SetPosition(Position value);

    /**
     * \returns the delay if set in child (eg. Nanocollector).
     */
    virtual double GetDelay();

    /**
     * sets the delay in child (eg. Nanocollector).
     */
    virtual void SetDelay(double value);

    /**
     * \returns the target organ if set in child (eg. Nanocollector).
     */
    virtual int GetTargetOrgan();

    /**
     * \returns if there is a fingerprint if set in child (eg. NanoLocator).
     */
    virtual bool HasFingerprintLoaded();

    /**
     * \returns if a tissue has been detected if set in child (eg.
     * Nanocollector).
     */
    virtual bool HasTissueDetected();

    /**
     * collects message if set in child (eg. Nanocollector).
     */
    virtual void collectMessage();

    /**
     * releases a fingerprint if set in child (eg. NanoLocator).
     */
    virtual void releaseFingerprintTiles();

    /**
     * \returns how often a particle was detected if set in child (eg.
     * Nanoparticle).
     */
    virtual int GotDetected();

    /**
     * signals a detection if set in child.
     */
    virtual void GetsDetected();

    virtual double GetDetectionRadius();

    virtual void SetDetectionRadius(double value);

    bool CanAge();

    void SetCanAge(bool canAge);

    uint64_t GetAge();

    bool IsAlive();

    bool Age();

    bool Age(int secCount);

    virtual void ResetMitosis();

    virtual bool AddPossibleMitosis(ParticleType type);
    
    virtual bool WillPerformMitosis();
};
}; // namespace particles
#endif

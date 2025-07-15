/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [WENDT]
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
 */

#ifndef CLASS_NANOBOT_
#define CLASS_NANOBOT_

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/object.h"
#include <cstdint>
#include <iostream>

namespace ns3 {
class Nanobot;

/**
 * \brief Nanobot is a mobile Object.
 *
 * Each Nanobot has a dimension [length and width] and can be positioned by a
 * Vector. The position of the Nanobot is the position of its node which it
 * owns. A Nanobot belongs to a particular stream of a specific Bloodvessel. Its
 * current velocity depends on the stream it is in. Nanobots are managed by
 * bloodvessels.
 */
enum ParticleType {
    NanobotType,
    NanocollectorType,
    NanolocatorType,
    NanoparticleType,
    CancerCellType,
    CarTCellType,
    TCellType
};

class Nanobot : public ns3::Object {
protected:
    int m_nanobotID; // nanobot's id
    double m_length; // nanobot's length.
    double m_width;  // nanobot's width.
    int m_stream_nb; // nanobot's stream.

    Ptr<Node> m_node; // nanobot has a node - a node has a position

    bool m_canAge;         // nanobot can age and die
    uint64_t m_maxAge;     // nanobot's maximum age [s]
    uint64_t m_ageCounter; // nanobot's age counter [s]

    bool m_shouldChange;  // nanobot's change stream setting
    ns3::Time m_timeStep; // sim time of the last change of the nanobot

    bool m_willPerformMitosis;  // set if cell will perform mitosis in next step
    bool m_mitosisTime;         // nanobot's time to mitosis
    bool m_mitosisCounter;      // counter for mitosis

public:
    ParticleType m_type;

    // static TypeId GetTypeId (void);

    /// Constructor to initialize values of all variables.
    /// Length an width are set to 100nm and the Nanobot does not change streams
    /// by default. The NanobotID is set in bloodcircuit, were the Nanobots are
    /// initialised.
    Nanobot();

    /// Destructor [does nothing].
    ~Nanobot();

    /// Compare is used for the purpose of sorting nanobots based on their ID in
    /// a list or a queue. returns true if the ID of v1 is smaller than v2's ID.
    static bool Compare(Ptr<Nanobot> v1, Ptr<Nanobot> v2);

    /// Getter and setter methods

    /**
     * \returns the Nanobot Id.
     */
    int GetNanobotID();

    /**
     * \param value a Nanobot Id.
     *
     * A Nanobot has an unique Id.
     */
    void SetNanobotID(int value);

    /**
     * \returns the length of the Nanobot.
     */
    double GetLength();

    /**
     * \param value the length of the Nanobot.
     */
    void SetLength(double value);

    /**
     * \returns the width of the Nanobot.
     */
    double GetWidth();

    /**
     * \param value the stream of the Nanobot.
     */
    void SetStream(int value);

    /**
     * \returns the stream of the Nanobot.
     */
    int GetStream();

    /**
     * \param value the width of the Nanobot.
     */
    void SetWidth(double value);

    /**
     * \returns true if the Nanobot should change
     */
    bool GetShouldChange();

    /**
     * \param bool if nanobot should change.
     */
    void SetShouldChange(bool value);

    /**
     * \returns the time of the last change of Nanobots position
     */
    ns3::Time GetTimeStep();

    /**
     * \sets the time of the last change in position.
     */
    void SetTimeStep();

    /**
     * \returns the position of Nanobot's Node which is located at the center
     * back of the Nanobot.
     */
    Vector GetPosition();

    /**
     * \param value a position Vector.
     *
     * This function sets the position of Nanobot's Node. Nanobot's position is
     * its Node's position. The position Vector must point to the center back of
     * the Nanobot.
     */
    void SetPosition(Vector value);

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
     * \returns if there is a fingerprint if set in child (eg. Nanolocator).
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
     * releases a fingerprint if set in child (eg. Nanolocator).
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
}; // namespace ns3
#endif

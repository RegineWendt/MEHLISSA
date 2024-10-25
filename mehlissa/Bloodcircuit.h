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

#ifndef CLASS_BLOODCIRCUIT_
#define CLASS_BLOODCIRCUIT_

#include "Bloodvessel.h"
#include "CancerCell.h"
#include "CarTCell.h"
#include "Nanobot.h"
#include "Nanocollector.h"
#include "Nanolocator.h"
#include "Nanoparticle.h"
#include "TCell.h"
#include "Randomizer.h"
#include "IDCounter.h"
#include <fstream>
#include <functional>
#include <random>
#include <stdexcept>

using namespace std;

namespace ns3 {
/**
 * \brief Bloodcircuit holds all Bloodvessels of the bodymodel of a female:
 * hight 1.72m, weight 69kg, age 29.
 *
 * Each Bloodvessel is created and the Nanobots are injected into one specific
 * Bloodvessel (default: Aorta_ascendens). The angle and length of each
 * Bloodvessel is calculated with the coordinates.
 */
class Bloodcircuit {
private:
    unsigned int valuesPerLine = 8;

    unsigned int injectionVesselID = 1;

    // A map of m_bloodvesselId to Bloodvessel.
    map<int, ns3::Ptr<ns3::Bloodvessel>> m_bloodvessels;

    Ptr<PrintNanobots> printer;

    vector<int> m_fingerprint_organs;

    // read in bloodcircuit data from the given file
    void ReadInBloodcircuit(string fileName);
    //
    // add a new vessel Object to the bloodcircuit
    void AddVesselData(int id, BloodvesselType type, Vector start, Vector stop);

    // creates vessel objects from array
    void InitialiseBloodvessels(int vesseldata[][8], unsigned int count);

    // creates connections between bloodvessels
    void ConnectBloodvessels();

    // reads probabilities for transitions from csv if provided
    void SetTransitionProbabilities();

    // reads fingerprint formation times for known organs from csv if provided
    void SetFingerprintTimes();

    void SetGatewayVessel(int detectionVesselID);

    /// Calculates the normalized direction vector of a vessel.
    /**
     * \param bloodvessel pointer to the Bloodvessel.
     * \return the normalized direction vector of the Bloodvessel.
     */
    Vector CalcDirectionVectorNorm(Ptr<Bloodvessel> bloodvessel);

    /**
     * \param numberOfNanobots to be simulated.
     * \param bloodvessel where the Nanobots get injected.
     */
    void InjectNanobots(int numberOfNanobots, int numberOfNanocollectors,
                        int numberOfNanolocators, int numberOfCancerCells,
                        int numberOfTCells, Ptr<Bloodvessel> bloodvessel, 
                        int particleMode);

    void AddNanoparticleToVessel(int numberOfNanobots,
                                 int numberOfNanocollectors,
                                 int numberOfNanolocators,
                                 Ptr<Bloodvessel> bloodvessel,
                                 int particleMode);

    void AddNanobot(int streamID,
                    Ptr<Bloodvessel> bloodvessel, Vector location);

    void AddNanocollector(int streamID,
                          Ptr<Bloodvessel> bloodvessel, Vector location,
                          int counter);

    void AddNanolocator(int streamID,
                        Ptr<Bloodvessel> bloodvessel, Vector location,
                        int counter);

    void AddNanoparticle(unsigned int vesselID, double delay, 
                         double detectionRadius, int streamID);

    void AddCancerCell(unsigned int vesselID, int streamID);

    void AddTCell(unsigned int vesselID, int streamID);

    void AddCarTCell(unsigned int vesselID, int streamID);

    void AddCarTCellInjectionToVessel(unsigned int numberOfCarTCells,
                                      unsigned int injectionVessel,
                                      unsigned int injectionTime);

    /**
     * \param type of Bloodvessel
     * \return speed of Bloodvesseltype
     */
    double GetSpeedClassOfBloodVesselType(BloodvesselType type);

    /// Return the Bloodcircuit map.
    map<int, ns3::Ptr<ns3::Bloodvessel>> GetBloodcircuit();

public:
    /// The constructor setting up the Bloodcircuit.
    Bloodcircuit(unsigned int numberOfNanobots, unsigned int numberOfCollectors,
                 unsigned int numberOfLocators, unsigned int injectionVessel,
                 Ptr<PrintNanobots> printer, unsigned int particleMode);

    Bloodcircuit(unsigned int numberOfCancerCells,
                 unsigned int numberOfCarTCells, unsigned int numberOfTCells,
                 unsigned int injectionVessel, unsigned int detectionVessel, 
                 unsigned int injectionTime, Ptr<PrintNanobots> printer);

    /// Destructor to clean up the map.
    ~Bloodcircuit();

    static int BeginSimulation(unsigned int simulationDuration,
                               unsigned int numOfNanobots,
                               unsigned int injectionVessel,
                               unsigned int numOfCollectors,
                               unsigned int numOfLocators,
                               unsigned int particleMode, bool isDeterministic);

    static int
    CancerSimulation(unsigned int numCancerCells, unsigned int numCarTCells,
                     unsigned int numTCells, unsigned int simulationDuration,
                     unsigned int injectionTime, unsigned int injectionVessel,
                     unsigned int detectionVessel, bool isDeterministic,
                     string simFile, string gwFile);

    void PrintStatistics();

    static unsigned int GetNextNanobotID();
};
}; // namespace ns3
#endif

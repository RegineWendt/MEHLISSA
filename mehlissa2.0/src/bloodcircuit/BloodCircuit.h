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

#ifndef CLASS_BLOODCIRCUIT_
#define CLASS_BLOODCIRCUIT_

#include "BloodVessel.h"
#include "../particles/CancerCell.h"
#include "../particles/CarTCell.h"
#include "../particles/Particle.h"
#include "../particles/Nanocollector.h"
#include "../particles/Nanolocator.h"
#include "../particles/Nanoparticle.h"
#include "../particles/TCell.h"
#include "../utils/Randomizer.h"
#include "../utils/RandomStream.h"
#include "../utils/IDCounter.h"
#include "../utils/Position.h"
#include <fstream>
#include <functional>
#include <random>
#include <stdexcept>
#include <memory>
#include <sstream>

using namespace std;
using namespace particles;
using namespace utils;

namespace bloodcircuit {
/**
 * \brief BloodCircuit holds all BloodVessels of the bodymodel of a female:
 * hight 1.72m, weight 69kg, age 29.
 *
 * Each BloodVessel is created and the Particles are injected into one specific
 * BloodVessel (default: Aorta_ascendens). The angle and length of each
 * BloodVessel is calculated with the coordinates.
 */
class BloodCircuit {
private:

    unsigned int valuesPerLine = 9;

    unsigned int injectionVesselID = 1;

    // A map of m_bloodvesselId to BloodVessel.
    map<int, shared_ptr<BloodVessel>> m_bloodvessels;

    shared_ptr<Printer> printer;

    vector<int> m_fingerprint_organs;

    // read in bloodcircuit data from the given file
    void ReadInBloodCircuit(string fileName);
    //
    // add a new vessel Object to the bloodcircuit
    void AddVesselData(int id, BloodVesselType type, Position start, Position stop);

    // creates vessel objects from array
    void InitialiseBloodVessels(int vesseldata[][8], unsigned int count);

    // creates connections between bloodvessels
    void ConnectBloodVessels();

    // reads probabilities for transitions from csv if provided
    void SetTransitionProbabilities();

    // reads fingerprint formation times for known organs from csv if provided
    void SetFingerprintTimes();

    void SetGatewayVessel(int detectionVesselID);

    /// Calculates the normalized direction vector of a vessel.
    /**
     * \param bloodvessel pointer to the BloodVessel.
     * \return the normalized direction vector of the BloodVessel.
     */
    Position CalcDirectionVectorNorm(shared_ptr<BloodVessel> bloodvessel);

    /**
     * \param numberOfParticles to be simulated.
     * \param bloodvessel where the Particles get injected.
     */
    void InjectParticles(int numberOfParticles, int numberOfNanocollectors,
                        int numberOfNanolocators, int numberOfCancerCells,
                        int numberOfTCells, shared_ptr<BloodVessel> bloodvessel, 
                        int particleMode);

    void AddNanoparticleToVessel(int numberOfParticles,
                                 int numberOfNanocollectors,
                                 int numberOfNanolocators,
                                 shared_ptr<BloodVessel> bloodvessel,
                                 int particleMode);

    void AddParticle(int streamID,
                    shared_ptr<BloodVessel> bloodvessel, Position location);

    void AddNanocollector(int streamID,
                          shared_ptr<BloodVessel> bloodvessel, Position location,
                          int counter);

    void AddNanolocator(int streamID,
                        shared_ptr<BloodVessel> bloodvessel, Position location,
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
     * \param type of BloodVessel
     * \return speed of BloodVesseltype
     */
    double GetSpeedClassOfBloodVesselType(BloodVesselType type);


public:

    static string vasculatureFile;
    static string transitionsFile;
    static string fingerprintFile;
    
    /// The constructor setting up the BloodCircuit.
    BloodCircuit(unsigned int numberOfParticles, unsigned int numberOfCollectors,
                 unsigned int numberOfLocators, unsigned int injectionVessel,
                 shared_ptr<Printer> printer, unsigned int particleMode);

    BloodCircuit(unsigned int numberOfCancerCells,
                 unsigned int numberOfCarTCells, unsigned int numberOfTCells,
                 unsigned int injectionVessel, unsigned int detectionVessel, 
                 unsigned int injectionTime, shared_ptr<Printer> printer);
    
    BloodCircuit(shared_ptr<Printer> printer);

    /// Destructor to clean up the map.
    ~BloodCircuit();

    static shared_ptr<BloodCircuit> 
    BeginSimulation(unsigned int simulationDuration,
                    unsigned int numOfParticles, unsigned int injectionVessel,
                    unsigned int numOfCollectors, unsigned int numOfLocators,
                    unsigned int particleMode, bool isDeterministic);

    static shared_ptr<BloodCircuit>
    CancerSimulation(unsigned int numCancerCells, unsigned int numCarTCells,
                     unsigned int numTCells, unsigned int simulationDuration,
                     unsigned int injectionTime, unsigned int injectionVessel,
                     unsigned int detectionVessel, bool isDeterministic,
                     string simFile, string gwFile);
    
    void PrintStatistics();

    static unsigned int GetNextParticleID();
    
    /// Return the BloodCircuit map.
    map<int, shared_ptr<BloodVessel>> GetBloodCircuit();

    static void SetVasculature(string vasculature, string transitions,
                               string fingerprints)
    {
        vasculatureFile = vasculature;
        transitionsFile = transitions;
        fingerprintFile = fingerprints;
    }
};
}; // namespace bloodcircuit
#endif

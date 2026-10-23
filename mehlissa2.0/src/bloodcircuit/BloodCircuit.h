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

#ifndef CLASS_BLOODCIRCUIT_
#define CLASS_BLOODCIRCUIT_

#include "../network/AdrenalGland.h"
#include "../network/Hypothalamus.h"
#include "../network/Pituitary.h"
#include "../network/Transceiver.h"
#include "../particles/CancerCell.h"
#include "../particles/CarTCell.h"
#include "../particles/Nanocollector.h"
#include "../particles/Nanolocator.h"
#include "../particles/Nanoparticle.h"
#include "../particles/Particle.h"
#include "../particles/TCell.h"
#include "../utils/IDCounter.h"
#include "../utils/Position.h"
#include "../utils/RandomStream.h"
#include "../utils/Randomizer.h"
#include "BloodVessel.h"
#include <fstream>
#include <functional>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
// cout << "count" << endl;

using namespace std;
using namespace particles;
using namespace utils;
using namespace network;

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
    unsigned int valuesPerLine = 17;

    unsigned int injectionVesselID = 1;

    // A map of m_bloodvesselId to BloodVessel.
    map<int, shared_ptr<BloodVessel>> m_bloodvessels;
    map<int, shared_ptr<Transceiver>> m_transceivers;

    shared_ptr<Printer> printer;

    vector<int> m_fingerprint_organs;

    int m_idHypothalamus;
    int m_idPituitary;
    int m_idAdrenalGland;

    // read in bloodcircuit data from the given file
    void ReadInBloodCircuit(string fileName);
    //
    // add a new vessel Object to the bloodcircuit
    void AddVesselData(int id, BloodVesselType type, Position start,
                       Position stop);

    int AddHypothalamus(int id, int baseline, double detect_prob,
                        double detect_dist, double f1, double f2);

    int AddPituitary(int id, int baseline_type, double detect_prob,
                     double detect_dist, double f1, double f2, double f3,
                     string baseline_input);

    int AddAdrenalGland(int id, int delay, double detect_prob,
                        double detect_dist, int baseline, double f1);

    void AddGateway(int id, int baseline, Position pos, double distance,
                    double factor_g1, double factor_g2, double p_g1_detect,
                    double p_s_detect_1, double p_s_detect_2,
                    double initial_input_time, int type);

    void AddSensor(int id, int delayType, Position pos, string sensorDelayInput,
                   double distance, double factor, double offset, double phase,
                   double amp, double alpha_step, double alpha_offset,
                   double gamma_step, double gamma_offset, int type);

    void BasicTransceiverSettings();

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
                         int numberOfTCells,
                         shared_ptr<BloodVessel> bloodvessel, int particleMode);

    void AddNanoparticleToVessel(int numberOfParticles,
                                 int numberOfNanocollectors,
                                 int numberOfNanolocators,
                                 shared_ptr<BloodVessel> bloodvessel,
                                 int particleMode);

    void AddParticle(int streamID, shared_ptr<BloodVessel> bloodvessel,
                     Position location);

    void AddNanocollector(int streamID, shared_ptr<BloodVessel> bloodvessel,
                          Position location, int counter);

    void AddNanolocator(int streamID, shared_ptr<BloodVessel> bloodvessel,
                        Position location, int counter);

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
    BloodCircuit(shared_ptr<Printer> printer, unsigned int numberOfParticles,
                 unsigned int numberOfCollectors, unsigned int numberOfLocators,
                 unsigned int numberOfCancerCells,
                 unsigned int numberOfCarTCells, unsigned int numberOfTCells,
                 unsigned int injectionVessel, unsigned int detectionVessel,
                 unsigned int injectionTime, unsigned int particleMode);
    BloodCircuit(unsigned int numberOfParticles,
                 unsigned int numberOfCollectors, unsigned int numberOfLocators,
                 unsigned int injectionVessel, shared_ptr<Printer> printer,
                 unsigned int particleMode);

    BloodCircuit(unsigned int numberOfCancerCells,
                 unsigned int numberOfCarTCells, unsigned int numberOfTCells,
                 unsigned int injectionVessel, unsigned int detectionVessel,
                 unsigned int injectionTime, shared_ptr<Printer> printer);

    BloodCircuit(shared_ptr<Printer> printer);

    /// Destructor to clean up the map.
    ~BloodCircuit();

    static shared_ptr<BloodCircuit> CreateBloodCircuit(
        bool isDeterministic, unsigned int printMode,
        unsigned int printInterval, string simFile, string gwFile,
        unsigned int injectionTime, unsigned int injectionVessel,
        unsigned int detectionVessel, unsigned int simulationDuration,
        unsigned int numOfParticles, unsigned int numOfCollectors,
        unsigned int numOfLocators, unsigned int numCancerCells,
        unsigned int numCarTCells, unsigned int numTCells);

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

    void PrintParticles();

    void ClosePrinter();

    void ConnectTransceivers();

    static unsigned int GetNextParticleID();

    /// Return the BloodCircuit map.
    map<int, shared_ptr<BloodVessel>> GetBloodCircuit();

    static void SetVasculature(string vasculature, string transitions,
                               string fingerprints) {
        vasculatureFile = vasculature;
        transitionsFile = transitions;
        fingerprintFile = fingerprints;
    }

    void RunTransceivers(int step);
};
}; // namespace bloodcircuit
#endif

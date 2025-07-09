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

#ifndef CLASS_BLOODVESSEL_
#define CLASS_BLOODVESSEL_

#include "Bloodstream.h"
#include "../particles/CancerCell.h"
#include "../particles/CarTCell.h"
#include "../particles/Particle.h"
#include "../particles/Nanocollector.h"
#include "../particles/Nanolocator.h"
#include "../particles/Nanoparticle.h"
#include "../particles/TCell.h"
#include "../utils/Printer.h"
#include "../utils/Randomizer.h"
#include "../utils/RandomStream.h"
#include "../utils/IDCounter.h"
#include "../utils/Position.h"
#include "../utils/GlobalTimer.h"
#include <random>
#include <memory>
#include <map>
#include <math.h>

using namespace std;
using namespace utils;
using namespace particles;

namespace bloodcircuit {
/**
 * \brief BloodVessel is the place holder of the Particle's and manages each step
 * of the Particle's mobility.
 *
 * A BloodVessel has up to total 5 lists (maximum 5 streams). At each step
 * (interval dt), BloodVessel browses the nanobots of each stream in order of
 * their positions and moves each Particle. If the resulting position exceeds the
 * current bloodvessel the nanobot gets pushed to the next bloodvessel (and so
 * on). Particles are added to the BloodVessels in BloodCircuit.
 */
enum BloodVesselType { ARTERY, VEIN, ORGAN };
class BloodVessel: public enable_shared_from_this<BloodVessel>{
private:
    // bool m_start;
    vector<shared_ptr<Bloodstream>> m_bloodstreams; // list of nanobots in streams
    int m_bloodvesselID;                     // unique ID, set in bloodcircuit
    double m_bloodvesselLength;              // the length of the bloodvessel
    double m_angle;                          // the angle of the bloodvessel
    double m_basevelocity;                   // velocity of the bloodvessel
    BloodVesselType m_bloodvesselType;       // type of the bloodvessel:
                                             // 0=artery, 1=vein, 2=organ
    Position m_startPositionBloodVessel;     // start-coordinates of the vessel
    Position m_stopPositionBloodVessel;      // end-coordinates of the vessel
    double m_vesselWidth;                    // the width of each stream in the
                                             // bloodvessel
    bool m_isGatewayVessel;                  // vessel records measurements
    CarTCell::CarTCellInjection injection;   // notes whether additional
                                             // CarTCells are injected into the
                                             // vessel during the simulation

    // Simulation time
    double m_deltaT;      // the mobility step interval
                          // (duration between each step)
    double m_stepsPerSec; // number of steps per second
    int m_secStepCounter; // counts steps in each second

    // Fingerprint functionality
    double m_fingerprintFormationTime;  // out of csv, time that a message
                                        // molecule needs to be formed after
                                        // release from the nanobot
    double m_fingerPrintTimer;
    bool m_hasActiveFingerprintMessage; // turns true after nanolocator was in
                                        // vessel and timer of formation ended
                                        // succesfully

    // Connections
    shared_ptr<BloodVessel> m_nextBloodVessel1;
    shared_ptr<BloodVessel> m_nextBloodVessel2;
    std::map<int, list<shared_ptr<Particle>>> reachedEndMap;

    // Transition probabilities for connections
    double m_transitionto1; // probability blood flows to first vessel
    double m_transitionto2; // probability blood flows to second vessel
                            // (if exists)

    // Stream settings
    int m_numberOfStreams;  // number of streams, maximum value is 5
    bool m_changeStreamSet; // true, if nanobots are able to change
                            // between streams
    // stream split according to power-law
    const int stream_definition[21][3] = {
        {100, 0, 0},  {99, -1, 0},  {99, +1, 0},  {99, 0, -1},  {99, 0, +1},
        {99, -1, -1}, {99, +1, +1}, {99, +1, -1}, {99, -1, +1}, {86, +2, 0},
        {86, -2, 0},  {86, 0, +2},  {86, 0, -2},  {86, +2, -1}, {86, -2, +1},
        {86, -1, +2}, {86, +1, -2}, {86, +2, +1}, {86, -2, -1}, {86, +1, +2},
        {86, -1, -2}};
    // stream split according to poiseuille
    // {100, 0, 0},  {96, -1, 0},  {96, +1, 0},  {96, 0, -1},  {96, 0, +1},
    // {96, -1, -1}, {96, +1, +1}, {96, +1, -1}, {96, -1, +1}, {60, +2, 0},
    // {64, -2, 0},  {64, 0, +2},  {64, 0, -2},  {64, +2, -1}, {64, -2, +1},
    // {64, -1, +2}, {64, +1, -2}, {64, +2, +1}, {64, -2, -1}, {64, +1, +2},
    // {64, -1, -2}};
    // original
    // {100, 0, 0},  {95, -1, 0},  {95, +1, 0},  {95, 0, -1},  {95, 0, +1},
    // {95, -1, -1}, {95, +1, +1}, {95, +1, -1}, {95, -1, +1}, {90, +2, 0},
    // {90, -2, 0},  {90, 0, +2},  {90, 0, -2},  {90, +2, -1}, {90, -2, +1},
    // {90, -1, +2}, {90, +1, -2}, {90, +2, +1}, {90, -2, -1}, {90, +1, +2},
    // {90, -1, -2}};
    const int stream_definition_size = 21;

    // Output printer and file with positions and timesteps.
    // ofstream m_nbTrace;
    // string m_nbTraceFilename;
    shared_ptr<Printer> printer;

    Position SetPosition(Position nbv, double distance, double angle,
                       int bloodvesselType, double startPosZ);

    // calculate Angle
    double CalcAngle();

    // calculate Length
    double CalcLength();

    double CalcDistance(shared_ptr<Particle> n_1, shared_ptr<Particle> n_2);

    /**
     * \param value the Number of Streams the bloodvessel can have.
     */
    void initStreams();

    /// Translates the Particles to the new position. Calles TranslatePosition
    /// (), ChangeStream (), TransposeParticles ().
    void TranslateParticles();

    /// Calculates the position and velocity of each nanobot for the passed step
    /// and the next step.
    void TranslatePosition(double dt);

    /// Changes the nanobot streams if possible. Calles DoChangeStreamIfPossible
    /// ().
    void ChangeStream();

    /// Changes the nanobot streams from current streams to the destination
    /// stream.
    void DoChangeStreamIfPossible(int curStream, int desStream);

    /// Transposes Particles from one bloodvessel to another.
    //void TransposeParticles(list<shared_ptr<Particle>> reachedEnd, int i);


    void TransferStep(list<shared_ptr<Particle>> reachedEnd, int stream);

    /// Moves one Particle to the next bloodvessel
    bool transposeParticle(shared_ptr<Particle> botToTranspose,
                          shared_ptr<BloodVessel> thisBloodVessel,
                          shared_ptr<BloodVessel> nextBloodVessel, int stream);
    /**
     * /return TRUE if position exceeds bloodvessel
     */
    bool moveParticle(shared_ptr<Particle> nb, int i, int randVelocityOffset,
                     bool direction, double dt);


    /**
     * \returns true, if all streams of the bloodvessel are empty.
     */
    bool IsEmpty();

    /**
     * \returns the Type of the BloodVessel.
     */
    BloodVesselType GetBloodVesselType();

    /**
     * \returns the Angle of the BloodVessel.
     */
    double GetBloodVesselAngle();

    /**
     * \returns the Length of the BloodVessel.
     */
    double GetbloodvesselLength();

    void PerformInjection();
    
    void CheckFingerprintRelease();

    void CheckParticleInteractions();

public:
    /**
     * Setting the default values:
     * dt=1.0, number of streams=3, changing stream set to true, velocity and
     * current stream is zero. x-, y- and z-direction are empty.
     */
    BloodVessel();

    /**
     *  Destructor to clean up the lists.
     */
    ~BloodVessel();

    shared_ptr<BloodVessel> Step(uint64_t timeInMS);

    void PerformTransferStep();

    bool NeedsTransferStep();

    list<shared_ptr<Particle>> GetParticles();
    /* 
     * Prints all nanobots in the BloodVessel to a csv file.
     */
    void PrintParticlesOfVessel();

    /**
     * \param value the traffic velocity m/s at entrance.
     */
    void InitBloodstreamLengthAngleAndVelocity(double velocity);

    /**
     * \param streamID: ID of Stream
     * \param bot: Pointer to bot to add
     */
    void AddParticleToStream(unsigned int streamID, shared_ptr<Particle> bot);

    void CheckRelease(list<shared_ptr<Particle>> nbToCheck);

    void CountStepsAndAgeCells();

    void PerformCellMitosis();
    
    int CountCancerCells();

    int CountCarTCells();

    /**
     * \param Id of a Stream
     * \returns a specific stream
     */
    shared_ptr<Bloodstream> GetStream(int id);

    /**
     * \returns the ID of the BloodVessel.
     */
    int GetbloodvesselID();

    /**
     * \returns the Number of Streams in the BloodVessel.
     */
    int GetNumberOfStreams();

    /**
     * \returns the Startposition of the BloodVessel.
     */
    Position GetStartPositionBloodVessel();

    /**
     * \returns the stopposition of the BloodVessel.
     */
    Position GetStopPositionBloodVessel();

    /**
     * \param value the ID of the BloodVessel.
     */
    void SetBloodVesselID(int b_id);

    /**
     * \param value the type of the BloodVessel.
     */
    void SetBloodVesselType(BloodVesselType value);

    /**
     * \param value the Width of the Streams.
     */
    void SetVesselWidth(double value);

    /**
     * \param value the start position of the BloodVessel.
     */
    void SetStartPositionBloodVessel(Position value);

    /**
     * \param value the start position of the BloodVessel.
     */
    void SetStopPositionBloodVessel(Position value);

    /**
     * \param value the following BloodVessel.
     */
    void SetNextBloodVessel1(shared_ptr<BloodVessel> value);

    /**
     * \param value the following BloodVessel.
     */
    void SetNextBloodVessel2(shared_ptr<BloodVessel> value);

    /**
     * \param value transition probability to choose the following BloodVessel.
     */
    void SetTransition1(double value);

    /**
     * \param value transition probability to choose the following BloodVessel.
     */
    void SetTransition2(double value);

    /**
     * Fingerprint functionality
     * \param value time a fingerprint needs to form a message after release.
     */
    void SetFingerprintFormationTime(double value);
    void SetFingerprintRelease(double time);

    /**
     * \returns the time it takes a fingerprint to be formed in the BloodVessel.
     */
    double GetFingerprintFormationTime();

    /**
     * Setter for the printer.
     */
    void SetPrinter(shared_ptr<Printer> printer);

    bool IsGatewayVessel();

    void SetIsGatewayVessel(bool value);

    /**
     * \returns true if fingerprint message molecules are active in the
     * BloodVessel.
     */
    bool isActive();

    /**
     * Checks if the Particle is of type nanocollector and in it's target organ.
     * If the target organ has message molecules active, the collector collects
     * them and turns it's tissue detected attribute to true. \param value list
     * of Particles that need to get checked.
     */
    void CheckCollect(list<shared_ptr<Particle>> nbToCheck);

    /**
     * Checks if the Particle is of type Particle and if there are Particles in
     * its range to detect it. If the Particle is detected, its count goes up.
     * \param value list of Particles that need to get checked.
     */
    void CheckDetect(list<shared_ptr<Particle>> nbToCheck);

    void ReleaseParticles();

    void AddCarTCellInjection(int injectionTime, int injectionVessel,
                              int numberOfCarTCells);

    void ExchangeParticles(std::vector<shared_ptr<Particle>> newBots);
};
}; // namespace bloodcircuit
#endif

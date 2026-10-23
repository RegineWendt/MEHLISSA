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

#include "BloodCircuit.h"

/**
 * This function sets the BloodVessel map of the human body of a female:
 * hight 1.72m, weight 69kg, age 29.
 */

namespace bloodcircuit {

string BloodCircuit::vasculatureFile;
string BloodCircuit::transitionsFile;
string BloodCircuit::fingerprintFile;

BloodCircuit::BloodCircuit(shared_ptr<Printer> printer) {
    m_bloodvessels = map<int, shared_ptr<BloodVessel>>();
    m_transceivers = map<int, shared_ptr<Transceiver>>();
    this->printer = printer;
    BasicTransceiverSettings();
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();
}

BloodCircuit::BloodCircuit(unsigned int numberOfCancerCells,
                           unsigned int numberOfCarTCells,
                           unsigned int numberOfTCells,
                           unsigned int injectionVessel,
                           unsigned int detectionVessel,
                           unsigned int injectionTime,
                           shared_ptr<Printer> printer) {
    m_bloodvessels = map<int, shared_ptr<BloodVessel>>();
    m_transceivers = map<int, shared_ptr<Transceiver>>();
    this->printer = printer;
    BasicTransceiverSettings();
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();
    SetGatewayVessel(detectionVessel);

    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    if (m_bloodvessels.size() > 1)
        InjectParticles(0, 0, 0, numberOfCancerCells, numberOfTCells,
                        m_bloodvessels[injectionVessel], 0);
    AddCarTCellInjectionToVessel(numberOfCarTCells, injectionVessel,
                                 injectionTime);
}

BloodCircuit::BloodCircuit(unsigned int numberOfParticles,
                           unsigned int numberOfCollectors,
                           unsigned int numberOfLocators,
                           unsigned int injectionVessel,
                           shared_ptr<Printer> printer,
                           unsigned int particleMode) {
    // initialise map with bloodvesselinformation
    m_bloodvessels = map<int, shared_ptr<BloodVessel>>();
    m_transceivers = map<int, shared_ptr<Transceiver>>();
    this->printer = printer;
    BasicTransceiverSettings();
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();

    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    if (m_bloodvessels.size() > 1) {
        InjectParticles(numberOfParticles, numberOfCollectors, numberOfLocators,
                        0, 0, m_bloodvessels[injectionVesselID], particleMode);
    }
}

BloodCircuit::BloodCircuit(
    shared_ptr<Printer> printer, unsigned int numberOfParticles,
    unsigned int numberOfCollectors, unsigned int numberOfLocators,
    unsigned int numberOfCancerCells, unsigned int numberOfCarTCells,
    unsigned int numberOfTCells, unsigned int injectionVessel,
    unsigned int detectionVessel, unsigned int injectionTime,
    unsigned int particleMode) {
    // initialise map with bloodvesselinformation
    m_bloodvessels = map<int, shared_ptr<BloodVessel>>();
    m_transceivers = map<int, shared_ptr<Transceiver>>();
    this->printer = printer;
    BasicTransceiverSettings();
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();
    if (detectionVessel >= 0)
        SetGatewayVessel(detectionVessel);

    if (injectionVessel >= 0)
        injectionVesselID = injectionVessel < m_bloodvessels.size()
                                ? injectionVessel
                                : m_bloodvessels.size() - 1;
    bool must_inject = numberOfCancerCells + numberOfCarTCells +
                           numberOfCollectors + numberOfLocators +
                           numberOfParticles + numberOfParticles +
                           numberOfTCells >
                       0;
    if (m_bloodvessels.size() > 1 && must_inject)
        InjectParticles(numberOfParticles, numberOfCollectors, numberOfLocators,
                        numberOfCancerCells, numberOfTCells,
                        m_bloodvessels[injectionVesselID], particleMode);
    if (numberOfCarTCells > 0)
        AddCarTCellInjectionToVessel(numberOfCarTCells, injectionVessel,
                                     injectionTime);
}

void BloodCircuit::BasicTransceiverSettings() {
    m_idHypothalamus = -1;
    m_idPituitary = -1;
    m_idAdrenalGland = -1;
}

shared_ptr<BloodCircuit> BloodCircuit::CreateBloodCircuit(
    bool isDeterministic, unsigned int printMode, unsigned int printInterval,
    string simFile, string gwFile, unsigned int injectionTime,
    unsigned int injectionVessel, unsigned int detectionVessel,
    unsigned int simulationDuration, unsigned int numOfParticles,
    unsigned int numOfCollectors, unsigned int numOfLocators,
    unsigned int numCancerCells, unsigned int numCarTCells,
    unsigned int numTCells) {
    try {
        Randomizer::InitRandomizer(isDeterministic);
        IDCounter::InitIDCounter();
        shared_ptr<Printer> printNano =
            make_shared<Printer>(simFile, gwFile, printMode, printInterval);
        shared_ptr<BloodCircuit> circuit = make_shared<BloodCircuit>(
            printNano, numOfParticles, numOfCollectors, numOfLocators,
            numCancerCells, numCarTCells, numTCells, injectionVessel,
            detectionVessel, injectionTime, printMode);
        map<int, shared_ptr<BloodVessel>> circuitMap =
            circuit->GetBloodCircuit();
        if (circuitMap.size() <= 1) {
            Logger::LogWarning(
                string("Not enough vessels for simulation! Please check ") +
                circuit->vasculatureFile + "");
            return nullptr;
        }
        return circuit;
    } catch (const exception &e) {
        Logger::LogError(string("Exception ") + e.what() + "");
        return nullptr;
    }
}

shared_ptr<BloodCircuit> BloodCircuit::BeginSimulation(
    unsigned int simulationDuration, unsigned int numOfParticles,
    unsigned int injectionVessel, unsigned int numOfCollectors,
    unsigned int numOfLocators, unsigned int particleMode,
    bool isDeterministic) {
    Randomizer::InitRandomizer(isDeterministic);
    shared_ptr<Printer> printNano = make_shared<Printer>(particleMode);
    // Create the bloodcircuit
    shared_ptr<BloodCircuit> circuit = make_shared<BloodCircuit>(
        printNano, numOfParticles, numOfCollectors, numOfLocators, 0, 0, 0,
        injectionVessel, -1, -1, particleMode);
    // Get the map of the bloodcircuit
    map<int, shared_ptr<BloodVessel>> circuitMap = circuit->GetBloodCircuit();
    if (circuitMap.size() <= 1) {
        Logger::LogWarning(
            string("Not enough vessels for simulation! Please check ") +
            circuit->vasculatureFile + "");
        return nullptr;
    }
    return circuit;
}

shared_ptr<BloodCircuit> BloodCircuit::CancerSimulation(
    unsigned int numCancerCells, unsigned int numCarTCells,
    unsigned int numTCells, unsigned int simulationDuration,
    unsigned int injectionTime, unsigned int injectionVessel,
    unsigned int detectionVessel, bool isDeterministic, string simFile,
    string gwFile) {
    try {
        Randomizer::InitRandomizer(isDeterministic);
        IDCounter::InitIDCounter();
        shared_ptr<Printer> printNano =
            make_shared<Printer>(simFile, gwFile, 0);

        // setup bloodcircuit
        shared_ptr<BloodCircuit> circuit = make_shared<BloodCircuit>(
            printNano, 0, 0, 0, numCancerCells, numCarTCells, numTCells,
            injectionVessel, detectionVessel, injectionTime, 0);

        map<int, shared_ptr<BloodVessel>> circuitMap =
            circuit->GetBloodCircuit();

        if (circuitMap.size() <= 1) {
            Logger::LogWarning(
                string("Not enough vessels for simulation! Please check ") +
                circuit->vasculatureFile + "");
            return nullptr;
        }
        return circuit;
    } catch (const exception &e) {
        Logger::LogError(string("Exception ") + e.what() + "");
        return nullptr;
    }
}

BloodCircuit::~BloodCircuit() {
    m_bloodvessels.clear();
    printer->~Printer();
}

map<int, shared_ptr<BloodVessel>> BloodCircuit::GetBloodCircuit() {
    return m_bloodvessels;
}

Position
BloodCircuit::CalcDirectionVectorNorm(shared_ptr<BloodVessel> m_bloodvessel) {
    Position start = m_bloodvessel->GetStartPositionBloodVessel();
    Position end = m_bloodvessel->GetStopPositionBloodVessel();
    double x = end.x - start.x;
    double y = end.y - start.y;
    double z = end.z - start.z;
    double vectorLength = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    x = x / vectorLength;
    y = y / vectorLength;
    z = z / vectorLength;
    return Position(x, y, z);
}

void BloodCircuit::InjectParticles(int numberOfParticles,
                                   int numberOfNanocollectors,
                                   int numberOfNanolocators,
                                   int numberOfCancerCells, int numberOfTCells,
                                   shared_ptr<BloodVessel> bloodvessel,
                                   int particleMode) {
    if (numberOfNanocollectors > numberOfParticles)
        throw runtime_error("Number of Nanocollectors too high");
    if (numberOfNanolocators > numberOfParticles)
        throw runtime_error("Number of Nanolocators too high");

    Logger::LogInfo("Starting first injections");
    int nanobotGroupSize = 1;
    shared_ptr<RandomStream> distribute_randomly =
        Randomizer::GetNewRandomStream(0, bloodvessel->GetNumberOfStreams());
    Position m_coordinateStart = bloodvessel->GetStartPositionBloodVessel();
    int intervall = (numberOfParticles >= nanobotGroupSize)
                        ? div(numberOfParticles, nanobotGroupSize).quot
                        : numberOfParticles;

    Position m_direction = CalcDirectionVectorNorm(bloodvessel);
    Position directionIntervall = Position(m_direction.x / nanobotGroupSize,
                                           m_direction.y / nanobotGroupSize,
                                           m_direction.z / nanobotGroupSize);
    unsigned int group = 0;

    switch (particleMode) {
    case 1: // Particle mode LDL Cholesterol
        Logger::LogInfo("---> Particles");
        // release particles from the liver Organ 36
        for (int i = 1; i <= 100; ++i) {
            AddNanoparticle(36, 2.32, 0.2,
                            floor(distribute_randomly->GetValue()));
        }
        break;
    case 2: // Particle mode liquid biopsy breast
        Logger::LogInfo("---> Particles");
        // release particles from the breast Organ 24
        for (int i = 1; i <= 160000; ++i) {
            AddNanoparticle(24, 4.71, 0.01,
                            floor(distribute_randomly->GetValue()));
        }
        break;
    default:
        break;
    }

    // Distribute Nanocollectors at the beginning of the start vessel.
    if (numberOfNanocollectors > 0) {
        Logger::LogInfo("---> Collectors");
        for (int i = 1; i <= numberOfNanocollectors; ++i)
            AddNanocollector(floor(distribute_randomly->GetValue()),
                             bloodvessel,
                             Position(m_coordinateStart.x, m_coordinateStart.y,
                                      m_coordinateStart.z),
                             i);
    }
    // Distribute Nanolocators at the beginning of the start vessel.
    if (numberOfNanolocators > 0) {
        Logger::LogInfo("---> Locators");
        for (int i = 1; i <= numberOfNanolocators; ++i)
            AddNanolocator(floor(distribute_randomly->GetValue()), bloodvessel,
                           Position(m_coordinateStart.x, m_coordinateStart.y,
                                    m_coordinateStart.z),
                           i);
    }
    // Distribute cancer cells uniformly
    if (numberOfCancerCells > 0) {
        Logger::LogInfo("---> Cancer cells");
        int bloodcircuitSize = m_bloodvessels.size();
        for (int i = 1; i <= numberOfCancerCells; ++i) {
            unsigned int vesselID = (i % bloodcircuitSize) + 1;
            AddCancerCell(vesselID, floor(distribute_randomly->GetValue()));
        }
    }
    // Distribute healthy T cells uniformly
    if (numberOfTCells > 0) {
        Logger::LogInfo("---> T cells");
        int bloodcircuitSize = m_bloodvessels.size();
        for (int i = 1; i <= numberOfTCells; ++i) {
            unsigned int vesselID = (i % bloodcircuitSize) + 1;
            AddTCell(vesselID, floor(distribute_randomly->GetValue()));
        }
    }
    // Distribute Particles in groups over the beginning of the start
    // vessel.
    if (numberOfParticles > 0) {
        Logger::LogInfo("---> Bots");
        for (int i = 1; i <= numberOfParticles; ++i) {
            group = (i - 1) / intervall;
            AddParticle(
                floor(distribute_randomly->GetValue()), bloodvessel,
                Position(m_coordinateStart.x + (directionIntervall.x * group),
                         m_coordinateStart.y + (directionIntervall.y * group),
                         m_coordinateStart.z + (directionIntervall.z * group)));
        }
    }
    // Print Particles in csv-file.
    bloodvessel->PrintParticlesOfVessel();
}

void BloodCircuit::AddParticle(int streamID,
                               shared_ptr<BloodVessel> bloodvessel,
                               Position location) {
    shared_ptr<Particle> tempNB = make_shared<Particle>();
    tempNB->SetParticleID(GetNextParticleID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    bloodvessel->AddParticleToStream(streamID, tempNB);
}

void BloodCircuit::AddNanocollector(int streamID,
                                    shared_ptr<BloodVessel> bloodvessel,
                                    Position location, int counter) {
    shared_ptr<Nanocollector> tempNB = make_shared<Nanocollector>();
    tempNB->SetParticleID(GetNextParticleID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    tempNB->SetTargetOrgan(m_fingerprint_organs[counter % 9]);
    bloodvessel->AddParticleToStream(streamID, tempNB);
}

void BloodCircuit::AddNanolocator(int streamID,
                                  shared_ptr<BloodVessel> bloodvessel,
                                  Position location, int counter) {
    shared_ptr<NanoLocator> tempNB = make_shared<NanoLocator>();
    tempNB->SetParticleID(GetNextParticleID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    tempNB->SetTargetOrgan(m_fingerprint_organs[counter % 9]);
    bloodvessel->AddParticleToStream(streamID, tempNB);
}

void BloodCircuit::AddNanoparticle(unsigned int vesselID, double delay,
                                   double detectionRadius, int streamID) {
    shared_ptr<BloodVessel> vessel = m_bloodvessels[vesselID];
    Position coordinateVessel = vessel->GetStartPositionBloodVessel();
    shared_ptr<Nanoparticle> tempNP = make_shared<Nanoparticle>();
    tempNP->SetParticleID(GetNextParticleID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Position(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    tempNP->SetDelay(delay);
    tempNP->SetDetectionRadius(detectionRadius);
    vessel->AddParticleToStream(streamID, tempNP);
}

void BloodCircuit::AddCancerCell(unsigned int vesselID, int streamID) {
    shared_ptr<BloodVessel> vessel = m_bloodvessels[vesselID];
    Position coordinateVessel = vessel->GetStartPositionBloodVessel();
    shared_ptr<CancerCell> tempNP = make_shared<CancerCell>();
    tempNP->SetParticleID(GetNextParticleID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Position(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddParticleToStream(streamID, tempNP);
}

void BloodCircuit::AddTCell(unsigned int vesselID, int streamID) {
    shared_ptr<BloodVessel> vessel = m_bloodvessels[vesselID];
    Position coordinateVessel = vessel->GetStartPositionBloodVessel();
    shared_ptr<TCell> tempNP = make_shared<TCell>();
    tempNP->SetParticleID(GetNextParticleID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Position(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddParticleToStream(streamID, tempNP);
}

void BloodCircuit::AddCarTCell(unsigned int vesselID, int streamID) {
    shared_ptr<BloodVessel> vessel = m_bloodvessels[vesselID];
    Position coordinateVessel = vessel->GetStartPositionBloodVessel();
    shared_ptr<CarTCell> tempNP = make_shared<CarTCell>();
    tempNP->SetParticleID(GetNextParticleID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Position(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddParticleToStream(streamID, tempNP);
}

void BloodCircuit::AddCarTCellInjectionToVessel(unsigned int numberOfCarTCells,
                                                unsigned int injectionVessel,
                                                unsigned int injectionTime) {
    if (injectionVessel <= 0)
        throw runtime_error("Cannot inject CAR-T cells into vessel with "
                            "negative ID. Given ID: " +
                            injectionVessel);
    if (numberOfCarTCells == 0)
        return;
    if (numberOfCarTCells < 0)
        throw runtime_error("Cannot inject negative number of CAR-T "
                            "cells. Input number was: " +
                            numberOfCarTCells);
    if (injectionTime < 0) {
        throw runtime_error("Cannot inject CAR-T cells at negative time."
                            "Given time: " +
                            injectionTime);
    } else if (injectionTime == 0) {
        shared_ptr<RandomStream> distribute_randomly =
            Randomizer::GetNewRandomStream(
                0, m_bloodvessels[injectionVessel]->GetNumberOfStreams());
        for (unsigned int i = 1; i <= numberOfCarTCells; ++i) {
            AddCancerCell(injectionVessel,
                          floor(distribute_randomly->GetValue()));
        }
    } else {
        m_bloodvessels[injectionVessel]->AddCarTCellInjection(
            injectionTime, injectionVessel, numberOfCarTCells);
    }
}

double BloodCircuit::GetSpeedClassOfBloodVesselType(BloodVesselType type) {
    if (type == ARTERY)
        return 10.0;
    else if (type == VEIN)
        return 3.7;
    else // if (type == ORGAN)
        return 1.0;
}

void BloodCircuit::ReadInBloodCircuit(string fileName) {
    Logger::LogInfo(string("Loading flow network from: ") + fileName);
    std::ifstream infile{fileName};
    if (infile.good()) {
        try {
            std::vector<string> numbers;
            numbers.resize(valuesPerLine);
            std::string buffer;
            std::string line;
            int errorflag = 0;
            // while (infile.good()) {
            while (std::getline(infile, line)) {
                std::stringstream lineStream(line);
                for (auto &&elem : numbers) {
                    if (std::getline(lineStream, buffer, ',')) {
                        elem = buffer;
                        // TODOD: check with Regine whether errorflag should be
                        // reset ehr... she's the only one using this...
                        // errorflag = 0;
                    } else {
                        elem = "0";
                        errorflag = 1;
                    }
                }
                if (errorflag == 0) {
                    if (stoi(numbers[1]) < 3) {
                        AddVesselData(
                            stoi(numbers[0]), (BloodVesselType)stoi(numbers[1]),
                            Position(stod(numbers[2]), stod(numbers[3]),
                                     stod(numbers[4])),
                            Position(stod(numbers[5]), stod(numbers[6]),
                                     stod(numbers[7])));
                    } else if (stoi(numbers[1]) == 100) {
                        m_idHypothalamus =
                            AddHypothalamus(stoi(numbers[0]), stoi(numbers[2]),
                                            stod(numbers[3]), stod(numbers[4]),
                                            stod(numbers[5]), stod(numbers[6]));
                    } else if (stoi(numbers[1]) == 101) {
                        m_idPituitary =
                            AddPituitary(stoi(numbers[0]), stoi(numbers[2]),
                                         stod(numbers[3]), stod(numbers[4]),
                                         stod(numbers[5]), stod(numbers[6]),
                                         stod(numbers[7]), numbers[8]);
                    } else if (stoi(numbers[1]) == 102) {
                        m_idAdrenalGland =
                            AddAdrenalGland(stoi(numbers[0]), stoi(numbers[2]),
                                            stod(numbers[3]), stod(numbers[4]),
                                            stoi(numbers[5]), stod(numbers[6]));
                    } else {
                        throw runtime_error(
                            "Encountered entry with unknown type ID: " + line);
                    }
                }
                ConnectTransceivers();
            }
            Logger::LogInfo(" ... done");
        } catch (const exception &e) {
            Logger::LogError(
                string("Exception during bloodcircuit laoding procedure: ") +
                e.what() + "");
        }
    } else { // Old way of loading data
        throw runtime_error("NO VALID CSV FILE FOUND! Please provide a valid "
                            "'vasculatures.csv'");
    }
}

void BloodCircuit::AddVesselData(int id, BloodVesselType type, Position start,
                                 Position stop) {
    shared_ptr<BloodVessel> vessel = make_shared<BloodVessel>();
    vessel->SetBloodVesselID(id);
    vessel->SetBloodVesselType(type);
    vessel->SetStartPositionBloodVessel(start);
    vessel->SetStopPositionBloodVessel(stop);
    vessel->SetVesselWidth(0.25);
    vessel->SetPrinter(printer);
    // Init BloodVessel: Calculate length and angle & velocity.
    vessel->InitBloodstreamLengthAngleAndVelocity(
        GetSpeedClassOfBloodVesselType(type));
    m_bloodvessels[id] = vessel;
    Logger::LogDebug(string("New Vessel (") + to_string(id) + "," +
                     to_string(type) + "," + to_string(start.x) + "," +
                     to_string(start.y) + "," + to_string(start.z) + "," +
                     to_string(stop.x) + "," + to_string(stop.y) + "," +
                     to_string(stop.z) + ")");
}

int BloodCircuit::AddHypothalamus(int id, int baseline, double detect_prob,
                                  double detect_dist, double f1, double f2) {
    shared_ptr<Hypothalamus> hypo = make_shared<Hypothalamus>(
        id, baseline, detect_prob, detect_dist, f1, f2);
    m_transceivers[id] = hypo;
    Logger::LogDebug("New Hypothalamus");
    return id;
}

int BloodCircuit::AddPituitary(int id, int baseline_type, double detect_prob,
                               double detect_dist, double f1, double f2,
                               double f3, string baseline_input) {
    shared_ptr<Pituitary> pitu =
        make_shared<Pituitary>(id, baseline_type, detect_prob, detect_dist, f1,
                               f2, f3, baseline_input);
    m_transceivers[id] = pitu;
    Logger::LogDebug("New Pituitary");
    return id;
}

int BloodCircuit::AddAdrenalGland(int id, int delay, double detect_prob,
                                  double detect_dist, int baseline, double f1) {
    shared_ptr<AdrenalGland> adre = make_shared<AdrenalGland>(
        id, delay, 1, detect_prob, detect_dist, baseline, f1);
    m_transceivers[id] = adre;
    Logger::LogDebug("New AdrenalGland");
    return id;
}

void BloodCircuit::InitialiseBloodVessels(int vesseldata[][8],
                                          unsigned int count) {
    for (unsigned int i = 0; i < count; i++)
        AddVesselData(
            vesseldata[i][0], (BloodVesselType)vesseldata[i][1],
            Position(vesseldata[i][2], vesseldata[i][3], vesseldata[i][4]),
            Position(vesseldata[i][5], vesseldata[i][6], vesseldata[i][7]));
}

void BloodCircuit::ClosePrinter() { printer->CloseFile(); }

void BloodCircuit::ConnectBloodVessels() {
    unsigned int count = m_bloodvessels.size();
    unsigned int transceiverCount = m_transceivers.size();
    // Set Connections between bloodvessels if they have the same start/end
    // coordinates
    Logger::LogInfo("connecting");
    for (unsigned int i = 1; i < count + 1; i++) {
        unsigned int counter = 0;
        Position origin = m_bloodvessels[i]->GetStartPositionBloodVessel();
        Position end = m_bloodvessels[i]->GetStopPositionBloodVessel();
        // Make sure that inititally all BloodVessels have no Fingerprint
        // Formation Times
        m_bloodvessels[i]->SetFingerprintFormationTime(0);
        for (unsigned int j = 1; j < count + 1; j++) {
            Position start = m_bloodvessels[j]->GetStartPositionBloodVessel();
            if (end.x == start.x && end.y == start.y && end.z == start.z) {
                counter++;
                if (counter == 1)
                    m_bloodvessels[i]->SetNextBloodVessel1(m_bloodvessels[j]);
                else
                    m_bloodvessels[i]->SetNextBloodVessel2(m_bloodvessels[j]);
                Logger::LogDebug(
                    string("connected ") +
                    to_string(m_bloodvessels[i]->GetbloodvesselID()) + " to " +
                    to_string(m_bloodvessels[j]->GetbloodvesselID()) + "");
            }
        }
        for (unsigned int k = 0; k < transceiverCount; ++k) {
            if (m_transceivers[k]->GetVessel() == nullptr) {
                Position pV =
                    m_transceivers[k]->GetPosition(); // IDEA: add transceiver
                                                      // to bloodvessel?
                if ((origin.x <= pV.x && pV.x <= end.x ||
                     end.x <= pV.x && pV.x <= origin.x) &&
                    (origin.y <= pV.y && pV.y <= end.y ||
                     end.y <= pV.y && pV.y <= origin.y) &&
                    (origin.z <= pV.z && pV.z <= end.z ||
                     end.z <= pV.z && pV.z <= origin.z)) {
                    m_transceivers[k]->SetVessel(m_bloodvessels[i]);
                }
            }
        }
    }
    Logger::LogInfo(" ... done");
}

void BloodCircuit::ConnectTransceivers() {
    if (m_idHypothalamus >= 0 && m_idPituitary >= 0 && m_idAdrenalGland >= 0) {
        shared_ptr<Hypothalamus> hypo = std::dynamic_pointer_cast<Hypothalamus>(
            m_transceivers[m_idHypothalamus]);
        shared_ptr<Pituitary> pitu =
            std::dynamic_pointer_cast<Pituitary>(m_transceivers[m_idPituitary]);
        shared_ptr<AdrenalGland> adre = std::dynamic_pointer_cast<AdrenalGland>(
            m_transceivers[m_idAdrenalGland]);

        adre->SetACTHOffset(pitu->GetBaseline());
        pitu->SetCORTOffset(adre->GetBaseline());
        hypo->SetCORTOffset(adre->GetBaseline());
    }
}

void BloodCircuit::SetTransitionProbabilities() {
    std::ifstream infile{transitionsFile};
    Logger::LogInfo("loading transitions");
    if (infile.good()) {
        std::vector<double> numbers;
        numbers.resize(3);
        std::string buffer;
        buffer.reserve(64);
        int errorflag = 0;
        while (infile.good()) {
            for (auto &&elem : numbers) {
                if (std::getline(infile, buffer, ',')) {
                    elem = std::stod(buffer);
                } else {
                    elem = 0;
                    errorflag = 1;
                }
            }
            if (errorflag == 0) {
                m_bloodvessels[numbers[0]]->SetTransition1(numbers[1]);
                m_bloodvessels[numbers[0]]->SetTransition2(numbers[2]);
            }
        }
        Logger::LogInfo(" ... done");
    } else { // Old way of loading data
        Logger::LogError(" ... aborted\n"
                         "NO VALID CSV FILE FOUND! \n"
                         "Transitions set to 1 and 0");
    }
}
void BloodCircuit::SetFingerprintTimes() {
    std::ifstream infile{fingerprintFile};
    Logger::LogInfo("loading fingerprint times");
    if (infile.good()) {
        std::vector<double> numbers;
        numbers.resize(2);
        std::string buffer;
        buffer.reserve(64);
        int errorflag = 0;
        while (infile.good()) {
            for (auto &&elem : numbers) {
                if (std::getline(infile, buffer, ',')) {
                    elem = std::stod(buffer);
                } else {
                    elem = 0;
                    errorflag = 1;
                }
            }
            if (errorflag == 0) {
                m_bloodvessels[numbers[0]]->SetFingerprintFormationTime(
                    numbers[1]);
                // Make a vector list, with the organs that have a
                // fingerprint
                m_fingerprint_organs.push_back(numbers[0]);
            }
        }
        Logger::LogInfo(" ... done");
    } else { // Old way of loading data
        Logger::LogError(" ... aborted\n"
                         "NO VALID CSV FILE FOUND!\n"
                         "Fingerprint times not added");
    }
}

void BloodCircuit::SetGatewayVessel(int detectionVesselID) {
    m_bloodvessels[detectionVesselID]->SetIsGatewayVessel(true);
}

void BloodCircuit::PrintStatistics() {
    int cancerCells = 0;
    int carTCells = 0;
    for (int i = 0; i < m_bloodvessels.size(); i++) {
        cancerCells += m_bloodvessels[i]->CountCancerCells();
        carTCells += m_bloodvessels[i]->CountCarTCells();
    }
    Logger::LogInfo(string("Cancer Cells: ") + to_string(cancerCells) +
                    ", CAR-T Cells: " + to_string(carTCells) + "");
}

void BloodCircuit::PrintParticles() {
    for (int i = 0; i < m_bloodvessels.size(); i++) {
        if (m_bloodvessels[i] == 0)
            continue;
        auto particles = m_bloodvessels[i]->GetParticles();
        auto id_vessel = m_bloodvessels[i]->GetbloodvesselID();
        printer->PrintParticles(particles, id_vessel);
    }
}

unsigned int BloodCircuit::GetNextParticleID() {
    return IDCounter::GetNextParticleID();
}

void BloodCircuit::RunTransceivers(int step) {
    ConnectTransceivers();
    unsigned int transceiverCount = m_transceivers.size();
    for (unsigned int k = 0; k < transceiverCount; ++k) {
        shared_ptr<Transceiver> t = m_transceivers[k];
        shared_ptr<BloodVessel> vessel = t->GetVessel();
        Position p = t->GetPosition();
        list<shared_ptr<Particle>> pList =
            vessel->GetParticles(); // TODO: only get nanobots in range
        std::vector<shared_ptr<Particle>> pVector{std::begin(pList),
                                                  std::end(pList)};
        pVector = t->DetectParticles(pVector);
        if (pVector.size() > 0)
            vessel->ExchangeParticles(pVector);
        string out = t->GetGatewayOutput();
        if (!out.empty())
            printer->PrintGateway(t->GetTransceiverID(), out);
    }
}
} // namespace bloodcircuit

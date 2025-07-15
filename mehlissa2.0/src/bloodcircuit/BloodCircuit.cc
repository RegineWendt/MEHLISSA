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
    this->printer = printer;
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
    this->printer = printer;
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();
    cout << "detection Vessel: " << detectionVessel << endl;
    SetGatewayVessel(detectionVessel);

    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    cout << "injection Vessel: " << injectionVesselID << endl;
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
    this->printer = printer;
    ReadInBloodCircuit(vasculatureFile);
    ConnectBloodVessels();
    SetTransitionProbabilities();
    cout << "load CSV - transitions" << endl;
    SetFingerprintTimes();

    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    if (m_bloodvessels.size() > 1) {
        InjectParticles(numberOfParticles, numberOfCollectors, numberOfLocators,
                       0, 0, m_bloodvessels[injectionVesselID], particleMode);
    }
}

shared_ptr<BloodCircuit> BloodCircuit::BeginSimulation(unsigned int simulationDuration,
                                  unsigned int numOfParticles,
                                  unsigned int injectionVessel,
                                  unsigned int numOfCollectors,
                                  unsigned int numOfLocators,
                                  unsigned int particleMode,
                                  bool isDeterministic) {
    Randomizer::InitRandomizer(isDeterministic);
    shared_ptr<Printer> printNano = make_shared<Printer>(particleMode);
    // Create the bloodcircuit
    shared_ptr<BloodCircuit> circuit = make_shared<BloodCircuit>(
                         numOfParticles, numOfCollectors, numOfCollectors,
                         injectionVessel, printNano, particleMode);
    // Get the map of the bloodcircuit
    map<int, shared_ptr<BloodVessel>> circuitMap = circuit->GetBloodCircuit();
    if (circuitMap.size() <= 1) {
        cout << "Not enough vessels for simulation! Please check "
             << circuit->vasculatureFile
             << endl;
        return nullptr;
    }
    return circuit;
}

shared_ptr<BloodCircuit> BloodCircuit::CancerSimulation(unsigned int numCancerCells, 
        unsigned int numCarTCells, unsigned int numTCells,
        unsigned int simulationDuration, unsigned int injectionTime,
        unsigned int injectionVessel, unsigned int detectionVessel,
        bool isDeterministic, string simFile, string gwFile) {
    try {
        Randomizer::InitRandomizer(isDeterministic);
        IDCounter::InitIDCounter();
        shared_ptr<Printer> printNano = make_shared<Printer>
                                              (0, simFile, gwFile);

        // setup bloodcircuit
        shared_ptr<BloodCircuit> circuit = make_shared<BloodCircuit>(
                                  numCancerCells, numCarTCells, numTCells, 
                                  injectionVessel, detectionVessel, 
                                  injectionTime, printNano);

        map<int, shared_ptr<BloodVessel>> circuitMap =
            circuit->GetBloodCircuit();

        if (circuitMap.size() <= 1) {
            cout << "Not enough vessels for simulation! Please check "
                 << circuit->vasculatureFile
                 << endl;
            return nullptr;
        }
        return circuit;
    } catch (const exception &e) {
        cout << "Exception " << e.what() << endl;
        return nullptr;
    }
}

BloodCircuit::~BloodCircuit() { m_bloodvessels.clear(); 
printer->~Printer();}

map<int, shared_ptr<BloodVessel>> BloodCircuit::GetBloodCircuit() {
    return m_bloodvessels;
}

Position BloodCircuit::CalcDirectionVectorNorm(shared_ptr<BloodVessel> m_bloodvessel) {
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

void BloodCircuit::InjectParticles(
    int numberOfParticles, int numberOfNanocollectors, int numberOfNanolocators,
    int numberOfCancerCells, int numberOfTCells, shared_ptr<BloodVessel> bloodvessel, 
    int particleMode) {
    if (numberOfNanocollectors > numberOfParticles)
        throw runtime_error("Number of Nanocollectors too high");
    if (numberOfNanolocators > numberOfParticles)
        throw runtime_error("Number of Nanolocators too high");

    cout << "Starting first injections" << endl;
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
        cout << "---> Particles" << endl;
        // release particles from the liver Organ 36
        for (int i = 1; i <= 100; ++i) {
            AddNanoparticle(36, 2.32, 0.2,
                            floor(distribute_randomly->GetValue()));
        }
        break;
    case 2: // Particle mode liquid biopsy breast
        cout << "---> Particles" << endl;
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
        cout << "---> Collectors" << endl;
        for (int i = 1; i <= numberOfNanocollectors; ++i)
            AddNanocollector(floor(distribute_randomly->GetValue()),
                             bloodvessel,
                             Position(m_coordinateStart.x, m_coordinateStart.y,
                                    m_coordinateStart.z),
                             i);
    }
    // Distribute Nanolocators at the beginning of the start vessel.
    if (numberOfNanolocators > 0) {
        cout << "---> Locators" << endl;
        for (int i = 1; i <= numberOfNanolocators; ++i)
            AddNanolocator(floor(distribute_randomly->GetValue()), bloodvessel,
                           Position(m_coordinateStart.x, m_coordinateStart.y,
                                  m_coordinateStart.z),
                           i);
    }
    // Distribute cancer cells uniformly
    if (numberOfCancerCells > 0) {
        cout << "---> Cancer cells" << endl;
        int bloodcircuitSize = m_bloodvessels.size();
        for (int i = 1; i <= numberOfCancerCells; ++i) {
            unsigned int vesselID = (i % bloodcircuitSize) + 1;
            AddCancerCell(vesselID, floor(distribute_randomly->GetValue()));
        }
    }
    // Distribute healthy T cells uniformly
    if (numberOfTCells > 0) {
        cout << "---> T cells" << endl;
        int bloodcircuitSize = m_bloodvessels.size();
        for (int i = 1; i <= numberOfTCells; ++i) {
            unsigned int vesselID = (i % bloodcircuitSize) + 1;
            AddTCell(vesselID, floor(distribute_randomly->GetValue()));
        }
    }
    // Distribute Particles in groups over the beginning of the start
    // vessel.
    if (numberOfParticles > 0)
        cout << "---> Bots" << endl;
    for (int i = 1; i <= numberOfParticles; ++i) {
        group = (i - 1) / intervall;
        AddParticle(
            floor(distribute_randomly->GetValue()), bloodvessel,
            Position(m_coordinateStart.x + (directionIntervall.x * group),
                   m_coordinateStart.y + (directionIntervall.y * group),
                   m_coordinateStart.z + (directionIntervall.z * group)));
    }
    // Print Particles in csv-file.
    bloodvessel->PrintParticlesOfVessel();
}

void BloodCircuit::AddParticle(int streamID,
                              shared_ptr<BloodVessel> bloodvessel, Position location) {
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
                                  shared_ptr<BloodVessel> bloodvessel, Position location,
                                  int counter) {
    shared_ptr<NanoLocator> tempNB = make_shared<NanoLocator>();
    tempNB->SetParticleID(GetNextParticleID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    tempNB->SetTargetOrgan(m_fingerprint_organs[counter % 9]);
    bloodvessel->AddParticleToStream(streamID, tempNB);
}

void BloodCircuit::AddNanoparticle(unsigned int vesselID,
                                   double delay,
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
    cout << numberOfCarTCells << endl;
    cout << injectionVessel << endl;
    cout << injectionTime << endl;
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
            AddCancerCell(injectionVessel, floor(distribute_randomly->GetValue()));
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
    cout << "Loading flow network from: "
         << fileName << endl;
    std::ifstream infile{fileName};
    if (infile.good()) {
        try {
            std::vector<int> numbers;
            numbers.resize(valuesPerLine);
            std::string buffer;
            std::string line;
            int errorflag = 0;
            //while (infile.good()) {
            while(std::getline(infile,line)) {
                // cout << line << endl;
                std::stringstream lineStream(line);
                for (auto &&elem : numbers) {
                    if (std::getline(lineStream, buffer, ',')) {
                        elem = std::stoi(buffer);
                    } else {
                        elem = 0;
                        errorflag = 1;
                    }
                }
                if (errorflag == 0) {
                    if (numbers[1]< 3) {
                    AddVesselData(numbers[0], (BloodVesselType)numbers[1],
                                  Position(numbers[2],
                                         numbers[3],
                                         numbers[4]),
                                  Position(numbers[5],
                                         numbers[6],
                                         numbers[7]));
                    }
                }
            }
            cout << "bloodcircuit loaded from " << fileName << endl;
        } catch (const exception &e) {
            cout << "Exception during bloodcircuit laoding procedure: "
                 << e.what() << endl;
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
    cout << "New Vessel(" + to_string(id) + "," + to_string(type) + "," +
                to_string(start.x) + "," + to_string(start.y) + "," +
                to_string(start.z) + "," + to_string(stop.x) + "," +
                to_string(stop.y) + "," + to_string(stop.z) + ")"
         << endl;
}

void BloodCircuit::InitialiseBloodVessels(int vesseldata[][8],
                                          unsigned int count) {
    for (unsigned int i = 0; i < count; i++)
        AddVesselData(
            vesseldata[i][0], (BloodVesselType)vesseldata[i][1],
            Position(vesseldata[i][2], vesseldata[i][3], vesseldata[i][4]),
            Position(vesseldata[i][5], vesseldata[i][6], vesseldata[i][7]));
}

void BloodCircuit::ConnectBloodVessels() {
    unsigned int count = m_bloodvessels.size();
    cout << count << endl;
    // Set Connections between bloodvessels if they have the same start/end
    // coordinates
    cout << "connecting";
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
            }
        }
    }
    cout << " ... done" << endl;
}

void BloodCircuit::SetTransitionProbabilities() {
    std::ifstream infile{transitionsFile};
    cout << "loading transitions";
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
        cout << " ... done" << endl;
    } else { // Old way of loading data
        cout << " ... aborted" << endl;
        cout << "NO VALID CSV FILE FOUND! " << endl;
        cout << "Transitions set to 1 and 0" << endl;
    }
}
void BloodCircuit::SetFingerprintTimes() {
    std::ifstream infile{fingerprintFile};
    cout << "loading fingerprint times";
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
        cout << " ... done" << endl;
    } else { // Old way of loading data
        cout << " ... aborted" << endl;
        cout << "NO VALID CSV FILE FOUND! " << endl;
        cout << "Fingerprint times not added" << endl;
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
    cout << "Cancer Cells: " << cancerCells << ", CAR-T Cells: " << carTCells
         << endl;
}

unsigned int BloodCircuit::GetNextParticleID() {
    return IDCounter::GetNextParticleID(); 
}
} // namespace bloodcircuit

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

#include "Bloodcircuit.h"

/**
 * This function sets the Bloodvessel map of the human body of a female:
 * hight 1.72m, weight 69kg, age 29.
 */

namespace ns3 {

Bloodcircuit::Bloodcircuit(unsigned int numberOfCancerCells,
                           unsigned int numberOfCarTCells,
                           unsigned int numberOfTCells,
                           unsigned int injectionVessel,
                           unsigned int detectionVessel,
                           unsigned int injectionTime,
                           Ptr<PrintNanobots> printer) {
    m_bloodvessels = map<int, Ptr<Bloodvessel>>();
    this->printer = printer;
    ReadInBloodcircuit("vasculature_transitions95.csv");
    ConnectBloodvessels();
    SetTransitionProbabilities();
    SetFingerprintTimes();
    cout << "detection Vessel: " << detectionVessel << endl;
    SetGatewayVessel(detectionVessel);

    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    cout << "injection Vessel: " << injectionVesselID << endl;
    if (m_bloodvessels.size() > 1)
        InjectNanobots(0, 0, 0, numberOfCancerCells, numberOfTCells,
                       m_bloodvessels[injectionVessel], 0);
    AddCarTCellInjectionToVessel(numberOfCarTCells, injectionVessel,
                                 injectionTime);
}

Bloodcircuit::Bloodcircuit(unsigned int numberOfNanobots,
                           unsigned int numberOfCollectors,
                           unsigned int numberOfLocators,
                           unsigned int injectionVessel,
                           Ptr<PrintNanobots> printer,
                           unsigned int particleMode) {
    // initialise map with bloodvesselinformation
    m_bloodvessels = map<int, Ptr<Bloodvessel>>();
    this->printer = printer;
    ReadInBloodcircuit("vasculature_transitions95.csv");
    ConnectBloodvessels();
    SetTransitionProbabilities();
    cout << "load CSV - transitions" << endl;
    SetFingerprintTimes();

    // Inject Nanobots here!
    // Places x Bots, randomly on Streams at specific vessel injected.
    // If you choose other values, the nanobots all get injected at the same
    // coordinates
    injectionVesselID = injectionVessel < m_bloodvessels.size()
                            ? injectionVessel
                            : m_bloodvessels.size() - 1;
    if (m_bloodvessels.size() > 1) {
        InjectNanobots(numberOfNanobots, numberOfCollectors, numberOfLocators,
                       0, 0, m_bloodvessels[injectionVesselID], particleMode);
    }
}

// Starts the simulation in a bloodvessel
void Starter(Ptr<Bloodvessel> vessel) { vessel->Start(); }

int Bloodcircuit::BeginSimulation(unsigned int simulationDuration,
                                  unsigned int numOfNanobots,
                                  unsigned int injectionVessel,
                                  unsigned int numOfCollectors,
                                  unsigned int numOfLocators,
                                  unsigned int particleMode,
                                  bool isDeterministic) {
    // execution time
    clock_t start, finish;
    start = clock();
    Randomizer::InitRandomizer(isDeterministic);
    Ptr<PrintNanobots> printNano = new PrintNanobots(particleMode);
    // Create the bloodcircuit
    Bloodcircuit circuit(numOfNanobots, numOfCollectors, numOfCollectors,
                         injectionVessel, printNano, particleMode);
    // Get the map of the bloodcircuit
    map<int, ns3::Ptr<ns3::Bloodvessel>> circuitMap = circuit.GetBloodcircuit();
    if (circuitMap.size() > 1) {
        // Schedule and run the Simulation in each bloodvessel
        for (unsigned int i = 1; i < circuitMap.size() + 1; i++)
            Simulator::Schedule(Seconds(0.0), &Starter, circuitMap[i]);
        // Stop simulation after specific time
        Simulator::Stop(Seconds(simulationDuration + 1));
        Simulator::Run();
        Simulator::Destroy();
        // output execution time
        finish = clock();
        cout << "Dauer: " << simulationDuration << "s " << numOfNanobots
             << "NB " << numOfCollectors << "NC " << numOfLocators << "NL -> "
             << (finish - start) / 1000000 << "s ------------------------"
             << endl;
        cout << "Injected Vessel: " << injectionVessel << endl;
        // Flushing last entries in csv
        printNano->~PrintNanobots();
        return 0;
    } else {
        cout << "Not enough vessels for simulation! Please check "
                "'vasculature.csv'"
             << endl;
        return 1;
    }
}

int Bloodcircuit::CancerSimulation(unsigned int numCancerCells, 
        unsigned int numCarTCells, unsigned int numTCells,
        unsigned int simulationDuration, unsigned int injectionTime,
        unsigned int injectionVessel, unsigned int detectionVessel,
        bool isDeterministic, string simFile, string gwFile) {
    try {
        // general setup
        clock_t start, finish;
        start = clock();
        Randomizer::InitRandomizer(isDeterministic);
        Ptr<PrintNanobots> printNano = new PrintNanobots(0, simFile, gwFile);

        // setup bloodcircuit
        Bloodcircuit circuit(numCancerCells, numCarTCells, numTCells, 
                             injectionVessel, detectionVessel, injectionTime, 
                             printNano);
        map<int, ns3::Ptr<ns3::Bloodvessel>> circuitMap =
            circuit.GetBloodcircuit();

        // run simulation
        if (circuitMap.size() > 1) {
            for (unsigned int i = 1; i < circuitMap.size() + 1; i++)
                Simulator::Schedule(Seconds(0.0), &Starter, circuitMap[i]);
            Simulator::Stop(Seconds(simulationDuration + 1));
            Simulator::Run();
            // circuit.PrintStatistics();
            Simulator::Destroy();
            finish = clock();
            cout << "Time total: " << simulationDuration << "s " << endl;
            cout << numCancerCells << " cancer cells, "
                 << numCarTCells << " CAR-T cells, "
                 << numTCells  << " T cells -> " << (finish - start) / 1000000
                 << "s ------------------------" << endl;
            cout << "Injection Vessel: " << injectionVessel << endl;
            cout << "Injection Time: " << injectionTime << endl;
            printNano->~PrintNanobots();
            return 0;
        } else {
            cout << "Not enough vessels for simulation! Please check "
                    "'vasculature.csv'"
                 << endl;
            return 1;
        }
    } catch (const exception &e) {
        cout << "Exception " << e.what() << endl;
        return 2;
    }
}

Bloodcircuit::~Bloodcircuit() { m_bloodvessels.clear(); }

map<int, ns3::Ptr<ns3::Bloodvessel>> Bloodcircuit::GetBloodcircuit() {
    return m_bloodvessels;
}

Vector Bloodcircuit::CalcDirectionVectorNorm(Ptr<Bloodvessel> m_bloodvessel) {
    Vector start = m_bloodvessel->GetStartPositionBloodvessel();
    Vector end = m_bloodvessel->GetStopPositionBloodvessel();
    double x = end.x - start.x;
    double y = end.y - start.y;
    double z = end.z - start.z;
    double vectorLength = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    x = x / vectorLength;
    y = y / vectorLength;
    z = z / vectorLength;
    return Vector(x, y, z);
}

void Bloodcircuit::InjectNanobots(
    int numberOfNanobots, int numberOfNanocollectors, int numberOfNanolocators,
    int numberOfCancerCells, int numberOfTCells, Ptr<Bloodvessel> bloodvessel, 
    int particleMode) {
    if (numberOfNanocollectors > numberOfNanobots)
        throw runtime_error("Number of Nanocollectors too high");
    if (numberOfNanolocators > numberOfNanobots)
        throw runtime_error("Number of Nanolocators too high");

    cout << "Starting first injections" << endl;
    int nanobotGroupSize = 1;
    Ptr<UniformRandomVariable> distribute_randomly =
        Randomizer::GetNewRandomStream(0, bloodvessel->GetNumberOfStreams());
    Vector m_coordinateStart = bloodvessel->GetStartPositionBloodvessel();
    int intervall = (numberOfNanobots >= nanobotGroupSize)
                        ? div(numberOfNanobots, nanobotGroupSize).quot
                        : numberOfNanobots;

    Vector m_direction = CalcDirectionVectorNorm(bloodvessel);
    Vector directionIntervall = Vector(m_direction.x / nanobotGroupSize,
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
                             Vector(m_coordinateStart.x, m_coordinateStart.y,
                                    m_coordinateStart.z),
                             i);
    }
    // Distribute Nanolocators at the beginning of the start vessel.
    if (numberOfNanolocators > 0) {
        cout << "---> Locators" << endl;
        for (int i = 1; i <= numberOfNanolocators; ++i)
            AddNanolocator(floor(distribute_randomly->GetValue()), bloodvessel,
                           Vector(m_coordinateStart.x, m_coordinateStart.y,
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
    // Distribute Nanobots in groups over the beginning of the start
    // vessel.
    if (numberOfNanobots > 0)
        cout << "---> Bots" << endl;
    for (int i = 1; i <= numberOfNanobots; ++i) {
        group = (i - 1) / intervall;
        AddNanobot(
            floor(distribute_randomly->GetValue()), bloodvessel,
            Vector(m_coordinateStart.x + (directionIntervall.x * group),
                   m_coordinateStart.y + (directionIntervall.y * group),
                   m_coordinateStart.z + (directionIntervall.z * group)));
    }
    // Print Nanobots in csv-file.
    bloodvessel->PrintNanobotsOfVessel();
}

void Bloodcircuit::AddNanobot(int streamID,
                              Ptr<Bloodvessel> bloodvessel, Vector location) {
    Ptr<Nanobot> tempNB = CreateObject<Nanobot>();
    tempNB->SetNanobotID(GetNextNanobotID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    bloodvessel->AddNanobotToStream(streamID, tempNB);
}

void Bloodcircuit::AddNanocollector(int streamID,
                                    Ptr<Bloodvessel> bloodvessel,
                                    Vector location, int counter) {
    Ptr<Nanocollector> tempNB = CreateObject<Nanocollector>();
    tempNB->SetNanobotID(GetNextNanobotID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    tempNB->SetTargetOrgan(m_fingerprint_organs[counter % 9]);
    bloodvessel->AddNanobotToStream(streamID, tempNB);
}

void Bloodcircuit::AddNanolocator(int streamID,
                                  Ptr<Bloodvessel> bloodvessel, Vector location,
                                  int counter) {
    Ptr<Nanolocator> tempNB = CreateObject<Nanolocator>();
    tempNB->SetNanobotID(GetNextNanobotID());
    tempNB->SetShouldChange(false);
    tempNB->SetPosition(location);
    tempNB->SetTargetOrgan(m_fingerprint_organs[counter % 9]);
    bloodvessel->AddNanobotToStream(streamID, tempNB);
}

void Bloodcircuit::AddNanoparticle(unsigned int vesselID,
                                   double delay,
                                   double detectionRadius, int streamID) {
    Ptr<Bloodvessel> vessel = m_bloodvessels[vesselID];
    Vector coordinateVessel = vessel->GetStartPositionBloodvessel();
    Ptr<Nanoparticle> tempNP = CreateObject<Nanoparticle>();
    tempNP->SetNanobotID(GetNextNanobotID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Vector(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    tempNP->SetDelay(delay);
    tempNP->SetDetectionRadius(detectionRadius);
    vessel->AddNanobotToStream(streamID, tempNP);
}

void Bloodcircuit::AddCancerCell(unsigned int vesselID, int streamID) {
    Ptr<Bloodvessel> vessel = m_bloodvessels[vesselID];
    Vector coordinateVessel = vessel->GetStartPositionBloodvessel();
    Ptr<CancerCell> tempNP = CreateObject<CancerCell>();
    tempNP->SetNanobotID(GetNextNanobotID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Vector(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddNanobotToStream(streamID, tempNP);
}

void Bloodcircuit::AddTCell(unsigned int vesselID, int streamID) {
    Ptr<Bloodvessel> vessel = m_bloodvessels[vesselID];
    Vector coordinateVessel = vessel->GetStartPositionBloodvessel();
    Ptr<TCell> tempNP = CreateObject<TCell>();
    tempNP->SetNanobotID(GetNextNanobotID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Vector(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddNanobotToStream(streamID, tempNP);
}

void Bloodcircuit::AddCarTCell(unsigned int vesselID, int streamID) {
    Ptr<Bloodvessel> vessel = m_bloodvessels[vesselID];
    Vector coordinateVessel = vessel->GetStartPositionBloodvessel();
    Ptr<CarTCell> tempNP = CreateObject<CarTCell>();
    tempNP->SetNanobotID(GetNextNanobotID());
    tempNP->SetShouldChange(false);
    tempNP->SetPosition(
        Vector(coordinateVessel.x, coordinateVessel.y, coordinateVessel.z));
    vessel->AddNanobotToStream(streamID, tempNP);
}

void Bloodcircuit::AddCarTCellInjectionToVessel(unsigned int numberOfCarTCells,
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
        Ptr<UniformRandomVariable> distribute_randomly =
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

double Bloodcircuit::GetSpeedClassOfBloodVesselType(BloodvesselType type) {
    if (type == ARTERY)
        return 10.0;
    else if (type == VEIN)
        return 3.7;
    else // if (type == ORGAN)
        return 1.0;
}

void Bloodcircuit::ReadInBloodcircuit(string fileName) {
    std::ifstream infile{fileName};
    if (infile.good()) {
        try {
            std::vector<int> numbers;
            numbers.resize(valuesPerLine);
            std::string buffer;
            buffer.reserve(64);
            int errorflag = 0;
            while (infile.good()) {
                for (auto &&elem : numbers) {
                    if (std::getline(infile, buffer, ',')) {
                        elem = std::stoi(buffer);
                    } else {
                        elem = 0;
                        errorflag = 1;
                    }
                }
                if (errorflag == 0)
                    AddVesselData(numbers[0], (BloodvesselType)numbers[1],
                                  Vector(numbers[2], numbers[3], numbers[4]),
                                  Vector(numbers[5], numbers[6], numbers[7]));
                // male
                //  if (errorflag == 0)
                //  {
                //    AddVesselData (numbers[0], (BloodvesselType)
                //    numbers[1],
                //                   Vector (round(numbers[2]*1.04),
                //                   round(numbers[3]*1.04), numbers[4]),
                //                   Vector (round(numbers[5]*1.04),
                //                   round(numbers[6]*1.04), numbers[7]));
                //  }
                // female
                //  if (errorflag == 0)
                //  {
                //    AddVesselData (numbers[0], (BloodvesselType)
                //    numbers[1],
                //                   Vector (round(numbers[2]*0.96),
                //                   round(numbers[3]*0.96), numbers[4]),
                //                   Vector (round(numbers[5]*0.96),
                //                   round(numbers[6]*0.96), numbers[7]));
                //  }
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

void Bloodcircuit::AddVesselData(int id, BloodvesselType type, Vector start,
                                 Vector stop) {
    Ptr<Bloodvessel> vessel = CreateObject<Bloodvessel>();
    vessel->SetBloodvesselID(id);
    vessel->SetBloodvesselType(type);
    vessel->SetStartPositionBloodvessel(start);
    vessel->SetStopPositionBloodvessel(stop);
    vessel->SetVesselWidth(0.25); // 0.25
    vessel->SetPrinter(printer);
    // Init Bloodvessel: Calculate length and angle & velocity.
    vessel->InitBloodstreamLengthAngleAndVelocity(
        GetSpeedClassOfBloodVesselType(type));
    m_bloodvessels[id] = vessel;
    cout << "New Vessel(" + to_string(id) + "," + to_string(type) + "," +
                to_string(start.x) + "," + to_string(start.y) + "," +
                to_string(start.z) + "," + to_string(stop.x) + "," +
                to_string(stop.y) + "," + to_string(stop.z) + ")"
         << endl;
}

void Bloodcircuit::InitialiseBloodvessels(int vesseldata[][8],
                                          unsigned int count) {
    for (unsigned int i = 0; i < count; i++)
        AddVesselData(
            vesseldata[i][0], (BloodvesselType)vesseldata[i][1],
            Vector(vesseldata[i][2], vesseldata[i][3], vesseldata[i][4]),
            Vector(vesseldata[i][5], vesseldata[i][6], vesseldata[i][7]));
}

void Bloodcircuit::ConnectBloodvessels() {
    unsigned int count = m_bloodvessels.size();
    cout << count << endl;
    // Set Connections between bloodvessels if they have the same start/end
    // coordinates
    for (unsigned int i = 1; i < count + 1; i++) {
        unsigned int counter = 0;
        Vector end = m_bloodvessels[i]->GetStopPositionBloodvessel();
        // Make sure that inititally all Bloodvessels have no Fingerprint
        // Formation Times
        m_bloodvessels[i]->SetFingerprintFormationTime(0);
        for (unsigned int j = 1; j < count + 1; j++) {
            Vector start = m_bloodvessels[j]->GetStartPositionBloodvessel();
            if (end.x == start.x && end.y == start.y && end.z == start.z) {
                counter++;
                if (counter == 1)
                    m_bloodvessels[i]->SetNextBloodvessel1(m_bloodvessels[j]);
                else
                    m_bloodvessels[i]->SetNextBloodvessel2(m_bloodvessels[j]);
            }
        }
    }
}

void Bloodcircuit::SetTransitionProbabilities() {
    std::ifstream infile{"transitions95.csv"};
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
        cout << "load transitions" << endl;
    } else { // Old way of loading data
        cout << "NO VALID CSV FILE FOUND! " << endl;
        cout << "Transitions set to 1 and 0" << endl;
    }
}
void Bloodcircuit::SetFingerprintTimes() {
    std::ifstream infile{"fingerprint.csv"};
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
        cout << "load fingerprint times" << endl;
    } else { // Old way of loading data
        cout << "NO VALID CSV FILE FOUND! " << endl;
        cout << "Fingerprint times not added" << endl;
    }
}

void Bloodcircuit::SetGatewayVessel(int detectionVesselID) {
    m_bloodvessels[detectionVesselID]->SetIsGatewayVessel(true);
}

void Bloodcircuit::PrintStatistics() {
    int cancerCells = 0;
    int carTCells = 0;
    for (int i = 0; i < m_bloodvessels.size(); i++) {
        cancerCells += m_bloodvessels[i]->CountCancerCells();
        carTCells += m_bloodvessels[i]->CountCarTCells();
    }
    cout << "Cancer Cells: " << cancerCells << ", CAR-T Cells: " << carTCells
         << endl;
}

unsigned int Bloodcircuit::GetNextNanobotID() {
    return IDCounter::GetNextNanobotID(); 
}

} // namespace ns3

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

#include "BloodVessel.h"

namespace bloodcircuit {


BloodVessel::BloodVessel() {
    m_deltaT = 1;
    m_stepsPerSec = 1 / m_deltaT;
    m_secStepCounter = 0;
    initStreams();
    m_changeStreamSet = true;
    m_basevelocity = 0;
    m_transitionto1 = 1;
    m_transitionto2 = 0;
    m_hasActiveFingerprintMessage = false;
    m_isGatewayVessel = false;
    injection.m_injectionTime = -1;
    injection.m_injectionVessel = -1;
    injection.m_injectionNumber = -1;
    m_fingerPrintTimer = -1;
}

BloodVessel::~BloodVessel() {
}

void BloodVessel::SetPrinter(shared_ptr<Printer> printer) {
    this->printer = printer;
}

shared_ptr<BloodVessel> BloodVessel::Step(uint64_t timeInS) {
    this->CheckFingerprintRelease();
    this->CheckParticleInteractions();
    this->CountStepsAndAgeCells();
    this->TranslateParticles();
    this->PerformCellMitosis();

    // to initiate a second particle release, change variable
    // secondParticleRelease from false to true
    bool secondParticleRelease = 0;
    if (secondParticleRelease && this->GetbloodvesselID() == 36 
                              && timeInS == 600)
            this->ReleaseParticles();

    if (this->injection.m_injectionVessel > 0) {
        if (this->injection.m_injectionTime <= timeInS) {
            cout << "Injecting CAR-T cells now" << endl;
            cout << "Vessel ID: " << this->GetbloodvesselID() << endl;
            this->PerformInjection();
            this->injection.m_injectionVessel = -1;
        }
    }

    if (this->IsEmpty())
        return nullptr;
    else
        return shared_from_this();
}

Position BloodVessel::SetPosition(Position nbv, double distance, double angle,
                                int bloodvesselType, double startPosZ) {
    // Check vessel direction and move according to distance.
    // right
    if (angle == 0.00 && bloodvesselType != ORGAN) {
        nbv.x += distance;
    }
    // left
    else if (angle == -180.00 || angle == 180.00) {
        nbv.x -= distance;
    }
    // down
    else if (angle == -90.00) {
        nbv.y -= distance;
    }
    // up
    else if (angle == 90.00) {
        nbv.y += distance;
    }
    // back
    else if (angle == 0.00 && bloodvesselType == ORGAN && startPosZ == 2) {
        nbv.z -= distance;
    }
    // front
    else if (angle == 0.00 && bloodvesselType == ORGAN && startPosZ == -2) {
        nbv.z += distance;
    }
    // right up
    else if ((0.00 < angle && angle < 90.00) ||
             (-90.00 < angle && angle < 0.00) ||
             (90.00 < angle && angle < 180.00) ||
             (-180.00 < angle && angle < -90.00)) {
        nbv.x += distance * (cos(fmod((angle), 360) * M_PI / 180));
        nbv.y += distance * (sin(fmod((angle), 360) * M_PI / 180));
    }
    return nbv;
}

void BloodVessel::TranslateParticles() {
    // FIXME: WAS IST DAS HIER????? 
    //       Warum kann global nur jeder zweite Particle springen?
    static int loop = 1;
    if (loop == 2)
        loop = 0;
    // Change streams only in organs
    if (loop == 0 && m_changeStreamSet == true &&
        this->GetBloodVesselType() == ORGAN)
            ChangeStream();
    // Translate their position every timestep
    TranslatePosition(m_deltaT);
    loop++;
}

bool BloodVessel::moveParticle(shared_ptr<Particle> nb, int i, int randVelocityOffset,
                              bool direction, double dt) {
    double distance = 0.0;
    double velocity = m_bloodstreams[i]->GetVelocity();
    if (nb->GetDelay() >= 0)
        velocity = velocity * (nb->GetDelay());

    if (direction)
        distance = (velocity - ((velocity / 100) * randVelocityOffset)) * dt;
    else
        distance = (velocity + ((velocity / 100) * randVelocityOffset)) * dt;
    // Check vessel direction and move accordingly.
    Position nbv =
        SetPosition(nb->GetPosition(), distance, GetBloodVesselAngle(),
                    GetBloodVesselType(), m_startPositionBloodVessel.z);
    nb->SetPosition(nbv);
    nb->SetTimeStep();
    double nbx = nb->GetPosition().x - m_startPositionBloodVessel.x;
    double nby = nb->GetPosition().y - m_startPositionBloodVessel.y;
    double length = sqrt(nbx * nbx + nby * nby);
    // check if position exceeds bloodvessel
    return (length > m_bloodvesselLength || (nbv.z < -2 && m_angle == 0) ||
            (nbv.z > 2 && m_angle == 0));
}

void BloodVessel::TranslatePosition(double dt) {
    list<shared_ptr<Particle>> print;
    int numCarTCells = 0;
    int numCancerCells = 0;
    // perform interaction between CarTCells and Cancer Cells
    for (int i = 0; i < m_numberOfStreams; i++) {
        for (uint j = 0; j < m_bloodstreams[i]->CountParticles(); j++) {
            shared_ptr<Particle> nb = m_bloodstreams[i]->GetParticle(j);
            if (nb->particleType == CarTCellType) {
                // cout << "Found CarTCell" << endl;
                shared_ptr<Particle> nb = m_bloodstreams[i]->GetParticle(j);
                if (!nb->IsAlive()) {
                    m_bloodstreams[i]->RemoveParticle(nb);
                    j =-1;
                    continue;
                }
                shared_ptr<CarTCell> ctc = dynamic_pointer_cast<CarTCell>(nb);
                if (ctc == NULL)
                    continue;
                for (int k = 0; k < m_numberOfStreams; k++) {
                    for (uint l = 0; l < m_bloodstreams[k]->CountParticles();
                         l++) {
                        shared_ptr<Particle> nb2 = m_bloodstreams[k]->GetParticle(l);
                        ctc->AddPossibleMitosis(nb2->particleType);
                        switch (nb2->particleType) {
                        case CancerCellType: {
                            // cout << "Found CancerCell" << endl;
                            shared_ptr<CancerCell> cc = dynamic_pointer_cast<CancerCell>(nb2);
                            if (cc == NULL)
                                continue;
                            double dist = CalcDistance(nb, nb2);
                            if (dist <= nb2->GetDetectionRadius()) {
                                if (ctc->KillCancerCell() == true) {
                                    // cout << "Killing CancerCell" << endl;
                                    cc->GetsDetected();
                                    m_bloodstreams[k]->RemoveParticle(nb2);
                                    l -= 1;
                                }
                            }
                            break;
                        }
                        case TCellType: {
                            // cout << "Found TCell" << endl;
                            shared_ptr<TCell> tc = dynamic_pointer_cast<TCell>(nb2);
                            if (tc == NULL)
                                continue;
                            double dist = CalcDistance(nb, nb2);
                            if (dist <= tc->GetDetectionRadius()) {
                                if (ctc->KillTCell() == true) {
                                    // cout << "Killing TCell" << endl;
                                    tc->GetsDetected();
                                    m_bloodstreams[k]->RemoveParticle(nb2);
                                    l -= 1;
                                }
                            }
                            break;
                        }
                        case CarTCellType: {
                            // cout << "Found other CarTCell" << endl;
                            shared_ptr<CarTCell> ctc2 = dynamic_pointer_cast<CarTCell>(nb2);
                            if (ctc2 == NULL)
                                continue;
                            double dist = CalcDistance(nb, nb2);
                            if (dist <= 0) {
                                if (ctc->KillCarTCell() == true) {
                                    // cout << "Killing other CarTCell" << endl;
                                    m_bloodstreams[k]->RemoveParticle(nb2);
                                    l -= 1;
                                }
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    // for every stream of the vessel
    for (int i = 0; i < m_numberOfStreams; i++) {
        // for every nanobot of the stream
        for (uint j = 0; j < m_bloodstreams[i]->CountParticles(); j++) {
            shared_ptr<Particle> nb = m_bloodstreams[i]->GetParticle(j);
            shared_ptr<CarTCell> ctc = dynamic_pointer_cast<CarTCell>(nb);
            if (nb->particleType == CarTCellType) {
                shared_ptr<CarTCell>ctc = dynamic_pointer_cast<CarTCell>(nb);
                if (ctc != NULL && ctc->IsActive())
                    numCarTCells++;
            }
            if (nb->particleType == CancerCellType)
                numCancerCells++;
            shared_ptr<CancerCell> cc = dynamic_pointer_cast<CancerCell>(nb);
            if (cc != NULL && cc->MustBeDeleted()) {
                m_bloodstreams[i]->RemoveParticle(nb);
                j -= 1;
                continue;
            }
            // move only nanobots that have not already been translated by
            // another vessel
            if (nb->GetTimeStepInSeconds() < GlobalTimer::NowInSeconds()) {
                // has nanobot reached end after moving
                if (moveParticle(nb, i, Randomizer::GetRandomValue(0,11),
                                Randomizer::GetRandomBoolean(), dt)) {
                    
                    reachedEndMap[i].push_back(nb);
                    m_bloodstreams[i]->RemoveParticle(nb);
                } else {
                    print.push_back(nb);
                }
            }
        }
    }
    printer->PrintParticles(print, this->GetbloodvesselID());
    if (m_isGatewayVessel == true || m_bloodvesselID == 1)
        printer->PrintGateway(m_bloodvesselID, numCancerCells, numCarTCells);
}

void BloodVessel::ChangeStream() {
    if (m_numberOfStreams > 1) {
        // set half of the nanobots randomly to change
        for (int i = 0; i < m_numberOfStreams; i++) {
            for (uint j = 0; j < m_bloodstreams[i]->CountParticles(); j++) {
                if (Randomizer::GetRandomBoolean())
                    m_bloodstreams[i]->GetParticle(j)->SetShouldChange(true);
            }
        }
        // after all nanobots that should change are flagged, do change
        for (int i = 0; i < m_numberOfStreams; i++) {
            int direction = Randomizer::GetRandomBoolean() == true ? -1 : 1;
            if (i == 0) // Special Case 1: outer lane left -> go to middle
                direction = 1;
            else if (i + 1 >= m_numberOfStreams) // Special Case 2: outer lane 
                                                 // right -> go to middle
                direction = -1;
            // Move randomly left or right
            DoChangeStreamIfPossible(i, i + direction);
        }
    }
}

void BloodVessel::DoChangeStreamIfPossible(int curStream, int desStream) {
    list<shared_ptr<Particle>> canChange;
    canChange.clear();
    for (uint j = 0; j < m_bloodstreams[curStream]->CountParticles(); j++) {
        if (m_bloodstreams[curStream]->GetParticle(j)->GetShouldChange()) {
            // set should change back to false
            m_bloodstreams[curStream]->GetParticle(j)->SetShouldChange(false);
            m_bloodstreams[desStream]->AddParticle(
                m_bloodstreams[curStream]->RemoveParticle(j));
        }
    }
    // Sort all Particles by ID
    m_bloodstreams[desStream]->SortStream();
}

bool BloodVessel::transposeParticle(shared_ptr<Particle> botToTranspose,
                                   shared_ptr<BloodVessel> thisBloodVessel,
                                   shared_ptr<BloodVessel> nextBloodVessel,
                                   int stream) {
    Position stopPositionOfVessel = thisBloodVessel->
        GetStopPositionBloodVessel();
    Position nanobotPosition = botToTranspose->GetPosition();
    double distance = sqrt(pow(nanobotPosition.x - stopPositionOfVessel.x, 2) +
                           pow(nanobotPosition.y - stopPositionOfVessel.y, 2) +
                           pow(nanobotPosition.z - stopPositionOfVessel.z, 2));
    distance = distance /
               thisBloodVessel->m_bloodstreams[stream]->GetVelocity() *
               nextBloodVessel->m_bloodstreams[stream]->GetVelocity();
    botToTranspose->SetPosition(nextBloodVessel->GetStartPositionBloodVessel());
    Position rmp = SetPosition(botToTranspose->GetPosition(), distance,
                             nextBloodVessel->GetBloodVesselAngle(),
                             nextBloodVessel->GetBloodVesselType(),
                             thisBloodVessel->GetStopPositionBloodVessel().z);
    botToTranspose->SetPosition(rmp);
    double nbx = botToTranspose->GetPosition().x -
                 nextBloodVessel->GetStartPositionBloodVessel().x;
    double nby = botToTranspose->GetPosition().y -
                 nextBloodVessel->GetStartPositionBloodVessel().y;
    double length = sqrt(nbx * nbx + nby * nby);
    // check if position exceeds bloodvessel
    return length > nextBloodVessel->GetbloodvesselLength() || rmp.z < -2 ||
           rmp.z > 2;
}

bool BloodVessel::NeedsTransferStep() {
    return reachedEndMap.size() > 0;
}

void BloodVessel::PerformTransferStep(){
    for (auto & x : reachedEndMap) {
        if (x.second.size() > 0) {
            TransferStep(x.second, x.first);
        }
        x.second.clear();
    }
    reachedEndMap.clear();
}

void BloodVessel::TransferStep(list<shared_ptr<Particle>> reachedEnd, 
                               int stream) {
    list<shared_ptr<Particle>> print1;
    list<shared_ptr<Particle>> print2;
    list<shared_ptr<Particle>> reachedEndAgain;

    int onetwo = -1;
    for (const shared_ptr<Particle> &botToTranspose : reachedEnd) {
        onetwo = Randomizer::GetRandomValue(0,100000);
        // to v2
        if (m_nextBloodVessel2 != 0 && (onetwo >= m_transitionto1 * 100000)) {
            // fits next vessel?
            if (transposeParticle(botToTranspose, shared_from_this(), 
                                 m_nextBloodVessel2, stream)) {
                reachedEndAgain.push_back(botToTranspose);
                m_nextBloodVessel2->TransferStep(reachedEndAgain, stream);
                reachedEndAgain.clear();
            } else {
                m_nextBloodVessel2->m_bloodstreams[stream]->AddParticle(
                    botToTranspose);
                print2.push_back(botToTranspose);
            }
        }
        // to v1
        else if (m_nextBloodVessel2 == 0 || onetwo < m_transitionto1 * 100000) {
            // fits next vessel?
            if (transposeParticle(botToTranspose, shared_from_this(), 
                                 m_nextBloodVessel1, stream)) {
                reachedEndAgain.push_back(botToTranspose);
                m_nextBloodVessel1->TransferStep(reachedEndAgain, stream);
                reachedEndAgain.clear();
            } else {
                m_nextBloodVessel1->m_bloodstreams[stream]->AddParticle(
                    botToTranspose);
                print1.push_back(botToTranspose);
            }
        }
        else {
            cout << "ERROR! Did not transpose particle correctly" << endl;
        }
    }

    printer->PrintParticles(print1, m_nextBloodVessel1->GetbloodvesselID());
    if (m_nextBloodVessel2 != 0)
        printer->PrintParticles(print2, m_nextBloodVessel2->GetbloodvesselID());
}

list<shared_ptr<Particle>> BloodVessel::GetParticles() {
    list<shared_ptr<Particle>> bots;
    for (uint j = 0; j < m_bloodstreams.size(); j++) {
        for (uint i = 0; i < m_bloodstreams[j]->CountParticles(); i++)
            bots.push_back(m_bloodstreams[j]->GetParticle(i));
    }
    return bots;
}

void BloodVessel::CheckParticleInteractions() {
    list<shared_ptr<Particle>> bots = GetParticles();
    if (this->GetFingerprintFormationTime() > 0)
        this->CheckRelease(bots);
    if (this->isActive())
        this->CheckCollect(bots);
    this->CheckDetect(bots);
}

// HELPER
void BloodVessel::PrintParticlesOfVessel() {
    for (uint j = 0; j < m_bloodstreams.size(); j++) {
        for (uint i = 0; i < m_bloodstreams[j]->CountParticles(); i++)
            printer->PrintParticle(m_bloodstreams[j]->GetParticle(i),
                                  GetbloodvesselID());
    }
}

void BloodVessel::initStreams() {
    int i;
    shared_ptr<Bloodstream> stream;
    for (i = 0; i < stream_definition_size; i++) {
        stream = make_shared<Bloodstream>();
        stream->initBloodstream(m_bloodvesselID, i, stream_definition[i][0],
                                stream_definition[i][1] / 10.0,
                                stream_definition[i][2] / 10.0,
                                GetBloodVesselAngle());
        m_bloodstreams.push_back(stream);
    }
    m_numberOfStreams = stream_definition_size;
}

double BloodVessel::CalcLength() {
    if (GetBloodVesselType() == ORGAN) {
        return 4;
    } else {
        Position m_start = GetStartPositionBloodVessel();
        Position m_end = GetStopPositionBloodVessel();
        double l =
            sqrt(pow(m_end.x - m_start.x, 2) + pow(m_end.y - m_start.y, 2));
        return l;
    }
}

double BloodVessel::CalcDistance(shared_ptr<Particle> n_1, 
                                 shared_ptr<Particle> n_2) {
    Position v_1 = n_1->GetPosition();
    Position v_2 = n_2->GetPosition();
    double l = sqrt(pow(v_1.x - v_2.x, 2) + pow(v_1.y - v_2.y, 2));
    return l;
}

double BloodVessel::CalcAngle() {
    Position m_start = GetStartPositionBloodVessel();
    Position m_end = GetStopPositionBloodVessel();
    double x = m_end.x - m_start.x;
    double y = m_end.y - m_start.y;
    return atan2(y, x) * 180 / M_PI;
}

void BloodVessel::CountStepsAndAgeCells() {
    m_secStepCounter++;
    if (m_secStepCounter >= m_stepsPerSec) {
        m_secStepCounter = 0;
        int secCount = 1;
        if (m_stepsPerSec < 0)
            secCount = m_deltaT;
        for (int i = 0; i < m_numberOfStreams; i++) {
            for (uint j = 0; j < m_bloodstreams[i]->CountParticles(); j++) {
                shared_ptr<Particle> nb = m_bloodstreams[i]->GetParticle(j);
                if (nb->CanAge() && !nb->Age(secCount)) {
                    m_bloodstreams[i]->RemoveParticle(nb);
                    j -= 1;
                }
            }
        }
    }
}

void BloodVessel::PerformCellMitosis() {
    for (int i = 0; i < m_numberOfStreams; i++) {
        for (uint j = 0; j < m_bloodstreams[i]->CountParticles(); j++) {
            shared_ptr<Particle> nb = m_bloodstreams[i]->GetParticle(j);
            if (nb->WillPerformMitosis()) {
                switch (nb->particleType) {
                case CarTCellType: {
                    Position m_coordinates = 
                        this->GetStartPositionBloodVessel();
                    //Position m_coordinates = nb->GetPosition();
                    shared_ptr<CarTCell> cell = make_shared<CarTCell>();
                    cell->SetParticleID(IDCounter::GetNextParticleID());
                    cell->SetShouldChange(false);
                    cell->SetPosition(Position(m_coordinates.x, 
                                             m_coordinates.y, 
                                             m_coordinates.z));
                    this->AddParticleToStream(i, cell);
                    break;
                }
                case CancerCellType: {
                    Position m_coordinates = 
                        this->GetStartPositionBloodVessel();
                    //Position m_coordinates = nb->GetPosition();
                    shared_ptr<CancerCell> cell = make_shared<CancerCell>();
                    cell->SetParticleID(IDCounter::GetNextParticleID());
                    cell->SetShouldChange(false);
                    cell->SetPosition(Position(m_coordinates.x, 
                                             m_coordinates.y, 
                                             m_coordinates.z));
                    this->AddParticleToStream(i, cell);
                    break;
                }
                default:
                    break;
                }
                nb->ResetMitosis();
            }
        }
    }
    
}

void BloodVessel::InitBloodstreamLengthAngleAndVelocity(double velocity) {
    int i;
    double length = CalcLength();
    m_bloodvesselLength = length < 0 ? 10000 : length;
    m_angle = CalcAngle();

    if (velocity >= 0) {
        m_basevelocity = velocity;
        int maxLength = 0;
        for (i = 0; i < m_numberOfStreams; i++) {
            if (maxLength < stream_definition[i][1])
                maxLength = stream_definition[i][1];
            if (maxLength < stream_definition[i][2])
                maxLength = stream_definition[i][2];
        }
        double offset = m_vesselWidth / 2.0 / maxLength;
        // change velocity for the heart
        if (m_bloodvesselID == 2 || m_bloodvesselID == 58)
            m_basevelocity = 5; // duration of one cardiac cycle 0.8 seconds and
                                // 4 cm distance.
        // Set velocity, angle and position offset
        for (i = 0; i < m_numberOfStreams; i++) {
            m_bloodstreams[i]->SetVelocity(m_basevelocity);
            m_bloodstreams[i]->SetAngle(m_angle,
                                        stream_definition[i][1] * offset,
                                        stream_definition[i][2] * offset);
        }
    }
}

void BloodVessel::CheckRelease(list<shared_ptr<Particle>> nbToCheck) {
    for (const shared_ptr<Particle> &bot : nbToCheck) {
        if (bot->HasFingerprintLoaded()) {
            if (bot->GetTargetOrgan() == m_bloodvesselID) {
                SetFingerprintRelease(m_fingerprintFormationTime);
                // When one NanoLocator reached the vessel it is assumed that
                // others will follow and the signal is strong enough. So the
                // vessel doesn't look for more nanolocators
                // m_fingerprintFormationflag = false;
                bot->releaseFingerprintTiles();
            }
        }
    }
}

void BloodVessel::CheckCollect(list<shared_ptr<Particle>> nbToCheck) {
    for (const shared_ptr<Particle> &bot : nbToCheck) {
        // Bot is nanocollector
        if (bot->particleType == NanocollectorType) {
            if (bot->GetTargetOrgan() == m_bloodvesselID)
                // std::cout << "Nanocollector: " << bot->GetParticleID() << "
                // collects message " << bot->GetTargetOrgan() << std::endl;
                bot->collectMessage();
        }
    }
}

void BloodVessel::CheckDetect(list<shared_ptr<Particle>> nbToCheck) {
    for (const shared_ptr<Particle> &particle : nbToCheck) {
        // Bot is nanoparticle
        if (particle->particleType == NanoparticleType) {
            double x = particle->GetPosition().x;
            double y = particle->GetPosition().y;
            double z = particle->GetPosition().z;
            double detectionRadius = particle->GetDetectionRadius();

            for (const shared_ptr<Particle> &bot : nbToCheck) {
                // is Particle
                if (bot->particleType == BaseParticleType) {
                    double bx = bot->GetPosition().x;
                    double by = bot->GetPosition().y;
                    double bz = bot->GetPosition().z;

                    // calculate distance
                    double distance =
                        sqrt(pow(x - bx, 2) + pow(y - by, 2) + pow(z - bz, 2));
                    // is in radius of Detection
                    if (distance < detectionRadius)
                        particle->GetsDetected();
                }
            }
        }
    }
}

bool BloodVessel::IsEmpty() {
    bool empty = true;
    for (int i = 0; i < m_numberOfStreams; i++)
        empty = empty && m_bloodstreams[i]->IsEmpty();
    return empty;
}

int BloodVessel::GetbloodvesselID() { return m_bloodvesselID; }

void BloodVessel::SetBloodVesselID(int b_id) { m_bloodvesselID = b_id; }

double BloodVessel::GetBloodVesselAngle() { return m_angle; }

int BloodVessel::GetNumberOfStreams() { return m_numberOfStreams; }

shared_ptr<Bloodstream> BloodVessel::GetStream(int id) { 
    return m_bloodstreams[id]; 
}

double BloodVessel::GetbloodvesselLength() { return m_bloodvesselLength; }

void BloodVessel::SetVesselWidth(double value) { m_vesselWidth = value; }

void BloodVessel::AddParticleToStream(unsigned int streamID, 
                                     shared_ptr<Particle> bot) {
    m_bloodstreams[streamID]->AddParticle(bot);
}

BloodVesselType BloodVessel::GetBloodVesselType() { return m_bloodvesselType; }

void BloodVessel::SetBloodVesselType(BloodVesselType value) {
    m_bloodvesselType = value;
}

void BloodVessel::SetNextBloodVessel1(shared_ptr<BloodVessel> value) {
    m_nextBloodVessel1 = value;
}

void BloodVessel::SetNextBloodVessel2(shared_ptr<BloodVessel> value) {
    m_nextBloodVessel2 = value;
}

void BloodVessel::SetTransition1(double value) { m_transitionto1 = value; }

void BloodVessel::SetTransition2(double value) { m_transitionto2 = value; }
void BloodVessel::SetFingerprintFormationTime(double value) {
    m_fingerprintFormationTime = value;
}

double BloodVessel::GetFingerprintFormationTime() {
    return m_fingerprintFormationTime;
}

Position BloodVessel::GetStartPositionBloodVessel() {
    return m_startPositionBloodVessel;
}

void BloodVessel::SetStartPositionBloodVessel(Position value) {
    m_startPositionBloodVessel = value;
}

Position BloodVessel::GetStopPositionBloodVessel() {
    return m_stopPositionBloodVessel;
}

void BloodVessel::SetStopPositionBloodVessel(Position value) {
    m_stopPositionBloodVessel = value;
}

bool BloodVessel::IsGatewayVessel() { return m_isGatewayVessel; }

void BloodVessel::SetIsGatewayVessel(bool value) { m_isGatewayVessel = value; }

void BloodVessel::SetFingerprintRelease(double time) {
    if (m_fingerPrintTimer < 0)
        m_fingerPrintTimer = time;
}

void BloodVessel::CheckFingerprintRelease() {
    if (m_fingerPrintTimer > 0) {
        m_fingerPrintTimer -= m_deltaT;
        if (m_fingerPrintTimer <= 0) {
            m_hasActiveFingerprintMessage = true;
            std::cout << "Timer expired! Fingerprint message received "
                      << m_hasActiveFingerprintMessage
                      << " in organ: " << m_bloodvesselID << std::endl;
        }
    }
}

bool BloodVessel::isActive() { return m_hasActiveFingerprintMessage; }

void BloodVessel::ReleaseParticles() {
    // release more particles
    shared_ptr<RandomStream> distribute_randomly =
        Randomizer::GetNewRandomStream(0, this->GetNumberOfStreams());
    // release particles from the liver Organ 36
    for (int i = 1; i <= 100; ++i) {
        // shared_ptr<BloodVessel> liver = m_bloodvessels[36];
        Position m_coordinateLiver = this->GetStartPositionBloodVessel();
        shared_ptr<Particle> temp_np = make_shared<Nanoparticle>();
        temp_np->SetParticleID(IDCounter::GetNextParticleID());
        // Get random stream number.
        int dr = floor(distribute_randomly->GetValue());
        temp_np->SetShouldChange(false);
        temp_np->SetPosition(Position(m_coordinateLiver.x, m_coordinateLiver.y,
                                      m_coordinateLiver.z));
        // Set Speed and Detection Radius of particles
        temp_np->SetDelay(2.32);
        temp_np->SetDetectionRadius(0.2);
        // Set position with random stream dr.
        this->AddParticleToStream(dr, temp_np);
    }
}

void BloodVessel::PerformInjection() {
    shared_ptr<RandomStream> distribute_randomly =
        Randomizer::GetNewRandomStream(0, this->GetNumberOfStreams());
    for (int i = 1; i <= injection.m_injectionNumber; ++i) {
        Position m_coordinates = this->GetStartPositionBloodVessel();
        shared_ptr<CarTCell> cell = make_shared<CarTCell>();
        cell->SetParticleID(IDCounter::GetNextParticleID());
        cell->SetShouldChange(false);
        cell->SetPosition(
            Position(m_coordinates.x, m_coordinates.y, m_coordinates.z));
        int dr = floor(distribute_randomly->GetValue());
        this->AddParticleToStream(dr, cell);
    }
}

void BloodVessel::AddCarTCellInjection(int injectionTime, int injectionVessel,
                                       int numberOfCarTCells) {
    injection.m_injectionTime = injectionTime;
    injection.m_injectionVessel = injectionVessel;
    injection.m_injectionNumber = numberOfCarTCells;
}

int BloodVessel::CountCancerCells() {
    int cancerCells = 0;
    for (int i = 0; i < m_numberOfStreams; i++)
        cancerCells += m_bloodstreams[i]->CountCancerCells();
    return cancerCells;
}

int BloodVessel::CountCarTCells() {
    int carTCells = 0;
    for (int i = 0; i < m_numberOfStreams; i++)
        carTCells += m_bloodstreams[i]->CountCarTCells();
    return carTCells;
}

void BloodVessel::ExchangeParticles(std::vector<shared_ptr<Particle>> newBots) {
    int numStreams = m_bloodstreams.size();
    for (int i = 0; i < numStreams; i++)
        m_bloodstreams[i]->ClearStream();
    
    for (std::shared_ptr<Particle> bot : newBots) {
        if (bot->GetStream() >= 0)
            m_bloodstreams[bot->GetStream()]->AddParticle(bot);
        else
            m_bloodstreams[Randomizer::GetRandomIntegerValue(0,numStreams)]->AddParticle(bot);
    }
}

} // namespace bloodcircuit

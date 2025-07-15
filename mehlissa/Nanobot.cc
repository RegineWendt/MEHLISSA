/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [GEYER]
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

#include "Nanobot.h"

namespace ns3 {
// TypeId Nanobot::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::Nanobot")
//   .SetParent<Object> ()
//   .AddConstructor<Nanobot> ()
//   ;
//   return tid;
// }

Nanobot::Nanobot() {
    m_type = NanobotType;
    m_node = CreateObject<Node>();
    // enables mobility
    MobilityHelper mobility;
    mobility.Install(m_node);

    m_length = 0.00001; // 100nm
    m_width = 0.00001;  // 100nm
    m_stream_nb = 0;
    m_shouldChange = false;
    m_timeStep = Seconds(0.0);

    m_canAge = false;
    m_maxAge = 0;
    m_ageCounter = m_maxAge;
    m_willPerformMitosis = false;
    m_mitosisTime = -1;
    m_mitosisCounter = 0;
}

Nanobot::~Nanobot() {}

bool Nanobot::Compare(Ptr<Nanobot> v1, Ptr<Nanobot> v2) {
    if (v1->GetNanobotID() < v2->GetNanobotID())
        return true;
    else
        return false;
}

int Nanobot::GetNanobotID() { return m_nanobotID; }

void Nanobot::SetNanobotID(int value) { m_nanobotID = value; }

double Nanobot::GetLength() { return m_length; }

void Nanobot::SetLength(double value) {
    if (value < 0)
        value = 0;
    m_length = value;
}

double Nanobot::GetWidth() { return m_width; }

void Nanobot::SetWidth(double value) {
    if (value < 0)
        value = 0;
    m_width = value;
}

void Nanobot::SetStream(int value) { m_stream_nb = value; }

int Nanobot::GetStream() { return m_stream_nb; }

bool Nanobot::GetShouldChange() { return m_shouldChange; }

void Nanobot::SetShouldChange(bool value) { m_shouldChange = value; }

ns3::Time Nanobot::GetTimeStep() { return m_timeStep; }

void Nanobot::SetTimeStep() { m_timeStep = Simulator::Now(); }

Vector Nanobot::GetPosition() {
    return m_node->GetObject<MobilityModel>()->GetPosition();
}

void Nanobot::SetPosition(Vector value) {
    m_node->GetObject<MobilityModel>()->SetPosition(value);
}

double Nanobot::GetDelay() { return 1; }

int Nanobot::GetTargetOrgan() { return 0; }

bool Nanobot::HasFingerprintLoaded() { return 0; }

bool Nanobot::HasTissueDetected() { return 0; }

void Nanobot::collectMessage() {}

void Nanobot::releaseFingerprintTiles() {}

void Nanobot::SetDelay(double value) {}

int Nanobot::GotDetected() { return 0; }

void Nanobot::GetsDetected() {}

double Nanobot::GetDetectionRadius() { return 0; }

void Nanobot::SetDetectionRadius(double value) {}

bool Nanobot::CanAge() { return m_canAge; }

void Nanobot::SetCanAge(bool canAge) { m_canAge = canAge; }

uint64_t Nanobot::GetAge() {
    uint64_t age = m_maxAge - m_ageCounter;
    return age;
}

bool Nanobot::IsAlive() { return m_canAge ? m_ageCounter > 0 : true; }

bool Nanobot::Age() {
    if (!m_canAge)
        return true;
    m_ageCounter--;
    return m_ageCounter > 0;
}

bool Nanobot::Age(int secCount) {
    if (m_mitosisTime > 0)
        m_mitosisCounter -= secCount;
    if (!m_canAge)
        return true;
    m_ageCounter -= secCount;
    return m_ageCounter > 0;
}

bool Nanobot::AddPossibleMitosis(ParticleType type) {
    return false;
}

bool Nanobot::WillPerformMitosis() {
    return false || (m_mitosisTime > 0 && m_mitosisCounter <= 0);
}

void Nanobot::ResetMitosis() {
    if (m_mitosisTime > 0)
        m_mitosisCounter = m_mitosisTime;
    return;
}
} // namespace ns3

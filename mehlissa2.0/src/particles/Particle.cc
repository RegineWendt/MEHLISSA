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

#include "Particle.h"
#include <cstdint>

namespace particles {

Particle::Particle() {
    particleType = BaseParticleType;
    Position m_position(0,0,0);

    m_length = 0.00001; // 100nm
    m_width = 0.00001;  // 100nm
    m_stream_nb = 0;
    m_shouldChange = false;
    m_timeStep = 0;

    m_canAge = false;
    m_maxAge = 0;
    m_ageCounter = m_maxAge;
    m_willPerformMitosis = false;
    m_mitosisTime = -1;
    m_mitosisCounter = 0;
}

Particle::~Particle() {}

bool Particle::Compare(shared_ptr<Particle> v1, shared_ptr<Particle> v2) {
    if (v1->GetParticleID() < v2->GetParticleID())
        return true;
    else
        return false;
}

int Particle::GetParticleID() { return m_nanobotID; }

void Particle::SetParticleID(int value) { m_nanobotID = value; }

double Particle::GetLength() { return m_length; }

void Particle::SetLength(double value) {
    if (value < 0)
        value = 0;
    m_length = value;
}

double Particle::GetWidth() { return m_width; }

void Particle::SetWidth(double value) {
    if (value < 0)
        value = 0;
    m_width = value;
}

void Particle::SetStream(int value) { m_stream_nb = value; }

int Particle::GetStream() { return m_stream_nb; }

bool Particle::GetShouldChange() { return m_shouldChange; }

void Particle::SetShouldChange(bool value) { m_shouldChange = value; }

uint64_t Particle::GetTimeStepInSeconds() { return m_timeStep; }

void Particle::SetTimeStep() { m_timeStep = GlobalTimer::NowInSeconds(); } // TODO

Position Particle::GetPosition() {
    return m_position;
}

void Particle::SetPosition(Position value) {
    m_position.x = value.x;
    m_position.y = value.y;
    m_position.z = value.z;
}

double Particle::GetDelay() { return 1; }

int Particle::GetTargetOrgan() { return 0; }

bool Particle::HasFingerprintLoaded() { return 0; }

bool Particle::HasTissueDetected() { return 0; }

void Particle::collectMessage() {}

void Particle::releaseFingerprintTiles() {}

void Particle::SetDelay(double value) {}

int Particle::GotDetected() { return 0; }

void Particle::GetsDetected() {}

double Particle::GetDetectionRadius() { return 0; }

void Particle::SetDetectionRadius(double value) {}

bool Particle::CanAge() { return m_canAge; }

void Particle::SetCanAge(bool canAge) { m_canAge = canAge; }

uint64_t Particle::GetAge() {
    uint64_t age = m_maxAge - m_ageCounter;
    return age;
}

bool Particle::IsAlive() { return m_canAge ? m_ageCounter > 0 : true; }

bool Particle::Age() {
    if (!m_canAge)
        return true;
    m_ageCounter--;
    return m_ageCounter > 0;
}

bool Particle::Age(int secCount) {
    if (m_mitosisTime > 0)
        m_mitosisCounter -= secCount;
    if (!m_canAge)
        return true;
    m_ageCounter -= secCount;
    return m_ageCounter > 0;
}

bool Particle::AddPossibleMitosis(ParticleType type) {
    return false;
}

bool Particle::WillPerformMitosis() {
    return false || (m_mitosisTime > 0 && m_mitosisCounter <= 0);
}

void Particle::ResetMitosis() {
    if (m_mitosisTime > 0)
        m_mitosisCounter = m_mitosisTime;
    return;
}
} // namespace particles

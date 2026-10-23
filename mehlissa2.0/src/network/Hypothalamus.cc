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

#include "Hypothalamus.h"

namespace network {

/// Constructor
Hypothalamus::Hypothalamus(int id, int baseline, double detect_prob,
                           double detect_dist, double f1, double f2)
    : Transceiver(TransceiverType::HypothalamusType, id, Position(-5, 44, 1.5),
                  nullptr, {HormoneType}, detect_prob * 100, 0) {
    m_detectionDist = detect_dist;
    m_baseline = baseline;
    m_lastOutput = baseline;
    m_p1 = 10;
    m_p2 = 15;
    m_p3 = 7.2;
    m_p4 = 0.05;
    m_p5 = 0.11;
    m_p6 = 2.9;
    m_f1 = f1;
    m_f2 = f2;
    m_offset = 0;
    m_receivedFeedback = false;
    m_initialInputTime = 10 * 60;
    Logger::LogInfo("Initialized Hypothalamus with start output " +
                    to_string(m_lastOutput) + " CRH");
}

/// Destructor
Hypothalamus::~Hypothalamus() { ; }

/**
 * Runs the detection algorithm of the transceiver on the given nanobots.
 * \returns.An updated list of nanobots.
 */
vector<shared_ptr<Particle>>
Hypothalamus::DetectParticles(vector<shared_ptr<Particle>> nanobots) {
    std::vector<std::shared_ptr<Particle>> particles;
    int numCORT = 0;
    for (shared_ptr<Particle> n : nanobots) {
        Logger::LogDebug("checking: " + to_string(n->GetParticleID()) +
                         ", type: " + to_string(n->particleType) + "");
        if (n->particleType == HormoneType &&
            Randomizer::GetRandomValue() <= m_p_detection &&
            Position::CalcDistance(n->GetPosition(), m_position) <=
                m_detectionDist) {
            std::shared_ptr<Hormone> h = std::dynamic_pointer_cast<Hormone>(n);
            if (h->GetHormoneName() == CORT)
                numCORT++;
            else
                particles.push_back(n);
        } else {
            particles.push_back(n);
        }
    }

    if (numCORT > 0)
        m_receivedFeedback = true;
    int numCRH = max(int(m_baseline * m_f2 - (numCORT - m_offset) * m_f1), 0);
    if (!m_receivedFeedback && GlobalTimer::NowInSeconds() > m_initialInputTime)
        numCRH = 0;
    for (int i = 0; i < numCRH; i++) {
        std::shared_ptr<Hormone> crh = make_shared<Hormone>(CRH);
        crh->SetPosition(m_position);
        crh->SetParticleID(IDCounter::GetNextParticleID());
        particles.push_back(std::dynamic_pointer_cast<Particle>(crh));
    }
    Logger::LogDebug("Hypothalamus released " + to_string(numCRH) + ", after " +
                     to_string(numCORT) + "");
    m_lastOutput = numCRH;
    return particles;
}

int Hypothalamus::GetBaseline() { return m_baseline; }
int Hypothalamus::GetOffset() { return m_offset; }

void Hypothalamus::SetCORTOffset(int offset) {
    m_offset =
        offset *
        0.1; // probabilty of blood reaching hypothalamus position from heart
}
} // namespace network

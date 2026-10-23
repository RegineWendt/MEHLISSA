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

#include "AdrenalGland.h"

namespace network {

/// Constructor
AdrenalGland::AdrenalGland(int id, int delay, int delay_type,
                           double detect_prob, double detect_dist, int baseline,
                           double f1)
    : Transceiver(TransceiverType::AdrenalGlandType, id, Position(3, -24, 0),
                  nullptr, {HormoneType}, detect_prob * 100, 0) {
    m_arivalProb = 0.21;
    m_detectionDist = detect_dist;
    m_lastOutput = 0;
    m_firstDelay = delay * 60;
    m_firstDelayLength = 10 * 60;
    m_secondDelay = (delay + 1) * 60;
    m_delay = m_firstDelay; // passed in [min], saved in [sec]
    m_maxDelay = m_secondDelay;
    m_delay_type = 1; // static delay for the adrenal gland
    m_baseline = baseline;
    m_base_offset = m_baseline * m_arivalProb;
    m_f1 = f1;
    m_offset = baseline;
    if (m_delay_type == 2) {
        std::vector<double> X = {0, 43200, 86400};
        std::vector<double> Y = {10.0, 30.00, 10.00};
        m_spline = new cubic_spline(X, Y, cubic_spline::natural);
    } else {
    }
    Logger::LogDebug(string("Initialized Adrenal Gland with delay ") +
                     to_string(m_delay) + " seconds");
    createAdrenalGlandStartTodos();
}

/// Destructor
AdrenalGland::~AdrenalGland() { ; }

void AdrenalGland::createAdrenalGlandStartTodos() {
    m_first = new AdrenalGlandTodo;
    m_first->prev = nullptr;
    m_first->next = nullptr;
    m_first->CORTTodo = 0;
    if (m_delay == 0)
        m_update = m_first;

    AdrenalGlandTodo *prev = m_first;
    for (int i = 1; i < m_maxDelay; i++) {
        AdrenalGlandTodo *node = new AdrenalGlandTodo;
        node->prev = prev;
        prev->next = node;
        prev = node;
        node->next = nullptr;
        node->CORTTodo = 0;
        if (i == m_delay)
            m_update = prev;
    }
    if (m_update == nullptr)
        m_update = prev;
    m_last = prev;
}

AdrenalGlandTodo *AdrenalGland::exchangeAdrenalGlandTodos(int numTodos) {
    AdrenalGlandTodo *node = new AdrenalGlandTodo;
    node->prev = m_last;
    node->next = nullptr;
    node->CORTTodo = 0;
    m_last->next = node;
    m_last = node;

    m_update = m_update->next;
    m_update->CORTTodo = m_update->CORTTodo + numTodos;

    AdrenalGlandTodo *next = m_first;
    m_first = m_first->next;
    m_first->prev = nullptr;
    return next;
}

int AdrenalGland::getDelay(int t) {
    if (m_delay_type == 1)
        return m_secondDelay;
    else if (m_delay_type == 2) {
        int day = 86400;
        if (t > day) // seconds in a day
            t = t % day;
        return (int)(m_spline->operator()(t));
    }
    return m_secondDelay;
}

void AdrenalGland::updateDelay() {
    Logger::LogDebug(string("in update at ") +
                     to_string(GlobalTimer::NowInSeconds()) + " with " +
                     to_string(m_delay) + "");
    int now = GlobalTimer::NowInSeconds();
    if (now >= m_firstDelayLength) {
        AdrenalGlandTodo *node = m_first;
        m_delay = getDelay(now);
        for (int d = 0; d < m_maxDelay; d++) {
            if (d == m_delay)
                m_update = node;
            else
                node = node->next;
        }
    }
}

/**
 * Runs the detection algorithm of the transceiver on the given nanobots.
 * \returns.An updated list of nanobots.
 */
vector<shared_ptr<Particle>>
AdrenalGland::DetectParticles(vector<shared_ptr<Particle>> nanobots) {
    updateDelay();
    std::vector<std::shared_ptr<Particle>> particles;
    int numTodo = 0;
    int numCORTTodo = 0;
    for (shared_ptr<Particle> n : nanobots) {
        if (n->particleType == HormoneType &&
            Randomizer::GetRandomValue() <= m_p_detection &&
            Position::CalcDistance(n->GetPosition(), m_position) <=
                m_detectionDist) {
            std::shared_ptr<Hormone> h = std::dynamic_pointer_cast<Hormone>(n);
            if (h->GetHormoneName() == ACTH)
                numTodo++;
            else if (h->GetHormoneName() == CORT) {
                numCORTTodo++;
                particles.push_back(n);
            } else
                particles.push_back(n);
        } else {
            particles.push_back(n);
        }
    }

    AdrenalGlandTodo *todo = exchangeAdrenalGlandTodos(numTodo);
    int additional = max(int(m_base_offset - numCORTTodo), 0);
    int numCORT = max(int(m_f1 * (todo->CORTTodo - m_offset) + additional), 0);
    for (int i = 0; i < numCORT; i++) {
        std::shared_ptr<Hormone> cort = make_shared<Hormone>(CORT);
        cort->SetPosition(m_position);
        cort->SetParticleID(IDCounter::GetNextParticleID());
        particles.push_back(std::dynamic_pointer_cast<Particle>(cort));
    }
    Logger::LogDebug(string("Adrenal Gland released ") + to_string(numCORT) +
                     ", after " + to_string(todo->CORTTodo) + "");
    m_lastOutput = numCORT;
    return particles;
}

int AdrenalGland::GetBaseline() { return m_baseline; }
int AdrenalGland::GetOffset() { return m_offset; }

void AdrenalGland::SetACTHOffset(int offset) {
    m_offset = 0; // offset * m_arivalProb; // probabilty of blood reaching
                  // adrenal gland from heart
}

} // namespace network

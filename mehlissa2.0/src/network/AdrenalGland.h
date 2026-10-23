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

#ifndef CLASS_ADRENALGLAND_
#define CLASS_ADRENALGLAND_

#include "../particles/Hormone.h"
#include "../utils/Spline.h"
#include "Transceiver.h"

using namespace particles;

namespace network {
class AdrenalGland;
/**
 * \brief  Implementation of a simplified Adrenal Gland,
 * Detects CRH and ACTH hormones, releases CORT according to input delay
 */

struct AdrenalGlandTodo {
    AdrenalGlandTodo *prev;
    AdrenalGlandTodo *next;
    int CORTTodo;
};

class AdrenalGland : public Transceiver {
private:
    double m_arivalProb;
    double m_detectionDist;
    int m_delay;
    int m_firstDelay;
    int m_firstDelayLength;
    int m_secondDelay;
    int m_maxDelay;
    int m_delay_type;
    cubic_spline *m_spline;
    int m_lastOutput;
    AdrenalGlandTodo *m_last;
    AdrenalGlandTodo *m_update;
    AdrenalGlandTodo *m_first;
    int m_baseline;
    double m_base_offset;
    double m_f1;
    int m_offset;

    void createAdrenalGlandStartTodos();
    AdrenalGlandTodo *exchangeAdrenalGlandTodos(int numTodos);
    int getDelay(int t);
    void updateDelay();

public:
    /// Constructor
    AdrenalGland(int id, int delay, int delay_type = 1,
                 double detect_prob = 0.99, double detect_dist = 0.5,
                 int baseline = 0, double f1 = 1);
    /// Destructor
    ~AdrenalGland();

    vector<shared_ptr<Particle>>
    DetectParticles(vector<shared_ptr<Particle>> nanobots);
    void SetACTHOffset(int offset);
    int GetBaseline();
    int GetOffset();
};
}; // namespace network
#endif // CLASS_ADRENALGLAND_

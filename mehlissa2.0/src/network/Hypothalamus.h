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

#ifndef CLASS_HYPOTHALAMUS_
#define CLASS_HYPOTHALAMUS_

#include "../particles/Hormone.h"
#include "Transceiver.h"

using namespace particles;

namespace network {
class Hypothalamus;
/**
 * \brief  Implementation of a simplified Hypothamalus,
 * Detects CORT hormones, releases CRH
 */

class Hypothalamus : public Transceiver {
private:
    double m_detectionDist;
    int m_baseline;
    int m_lastOutput;
    double m_p1;
    double m_p2;
    double m_p3;
    double m_p4;
    double m_p5;
    double m_p6;
    double m_f1;
    double m_f2;
    int m_offset;
    bool m_receivedFeedback;
    int m_initialInputTime;

public:
    /// Constructor
    Hypothalamus(int id, int baseline, double detect_prob = 0.99,
                 double detect_dist = 0.1, double f1 = 0.5, double f2 = 1);
    /// Destructor
    ~Hypothalamus();

    vector<shared_ptr<Particle>>
    DetectParticles(vector<shared_ptr<Particle>> nanobots);
    void SetCORTOffset(int offset);
    int GetBaseline();
    int GetOffset();
};
}; // namespace network
#endif // CLASS_HYPOTHALAMUS_

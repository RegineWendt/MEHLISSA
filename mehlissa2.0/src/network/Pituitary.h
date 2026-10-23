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

#ifndef CLASS_PITUITARY_
#define CLASS_PITUITARY_

#include "../particles/Hormone.h"
#include "../utils/Spline.h"
#include "Transceiver.h"
#include <cmath>

using namespace particles;

namespace network {
class Pituitary;
/**
 * \brief  Implementation of a simplified Pituitary Gland,
 * Detects CRH and CORT hormones, releases ACTH
 */

enum BaselineType { bsStatic, bsSinusoidal, bsDynamic };

class Pituitary : public Transceiver {
private:
    double m_arivalProb;
    double m_detectionDist;
    int m_baseline;
    int m_lastOutput;
    BaselineType m_baseline_type;
    cubic_spline *m_spline;
    double m_p1;
    double m_p2;
    double m_p3;
    double m_p4;
    double m_p5;
    double m_p6;
    double m_f1;
    double m_f2;
    double m_f3;
    int m_offset;
    double m_base_offset;
    int m_t_max;
    std::vector<double> m_Y;

    double calcP1(int t);
    double calcSinusP1(double amp, double phase, double period, double offset,
                       int t);
    double calcSplineP1(int t);

public:
    /// Constructor
    Pituitary(int id, int baseline_type = -1, double detect_prob = 0.99,
              double detect_dist = 0.1, double f1 = 1, double f2 = 1,
              double f3 = 0, string baseline_input = "");
    /// Destructor
    ~Pituitary();

    vector<shared_ptr<Particle>>
    DetectParticles(vector<shared_ptr<Particle>> nanobots);
    void SetCORTOffset(int offset);
    int GetBaseline();
    int GetOffset();
};
}; // namespace network
#endif // CLASS_PITUITARY_

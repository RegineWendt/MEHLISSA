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
 * Author: Lisa Y. Debug <debus@ccs-labs.org>
 */

#include "Pituitary.h"

namespace network {

/// Constructor
Pituitary::Pituitary(int id, int baseline_type, double detect_prob,
                     double detect_dist, double f1, double f2, double f3,
                     string baseline_input)
    : Transceiver(TransceiverType::PituitaryType, id, Position(-5, 44, -1.5),
                  nullptr, {HormoneType}, detect_prob * 100, 0) {
    m_arivalProb = 0.1;
    m_detectionDist = detect_dist;
    m_baseline_type = static_cast<BaselineType>(baseline_type);
    switch (m_baseline_type) {
    case bsStatic: {
        double y_in = std::stod(baseline_input);
        m_Y.push_back(y_in);
        m_t_max = 86400;
        break;
    }
    case bsSinusoidal: {
        std::stringstream sinput(baseline_input);
        if (sinput.good()) {
            string substr;
            getline(sinput, substr, ';');
            m_Y.push_back(std::stod(substr)); // amp
        }
        if (sinput.good()) {
            string substr;
            getline(sinput, substr, ';');
            m_Y.push_back(std::stod(substr)); // phase
        }
        if (sinput.good()) {
            string substr;
            getline(sinput, substr, ';');
            m_Y.push_back(std::stod(substr)); // period
        }
        if (sinput.good()) {
            string substr;
            getline(sinput, substr, ';');
            m_Y.push_back(std::stod(substr)); // offset
        }
    }
    case bsDynamic: {
        std::vector<double> X;
        std::stringstream sinput(baseline_input);
        int x_in, y_in;
        while (sinput.good()) {
            string substr;
            getline(sinput, substr, ';');
            Logger::LogDebug(substr + ", ");
            x_in = std::stod(substr);
            getline(sinput, substr, ';');
            Logger::LogDebug(substr + "");
            y_in = std::stod(substr);
            X.push_back(x_in);
            m_Y.push_back(y_in);
        }
        m_t_max = x_in;
        m_spline = new cubic_spline(X, m_Y, cubic_spline::natural);
        break;
    }
    default: {
        std::vector<double> X = {0,     10800, 21600, 32400, 43200,
                                 54000, 64800, 75600, 86400};
        std::vector<double> Y = {3, 4, 2, 2, 3, 8, 8, 4, 3};
        m_Y.insert(m_Y.end(), std::begin(Y), std::end(Y));
        m_spline = new cubic_spline(X, m_Y, cubic_spline::natural);
        m_t_max = 86400;
        m_baseline_type = bsDynamic;
        break;
    }
    }
    m_baseline = m_Y[0];
    m_base_offset = m_baseline * m_arivalProb;
    m_lastOutput = m_baseline;

    m_p1 = 1;
    m_p2 = 15;
    m_p3 = 7.2;
    m_p4 = 0.05;
    m_p5 = 0.11;
    m_p6 = 2.9;
    m_f1 = f1;
    m_f2 = f2;
    m_f3 = f3;
    m_offset = 0;
    Logger::LogDebug("Initialized Pituitary Gland with start output " +
                     to_string(m_lastOutput) + " ACTH");
}

/// Destructor
Pituitary::~Pituitary() { ; }

double Pituitary::calcP1(int t) {
    switch (m_baseline_type) {
    case bsStatic:
        return m_baseline;
    case bsSinusoidal:
        return calcSinusP1(m_Y[0], m_Y[1], m_Y[2], m_Y[3], t);
    case bsDynamic:
        return calcSplineP1(t);
    default:
        return m_baseline;
    }
    return -1.0;
}

double Pituitary::calcSinusP1(double amp, double phase, double period,
                              double offset, int t) {
    double a = amp < 0 ? 10 : amp;
    double ph = phase < 0 ? 0 : phase;
    double pe = period < 0 ? 86400 : period;
    double o = offset < 0 ? 10 : offset;

    if (t > 86400) // seconds in a day
        t = t % 86400;

    double period_factor = 2 * M_PI / (2 * pe);
    double p1_at_t = sin(period_factor * (t + ph)) * a + o;
    return p1_at_t;
}

double Pituitary::calcSplineP1(int t) {
    if (t > m_t_max)
        t = t % m_t_max;
    return m_spline->operator()(t);
}

/**
 * Runs the detection algorithm of the transceiver on the given nanobots.
 * \returns.An updated list of nanobots.
 */
vector<shared_ptr<Particle>>
Pituitary::DetectParticles(vector<shared_ptr<Particle>> nanobots) {
    std::vector<std::shared_ptr<Particle>> particles;
    int numCRH = 0;
    int numCORT = 0;
    int numACTHFound = 0;
    for (shared_ptr<Particle> n : nanobots) {
        Logger::LogDebug("checking: " + to_string(n->GetParticleID()) +
                         ", type: " + to_string(n->particleType) + "");
        if (n->particleType == HormoneType &&
            Randomizer::GetRandomValue() <= m_p_detection &&
            Position::CalcDistance(n->GetPosition(), m_position) <=
                m_detectionDist) {
            std::shared_ptr<Hormone> h = std::dynamic_pointer_cast<Hormone>(n);
            if (h->GetHormoneName() == CRH)
                numCRH++;
            else if (h->GetHormoneName() == CORT)
                numCORT++;
            else if (h->GetHormoneName() == ACTH) {
                numACTHFound++;
                particles.push_back(n);
            } else
                particles.push_back(n);
        } else {
            particles.push_back(n);
        }
    }

    m_p1 = calcP1(GlobalTimer::NowInSeconds());
    int additional = max(int(m_base_offset * m_p1 - numACTHFound), 0);
    int numACTH = max((int(numCRH * m_p1 * m_f1 - (numCORT - m_offset) * m_f2 +
                           additional * m_f3)),
                      0);
    for (int i = 0; i < numACTH; i++) {
        std::shared_ptr<Hormone> acth = make_shared<Hormone>(ACTH);
        acth->SetPosition(m_position);
        acth->SetParticleID(IDCounter::GetNextParticleID());
        particles.push_back(std::dynamic_pointer_cast<Particle>(acth));
    }
    Logger::LogDebug("Pituitary Gland released " + to_string(numACTH) +
                     ", after " + to_string(numCORT) + " CORT with m_p1 = " +
                     to_string(m_p1) + ", after " + to_string(numCRH) + " CRH");
    m_lastOutput = numACTH;
    return particles;
}

int Pituitary::GetBaseline() { return int(m_baseline * m_p1); }
int Pituitary::GetOffset() { return m_offset; }

void Pituitary::SetCORTOffset(int offset) {
    m_offset = 0; // offset * m_arivalProb; // probabilty of blood reaching
                  // pituitary from heart
}
} // namespace network

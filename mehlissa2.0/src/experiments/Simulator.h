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

#ifndef CLASS_SIMULATOR_
#define CLASS_SIMULATOR_

#include "../bloodcircuit/BloodVessel.h"
#include "../bloodcircuit/BloodCircuit.h"
#include "../utils/GlobalTimer.h"
#include <fstream>
#include <functional>
#include <random>
#include <stdexcept>
#include <list>
#include <memory>
#include <mutex>
#include <omp.h>

using namespace std;
using namespace bloodcircuit;
using namespace utils;

namespace experiments {
class Simulator {
private:

    shared_ptr<BloodCircuit> m_circuit;
    mutex m_currentSteps_mutex;
    mutex m_transferSteps_mutex;
    mutex m_nextSteps_mutex;
    list<shared_ptr<BloodVessel>> m_currentSteps;
    list<shared_ptr<BloodVessel>> m_transferSteps;
    list<shared_ptr<BloodVessel>> m_nextSteps;
    int m_parallelity;
    double m_timeStep; // in seconds

    void m_nextStepsSafeClear();

    void m_currentStepsSafeClear();

    void m_transferStepsSafeClear();

    void m_nextStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs);

    void m_currentStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs);

    void m_transferStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs);

    void m_transferStepsSafePushBack(shared_ptr<BloodVessel> bv);

    void m_currentStepsSafePushBack(shared_ptr<BloodVessel> bv);

    void m_nextStepsSafePushBack(shared_ptr<BloodVessel> bv);

    shared_ptr<BloodVessel> m_transferStepsSafePop();
    
    shared_ptr<BloodVessel> m_nextStepsSafePop();
    
    shared_ptr<BloodVessel> m_currentStepsSafePop();
    
    int SimulateSequential(uint64_t numberOfSeconds);
    
public:
    Simulator(int parallelity, double timeStep, shared_ptr<BloodCircuit> circuit);

    ~Simulator();

    int Simulate(uint64_t numberOfSeconds);
};
}; // namespace experiments
#endif

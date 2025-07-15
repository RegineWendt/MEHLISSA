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

#include "Simulator.h"

namespace experiments {

Simulator::Simulator(int parallelity, double timeStep, shared_ptr<BloodCircuit> circuit){
    this->m_parallelity = parallelity;
    this->m_timeStep = timeStep;
    GlobalTimer::ResetTimer();
    
    this->m_circuit = circuit;
    this->m_currentSteps = {};
    this->m_transferSteps = {};

    this->m_nextSteps = {};
    map<int, shared_ptr<BloodVessel>> cMap = this->m_circuit->GetBloodCircuit();
    for (unsigned int i = 1; i <= cMap.size(); i++)
        this->m_nextSteps.push_back(cMap[i]);
}

Simulator::~Simulator() {
    m_currentStepsSafeClear();
    m_transferStepsSafeClear();
    m_nextStepsSafeClear();
}

int Simulator::Simulate(uint64_t numberOfSeconds) {
    if (m_parallelity > 1)
        cout << "No parallel execution implemented, yet! Simulating sequentially." << endl;
    SimulateSequential(numberOfSeconds);
    return GlobalTimer::NowInSeconds();
}

int Simulator::SimulateSequential(uint64_t numberOfSeconds) {
    while(m_nextSteps.size() > 0 && GlobalTimer::NowInSeconds() <= numberOfSeconds) {
        cout << GlobalTimer::NowInSeconds() << "s" << endl;
        for (auto s : m_nextSteps)
            m_currentSteps.push_back(s);
        m_nextSteps.clear();

        clock_t start, inbetween, finish;
        start = clock();

        while(m_currentSteps.size() > 0) {
            shared_ptr<BloodVessel> bv = m_currentStepsSafePop();
            shared_ptr<BloodVessel> bv_next = bv->Step(GlobalTimer::NowInSeconds());
            // if (bv_next != nullptr)
                // m_nextSteps.push_back(bv_next);
            m_nextSteps.push_back(bv);
            if (bv->NeedsTransferStep())
                m_transferStepsSafeAppendRange({bv});
        }
        // cout << "do transfer" << endl;
        inbetween = clock();

        while (m_transferSteps.size() > 0) {
            //cout << "t";
            shared_ptr<BloodVessel> ts = m_transferStepsSafePop();
            //cout << ts->GetbloodvesselID() << endl;
            ts->PerformTransferStep();
        }

        finish = clock();
        // cout << "first part: " << (inbetween - start)/CLOCKS_PER_SEC
        //      << "    second part: " << (finish - inbetween)/CLOCKS_PER_SEC
        //      << endl;
        GlobalTimer::IncreaseTimer(m_timeStep);
    }
    return GlobalTimer::NowInSeconds();
}

void Simulator::m_nextStepsSafeClear()
{
    const std::lock_guard<std::mutex> lock(m_nextSteps_mutex);
    if (m_nextSteps.size() <= 0)
        return;
    m_nextSteps.clear();
}

void Simulator::m_currentStepsSafeClear()
{
    const std::lock_guard<std::mutex> lock(m_currentSteps_mutex);
    if (m_currentSteps.size() <= 0)
        return;
    m_currentSteps.clear();
}

void Simulator::m_transferStepsSafeClear()
{
    const std::lock_guard<std::mutex> lock(m_transferSteps_mutex);
    if (m_transferSteps.size() <= 0)
        return;
    m_transferSteps.clear();
}

void Simulator::m_nextStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs)
{
    const std::lock_guard<std::mutex> lock(m_nextSteps_mutex);
    for (auto bv : bvs)
        m_nextSteps.push_back(bv);
}

void Simulator::m_currentStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs)
{
    const std::lock_guard<std::mutex> lock(m_currentSteps_mutex);
    for (auto bv : bvs)
        m_currentSteps.push_back(bv);
}

void Simulator::m_transferStepsSafeAppendRange(list<shared_ptr<BloodVessel>> bvs)
{
    const std::lock_guard<std::mutex> lock(m_transferSteps_mutex);
    for (auto bv : bvs)
        m_transferSteps.push_back(bv);
}

void Simulator::m_transferStepsSafePushBack(shared_ptr<BloodVessel> bv)
{
    const std::lock_guard<std::mutex> lock(m_transferSteps_mutex);
    m_transferSteps.push_back(bv);
}

void Simulator::m_currentStepsSafePushBack(shared_ptr<BloodVessel> bv)
{
    const std::lock_guard<std::mutex> lock(m_currentSteps_mutex);
    m_currentSteps.push_back(bv);
}

void Simulator::m_nextStepsSafePushBack(shared_ptr<BloodVessel> bv)
{
    const std::lock_guard<std::mutex> lock(m_nextSteps_mutex);
    m_nextSteps.push_back(bv);
}

shared_ptr<BloodVessel> Simulator::m_transferStepsSafePop()
{
    const std::lock_guard<std::mutex> lock(m_transferSteps_mutex);
    if (m_transferSteps.size() <= 0)
        return nullptr;
    shared_ptr<BloodVessel> front = m_transferSteps.front();
    m_transferSteps.pop_front();
    return front;
}

shared_ptr<BloodVessel> Simulator::m_nextStepsSafePop()
{
    const std::lock_guard<std::mutex> lock(m_nextSteps_mutex);
    if (m_nextSteps.size() <= 0)
        return nullptr;
    shared_ptr<BloodVessel> front = m_nextSteps.front();
    m_nextSteps.pop_front();
    return front;
}

shared_ptr<BloodVessel> Simulator::m_currentStepsSafePop()
{
    const std::lock_guard<std::mutex> lock(m_currentSteps_mutex);
    if (m_currentSteps.size() <= 0)
        return nullptr;
    shared_ptr<BloodVessel> front = m_currentSteps.front();
    m_currentSteps.pop_front();
    return front;
}

} // namespace experiments

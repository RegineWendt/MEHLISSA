/*
 * Copyright (c) 2024 Technische Universität Berlin [DEBUS]
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
 * Author: Lisa Y. Debus <debus@ccs-labs.org>
 */

#include "Randomizer.h"

namespace ns3 {
uint32_t Randomizer::m_seed = 1;
uint32_t Randomizer::m_run = 1;
Ptr<UniformRandomVariable> Randomizer::stream_bool = nullptr;
Ptr<UniformRandomVariable> Randomizer::stream_0_1 = nullptr;

void Randomizer::InitRandomizer(bool isDeterministic) {
    m_seed = 1;
    std::random_device rnd = std::random_device();
    if (isDeterministic == true)
        m_run = 1;
    else
        m_run = rnd();
    SeedManager::SetSeed(m_seed);
    SeedManager::SetRun(m_run);
    stream_bool = CreateObject<UniformRandomVariable>();
    stream_bool->SetAttribute("Min", DoubleValue(0));
    stream_bool->SetAttribute("Max", DoubleValue(1));
    stream_0_1 = CreateObject<UniformRandomVariable>();
    stream_0_1->SetAttribute("Min", DoubleValue(0));
    stream_0_1->SetAttribute("Max", DoubleValue(1));
}

bool Randomizer::GetRandomBoolean() {
    double value = stream_bool->GetValue();
    return value >= 0.5;
}

double Randomizer::GetRandomValue() {
    double value = stream_0_1->GetValue();
    return value;
}

Ptr<UniformRandomVariable> Randomizer::GetNewRandomStream(double min,
                                                          double max) {
    Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable>();
    random->SetAttribute("Min", DoubleValue(min));
    random->SetAttribute("Max", DoubleValue(max));
    return random;
}
} // namespace ns3

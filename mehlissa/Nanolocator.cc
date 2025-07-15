/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [WENDT]
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
 */

#include "Nanolocator.h"

namespace ns3 {

// Nanolocator::Nanolocator (int target) : targetOrgan(target)
// {

// }
Nanolocator::Nanolocator() {
    m_type = NanolocatorType;
    m_hasFingerprint = true;
}

Nanolocator::~Nanolocator() {}

bool Nanolocator::HasFingerprintLoaded() { return m_hasFingerprint; }

void Nanolocator::SetTargetOrgan(int target) {
    m_targetOrgan = target;
    // std::cout << "target set" << std::endl;
}

int Nanolocator::GetTargetOrgan() { return m_targetOrgan; }

void Nanolocator::releaseFingerprintTiles() {
    m_hasFingerprint = false;
    std::cout << "Tiles released" << std::endl;
}

} // namespace ns3

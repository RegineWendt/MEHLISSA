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

#include "Nanolocator.h"

namespace particles {

NanoLocator::NanoLocator() {
    particleType = NanolocatorType;
    m_hasFingerprint = true;
}

NanoLocator::~NanoLocator() {}

bool NanoLocator::HasFingerprintLoaded() { return m_hasFingerprint; }

void NanoLocator::SetTargetOrgan(int target) {
    m_targetOrgan = target;
}

int NanoLocator::GetTargetOrgan() { return m_targetOrgan; }

void NanoLocator::releaseFingerprintTiles() {
    m_hasFingerprint = false;
    std::cout << "Tiles released" << std::endl;
}

} // namespace particles

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

#include "CarTCell.h"

namespace ns3 {

CarTCell::CarTCell() {
    m_type = CarTCellType;
    double default_nanobot_size = 0.000004702; // 47.02 nm
    double spacer_size = 0.0000004;            // 4 nm
    double CD19_size = 0.0000012;              // 12 nm
    double CD45_size = 0.0000025;              // 22 - 28 nm
    // Jurkat
    m_length = 0.0011500; // 11.5 mum
    m_width = 0.0011500;  // 11.5 mum
    // HL60
    // m_length = 0.0012400;                        // 12.4 mum
    // m_width = 0.0012400;                         // 12.4 mum
    // must add spacer and CAR size to used T cell size
    double CAR_size = spacer_size + CD45_size;
    m_length += CAR_size;
    m_width += CAR_size;
    m_delay =
        1; // default_nanobot_size / m_length;  // calculated based on size
           //  in relation to Nanobots

    m_cancerFratricideP = 6e-11;
    m_tFratricideP = 6e-11;
    m_carTFratricideP = 6e-11;
    m_cancerMitosisP = 1e-11;
    m_tMitosisP = 1e-11;
    m_carTMitosisP = 1e-11;
    //m_cancerFratricideP = 0.15;
    //m_tFratricideP = 0.15;
    //m_carTFratricideP = 0.20;
    //m_cancerMitosisP = 0.30;
    //m_tMitosisP = 0.30;
    //m_carTMitosisP = 0.15;
    m_isActive = false;
    m_detectedCancerCells = 0;
    m_killedCancerCells = 0;
    m_detectedTCells = 0;
    m_killedTCells = 0;
    m_detectedCarTCells = 0;
    m_killedCarTCells = 0;

    m_canAge = true;
    m_maxAge = 1814400;
    m_ageCounter = m_maxAge;
    m_willPerformMitosis = false;
    m_mitosisTime = -1;
    m_mitosisCounter = 0;
}

CarTCell::~CarTCell() {}

double CarTCell::GetDelay() { return m_delay; }

double CarTCell::GetCancerFratricideP() { return m_cancerFratricideP; }

double CarTCell::GetTFratricideP() { return m_tFratricideP; }

double CarTCell::GetCarTFratricideP() { return m_carTFratricideP; }

bool CarTCell::IsActive() { return IsAlive() && m_isActive; }

bool CarTCell::HasDetectedTCells() { return m_detectedTCells > 0; }

int CarTCell::GetNumberOfDetectedTCells() { return m_detectedTCells; }

int CarTCell::DetectTCells(int numberCells) {
    m_detectedTCells += numberCells;
    return m_detectedTCells > 0;
}

bool CarTCell::HasKilledTCells() { return m_killedTCells > 0; }

int CarTCell::GetNumberOfKilledTCells() { return m_killedTCells; }

int CarTCell::KillSomeTCells(int numberCells) {
    int killedCells = 0;
    for (int i = 0; i < numberCells; i++)
        killedCells += int(KillTCell());
    return killedCells;
}

bool CarTCell::KillTCell() {
    if (!IsAlive())
        return false;
    bool killIt = Randomizer::GetRandomValue() < m_tFratricideP;
    if (killIt == true) {
        m_killedTCells += 1;
        m_isActive = true;
    }
    return killIt;
}

bool CarTCell::HasDetectedCancerCells() { return m_detectedCancerCells > 0; }

int CarTCell::GetNumberOfDetectedCancerCells() { return m_detectedCancerCells; }

int CarTCell::DetectCancerCells(int numberCells) {
    m_detectedCancerCells += numberCells;
    return m_detectedCancerCells > 0;
}

bool CarTCell::HasKilledCancerCells() { return m_killedCancerCells > 0; }

int CarTCell::GetNumberOfKilledCancerCells() { return m_killedCancerCells; }

int CarTCell::KillSomeCancerCells(int numberCells) {
    int killedCells = 0;
    for (int i = 0; i < numberCells; i++)
        killedCells += int(KillCancerCell());
    return killedCells;
}

bool CarTCell::KillCancerCell() {
    if (!IsAlive())
        return false;
    bool killIt = Randomizer::GetRandomValue() < m_cancerFratricideP;
    if (killIt == true) {
        m_killedCancerCells += 1;
        m_isActive = true;
    }
    return killIt;
}

bool CarTCell::HasDetectedCarTCells() { return m_detectedCarTCells > 0; }

int CarTCell::GetNumberOfDetectedCarTCells() { return m_detectedCarTCells; }

int CarTCell::DetectCarTCells(int numberCells) {
    m_detectedCarTCells += numberCells;
    return m_detectedCarTCells > 0;
}

bool CarTCell::HasKilledCarTCells() { return m_killedCarTCells > 0; }

int CarTCell::GetNumberOfKilledCarTCells() { return m_killedCarTCells; }

int CarTCell::KillSomeCarTCells(int numberCells) {
    int killedCells = 0;
    for (int i = 0; i < numberCells; i++)
        killedCells += int(KillCarTCell());
    return killedCells;
}

bool CarTCell::KillCarTCell() {
    if (!IsAlive())
        return false;
    bool killIt = Randomizer::GetRandomValue() < m_carTFratricideP;
    if (killIt == true) {
        m_killedCarTCells += 1;
        m_isActive = true;
    }
    return killIt;
}

bool CarTCell::AddPossibleMitosis(ParticleType type) {
    double mitosisP = 0;
    switch(type) {
        case CarTCellType:
            mitosisP = m_carTMitosisP;
            break;
        case TCellType:
            mitosisP = m_tMitosisP;
            break;
        case CancerCellType:
            mitosisP = m_cancerMitosisP;
            break;
        default:
            break;
    } 
    m_willPerformMitosis = m_willPerformMitosis ||
        Randomizer::GetRandomValue() < mitosisP;
    return m_willPerformMitosis;
}

bool CarTCell::WillPerformMitosis() {
    return m_willPerformMitosis;
}

void CarTCell::ResetMitosis() {
    m_willPerformMitosis = false;
}
} // namespace ns3

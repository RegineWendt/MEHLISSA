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

#ifndef CLASS_CARTCELL_
#define CLASS_CARTCELL_

#include "Nanobot.h"
#include "Randomizer.h"
#include <iostream>
#include <random>

namespace ns3 {
class CarTCell;
/**
 * \brief CarTCell is a mobile object that detects CancerCells.
 */

class CarTCell : public Nanobot {
private:
    double m_delay;             // factor for changed traveling speed
    double m_cancerFratricideP; // probability in [0,1] that describes how
                                // likely the interaction with a CancerCell will
                                // lead to its destruction
    double m_tFratricideP;      // probability in [0,1] that describes how
                                // likely the interaction with a TCell will
                                // lead to its destruction
    double m_carTFratricideP;   // probability in [0,1] that describes how
                                // likely the interaction with a CarTCell will
                                // lead to its destruction
    double m_cancerMitosisP;    // probability in [0,1] that describes how
                                // likely the interaction with a CancerCell will
                                // lead to the mitosis of the CarTCell
    double m_tMitosisP;         // probability in [0,1] that describes how
                                // likely the interaction with a TCell will
                                // lead to the mitosis of the CarTCell
    double m_carTMitosisP;      // probability in [0,1] that describes how
                                // likely the interaction with a CarTCell will
                                // lead to the mitosis of the CarTCell
    bool m_isActive;            // true if CarTCell is active
    int m_detectedCancerCells;  // number of detected CancerCells
    int m_killedCancerCells;    // number of killed CancerCells
    int m_detectedTCells;       // number of detected TCells
    int m_killedTCells;         // number of killed TCells
    int m_detectedCarTCells;    // number of detected CarTCells
    int m_killedCarTCells;      // number of killed CarTCells

public:
    CarTCell();
    ~CarTCell();

    double GetDelay();

    double GetCancerFratricideP();

    double GetTFratricideP();

    double GetCarTFratricideP();

    bool IsActive();

    bool HasDetectedTCells();

    int GetNumberOfDetectedTCells();

    int DetectTCells(int numberCells);

    bool HasKilledTCells();

    int GetNumberOfKilledTCells();

    int KillSomeTCells(int numberCells);

    bool KillTCell();

    bool HasDetectedCancerCells();

    int GetNumberOfDetectedCancerCells();

    int DetectCancerCells(int numberCells);

    bool HasKilledCancerCells();

    int GetNumberOfKilledCancerCells();

    int KillSomeCancerCells(int numberCells);

    bool KillCancerCell();

    bool HasDetectedCarTCells();

    int GetNumberOfDetectedCarTCells();

    int DetectCarTCells(int numberCells);

    bool HasKilledCarTCells();

    int GetNumberOfKilledCarTCells();

    int KillSomeCarTCells(int numberCells);

    bool KillCarTCell();

    bool AddPossibleMitosis(ParticleType type) override;

    bool WillPerformMitosis() override;

    void ResetMitosis() override;

    struct CarTCellInjection {
        int m_injectionTime;
        int m_injectionVessel;
        int m_injectionNumber;
    };
};
}; // namespace ns3
#endif

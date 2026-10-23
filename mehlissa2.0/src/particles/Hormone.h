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

#ifndef CLASS_HORMONE_
#define CLASS_HORMONE_

#include "Nanoparticle.h"
#include <iostream>
#include <random>

namespace particles {

class Hormone;
/**
 * \brief A hormone is a passive particle that moves through the HCS.
 * Organs react to the detection of specfic hormones.
 */

enum HormoneName { ACTH, CRH, CORT };

class Hormone : public Nanoparticle {
private:
    HormoneName m_hormoneName;

public:
    Hormone(HormoneName name);
    ~Hormone();

    HormoneName GetHormoneName();
    int GetSubType() override;
};
}; // namespace particles
#endif

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

#include "Position.h"

namespace utils {

Position::Position() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

Position::Position(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Position::Position(std::shared_ptr<Position> p) {
    this->x = p->x;
    this->y = p->y;
    this->z = p->z;
}

Position::~Position() {}

double Position::CalcDistance(Position a, Position b) {
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - b.z, 2));
}
} // namespace utils

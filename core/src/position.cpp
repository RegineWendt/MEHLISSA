// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/position.hpp>

#include <cmath>

namespace mehlissa::core {

Length distance(const Position3D& first, const Position3D& second) noexcept {
    const auto delta_x = in_meters(second.x - first.x);
    const auto delta_y = in_meters(second.y - first.y);
    const auto delta_z = in_meters(second.z - first.z);
    return meters(std::hypot(delta_x, delta_y, delta_z));
}

} // namespace mehlissa::core

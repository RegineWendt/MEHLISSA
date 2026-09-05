// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/position.hpp>

#include <cmath>
#include <limits>
#include <utility>

namespace mehlissa::core {

Length distance(const Position3D& first, const Position3D& second) noexcept {
    // Keep the operation order independent of the standard library's
    // three-argument hypot implementation so serialized derived geometry is
    // byte-stable across supported toolchains.
    auto largest = std::abs(in_meters(second.x - first.x));
    auto second_largest = std::abs(in_meters(second.y - first.y));
    auto third_largest = std::abs(in_meters(second.z - first.z));

    if (second_largest > largest) {
        std::swap(largest, second_largest);
    }
    if (third_largest > largest) {
        std::swap(largest, third_largest);
    }

    if (largest == std::numeric_limits<double>::infinity()) {
        return meters(largest);
    }
    if (largest * std::numeric_limits<double>::epsilon() >= second_largest &&
        largest * std::numeric_limits<double>::epsilon() >= third_largest) {
        return meters(largest);
    }

    const auto second_ratio = second_largest / largest;
    const auto third_ratio = third_largest / largest;
    return meters(largest *
                  std::sqrt(1.0 + (second_ratio * second_ratio) + (third_ratio * third_ratio)));
}

} // namespace mehlissa::core

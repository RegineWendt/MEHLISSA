// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_POSITION_HPP
#define MEHLISSA_CORE_POSITION_HPP

#include <mehlissa/core/quantity.hpp>

namespace mehlissa::core {

struct Position3D final {
    Length x{};
    Length y{};
    Length z{};
};

[[nodiscard]] Length distance(const Position3D& first, const Position3D& second) noexcept;

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_POSITION_HPP

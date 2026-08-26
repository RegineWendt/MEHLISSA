// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_RANDOM_STREAM_HPP
#define MEHLISSA_CORE_RANDOM_STREAM_HPP

#include <cstdint>
#include <random>
#include <string_view>

namespace mehlissa::core {

class RandomStream final {
  public:
    RandomStream(std::uint64_t experiment_seed, std::string_view stream_name);

    [[nodiscard]] std::uint64_t next_u64();
    [[nodiscard]] std::uint64_t derived_seed() const noexcept;
    [[nodiscard]] std::uint64_t draw_count() const noexcept;

  private:
    std::uint64_t derived_seed_{};
    std::uint64_t draw_count_{};
    std::mt19937_64 engine_;
};

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_RANDOM_STREAM_HPP

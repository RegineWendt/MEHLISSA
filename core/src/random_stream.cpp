// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/random_stream.hpp>

#include <mehlissa/core/error.hpp>

#include <limits>

namespace mehlissa::core {
namespace {

constexpr std::uint64_t fnv_offset_basis = 14'695'981'039'346'656'037ULL;
constexpr std::uint64_t fnv_prime = 1'099'511'628'211ULL;

[[nodiscard]] std::uint64_t stable_name_hash(std::string_view name) noexcept {
    auto hash = fnv_offset_basis;
    for (const char character : name) {
        hash ^= static_cast<unsigned char>(character);
        hash *= fnv_prime;
    }
    return hash;
}

[[nodiscard]] std::uint64_t splitmix64(std::uint64_t value) noexcept {
    value += 0x9E3779B97F4A7C15ULL;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}

} // namespace

RandomStream::RandomStream(const std::uint64_t experiment_seed, const std::string_view stream_name)
    : derived_seed_(splitmix64(experiment_seed ^ stable_name_hash(stream_name))),
      engine_(derived_seed_) {
    if (stream_name.empty()) {
        throw MehlissaError{ErrorCode::invariant_violated,
                            "A random stream must have a non-empty name"};
    }
}

std::uint64_t RandomStream::next_u64() {
    if (draw_count_ == std::numeric_limits<std::uint64_t>::max()) {
        throw MehlissaError{ErrorCode::numeric_overflow, "Random-stream draw counter overflow"};
    }
    const auto value = engine_();
    ++draw_count_;
    return value;
}

std::uint64_t RandomStream::derived_seed() const noexcept { return derived_seed_; }

std::uint64_t RandomStream::draw_count() const noexcept { return draw_count_; }

} // namespace mehlissa::core

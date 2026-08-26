// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

namespace {

constexpr std::uint64_t master_seed = 0x6a09e667f3bcc909ULL;
constexpr std::uint64_t fnv_offset_basis = 14695981039346656037ULL;
constexpr std::uint64_t fnv_prime = 1099511628211ULL;

void hash_u64(std::uint64_t& hash, const std::uint64_t value) noexcept {
    for (int shift = 56; shift >= 0; shift -= 8) {
        hash ^= (value >> shift) & 0xffULL;
        hash *= fnv_prime;
    }
}

struct ReferenceState final {
    std::uint64_t circulation_hash{fnv_offset_basis};
    std::uint64_t sensor_hash{fnv_offset_basis};
    std::uint64_t combined_hash{fnv_offset_basis};
};

class ReferenceComponent final : public mehlissa::core::SimulationComponent {
  public:
    explicit ReferenceComponent(ReferenceState& state) : state_{state} {}

    [[nodiscard]] std::string_view name() const noexcept override { return "reference-kernel"; }

    void initialize(mehlissa::core::SimulationContext& context) override {
        static_cast<void>(context.random_stream("circulation"));
        static_cast<void>(context.random_stream("sensor-noise"));
    }

    void advance(mehlissa::core::SimulationContext& context,
                 mehlissa::core::SimulationClock::Duration) override {
        auto& circulation = context.random_stream("circulation");
        auto& sensor_noise = context.random_stream("sensor-noise");
        for (int draw = 0; draw < 2; ++draw) {
            const auto value = circulation.next_u64();
            hash_u64(state_.circulation_hash, value);
            hash_u64(state_.combined_hash, value);
        }
        const auto value = sensor_noise.next_u64();
        hash_u64(state_.sensor_hash, value);
        hash_u64(state_.combined_hash, value);
    }

    void finalize(mehlissa::core::SimulationContext&) noexcept override {}

  private:
    ReferenceState& state_;
};

[[nodiscard]] std::string hexadecimal(const std::uint64_t value) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(16) << value;
    return stream.str();
}

[[nodiscard]] std::string make_reference_document() {
    using namespace std::chrono_literals;

    ReferenceState state;
    mehlissa::core::ComponentHost simulation{master_seed};
    simulation.add(std::make_unique<ReferenceComponent>(state));
    simulation.initialize();

    for (int step = 0; step < 16; ++step) {
        simulation.advance(62'500'000ns);
    }
    simulation.finalize();
    const auto streams = simulation.context().random_stream_states();

    std::ostringstream document;
    document << "{\n"
             << "  \"format_version\": \"1.0.0\",\n"
             << "  \"reference\": \"core-rng-clock-v1\",\n"
             << "  \"master_seed\": " << master_seed << ",\n"
             << "  \"simulation_time_ns\": " << simulation.context().clock().now().count() << ",\n"
             << "  \"streams\": [\n"
             << "    {\"name\": \"circulation\", \"draw_count\": " << streams.at(0).draw_count
             << ", \"fnv1a64\": \"" << hexadecimal(state.circulation_hash) << "\"},\n"
             << "    {\"name\": \"sensor-noise\", \"draw_count\": " << streams.at(1).draw_count
             << ", \"fnv1a64\": \"" << hexadecimal(state.sensor_hash) << "\"}\n"
             << "  ],\n"
             << "  \"combined_fnv1a64\": \"" << hexadecimal(state.combined_hash) << "\"\n"
             << "}\n";
    return document.str();
}

} // namespace

int main(const int argc, const char* const argv[]) noexcept {
    try {
        if (argc != 2) {
            return 2;
        }

        std::ofstream output{std::filesystem::path{argv[1]}, std::ios::binary | std::ios::trunc};
        if (!output) {
            return 3;
        }
        output << make_reference_document();
        return output ? 0 : 4;
    } catch (...) {
        return 5;
    }
}

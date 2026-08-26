// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

enum class FailurePoint : std::uint8_t { none, initialize, advance };

class FinalizationLog final {
  public:
    void record(const int token) noexcept {
        if (size_ < entries_.size()) {
            entries_[size_] = token;
            ++size_;
        }
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] int operator[](const std::size_t index) const noexcept { return entries_[index]; }

  private:
    std::array<int, 8> entries_{};
    std::size_t size_{};
};

class RecordingComponent final : public mehlissa::core::SimulationComponent {
  public:
    RecordingComponent(std::string name, std::vector<std::string>& events,
                       FinalizationLog& finalizations, const int finalization_token,
                       const FailurePoint failure = FailurePoint::none)
        : name_{std::move(name)}, events_{events}, finalizations_{finalizations},
          finalization_token_{finalization_token}, failure_{failure} {}

    [[nodiscard]] std::string_view name() const noexcept override { return name_; }

    void initialize(mehlissa::core::SimulationContext&) override {
        events_.push_back("initialize:" + name_);
        if (failure_ == FailurePoint::initialize) {
            throw std::runtime_error{"Initialization failed"};
        }
    }

    void advance(mehlissa::core::SimulationContext& context,
                 const mehlissa::core::SimulationClock::Duration) override {
        events_.push_back("advance:" + name_ + "@" + std::to_string(context.clock().now().count()));
        if (failure_ == FailurePoint::advance) {
            throw std::runtime_error{"Advance failed"};
        }
    }

    void finalize(mehlissa::core::SimulationContext&) noexcept override {
        finalizations_.record(finalization_token_);
    }

  private:
    std::string name_;
    std::vector<std::string>& events_;
    FinalizationLog& finalizations_;
    int finalization_token_{};
    FailurePoint failure_;
};

[[nodiscard]] std::unique_ptr<RecordingComponent>
component(std::string name, std::vector<std::string>& events, FinalizationLog& finalizations,
          const int finalization_token, const FailurePoint failure = FailurePoint::none) {
    return std::make_unique<RecordingComponent>(std::move(name), events, finalizations,
                                                finalization_token, failure);
}

} // namespace

TEST_CASE("Components follow one forward and reverse lifecycle", "[core][component]") {
    using namespace std::chrono_literals;

    std::vector<std::string> events;
    FinalizationLog finalizations;
    mehlissa::core::ComponentHost host{std::uint64_t{123}};
    host.add(component("first", events, finalizations, 1));
    host.add(component("second", events, finalizations, 2));

    host.initialize();
    host.advance(5ns);
    REQUIRE(host.context().clock().now() == 5ns);
    host.finalize();
    host.finalize();

    const std::vector<std::string> expected{
        "initialize:first",
        "initialize:second",
        "advance:first@0",
        "advance:second@0",
    };
    REQUIRE(events == expected);
    REQUIRE(finalizations.size() == 2);
    REQUIRE(finalizations[0] == 2);
    REQUIRE(finalizations[1] == 1);
    REQUIRE(host.state() == mehlissa::core::ComponentHost::State::finalized);
    REQUIRE(host.component_count() == 2);
}

TEST_CASE("Initialization failure finalizes prior components in reverse", "[core][component]") {
    std::vector<std::string> events;
    FinalizationLog finalizations;
    mehlissa::core::ComponentHost host{std::uint64_t{1}};
    host.add(component("ready", events, finalizations, 1));
    host.add(component("failing", events, finalizations, 2, FailurePoint::initialize));

    REQUIRE_THROWS_WITH(host.initialize(), "Initialization failed");

    const std::vector<std::string> expected{
        "initialize:ready",
        "initialize:failing",
    };
    REQUIRE(events == expected);
    REQUIRE(finalizations.size() == 1);
    REQUIRE(finalizations[0] == 1);
    REQUIRE(host.state() == mehlissa::core::ComponentHost::State::finalized);
}

TEST_CASE("Failed advance does not advance the shared clock", "[core][component]") {
    using namespace std::chrono_literals;

    std::vector<std::string> events;
    FinalizationLog finalizations;
    mehlissa::core::ComponentHost host{std::uint64_t{1}};
    host.add(component("failing", events, finalizations, 1, FailurePoint::advance));
    host.initialize();

    REQUIRE_THROWS_WITH(host.advance(1s), "Advance failed");
    REQUIRE(host.context().clock().now() == 0ns);
}

TEST_CASE("Component ownership and lifecycle transitions are guarded", "[core][component]") {
    std::vector<std::string> events;
    FinalizationLog finalizations;
    mehlissa::core::ComponentHost host{std::uint64_t{1}};

    REQUIRE_THROWS_AS(host.add(nullptr), std::invalid_argument);
    REQUIRE_THROWS_AS(host.add(component("", events, finalizations, 0)), std::invalid_argument);
    host.add(component("unique", events, finalizations, 1));
    REQUIRE_THROWS_WITH(host.add(component("unique", events, finalizations, 2)),
                        "Duplicate simulation component name: unique");
    REQUIRE_THROWS_AS(host.advance(mehlissa::core::SimulationClock::Duration{1}), std::logic_error);

    host.initialize();
    REQUIRE_THROWS_AS(host.add(component("late", events, finalizations, 3)), std::logic_error);
    REQUIRE_THROWS_AS(host.initialize(), std::logic_error);
}

TEST_CASE("The host destructor finalizes initialized components exactly once",
          "[core][component]") {
    std::vector<std::string> events;
    FinalizationLog finalizations;
    {
        mehlissa::core::ComponentHost host{std::uint64_t{1}};
        host.add(component("owned", events, finalizations, 1));
        host.initialize();
    }

    const std::vector<std::string> expected{"initialize:owned"};
    REQUIRE(events == expected);
    REQUIRE(finalizations.size() == 1);
    REQUIRE(finalizations[0] == 1);
}

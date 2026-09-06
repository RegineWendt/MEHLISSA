// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "mehlissa/scenarios/fdg_pet/fdg_pet_model.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>

namespace fdg = mehlissa::scenarios::fdg_pet;
namespace core = mehlissa::core;

TEST_CASE("FDG/PET candidate produces bounded frame averages") {
    const std::vector<fdg::Frame> frames{{core::seconds(0), core::seconds(10)},
                                         {core::seconds(10), core::seconds(20)},
                                         {core::seconds(30), core::seconds(30)},
                                         {core::minutes(1), core::minutes(4)}};
    const auto rows = fdg::simulate(fdg::source_disjoint_reference_candidate(), frames);
    REQUIRE(rows.size() == frames.size());
    for (const auto& row : rows) {
        REQUIRE(std::isfinite(fdg::in_becquerels_per_milliliter(row.aortic_input)));
        REQUIRE(fdg::in_becquerels_per_milliliter(row.aortic_input) >= 0.0);
        REQUIRE(fdg::in_becquerels_per_milliliter(row.lung) >= 0.0);
        REQUIRE(fdg::in_becquerels_per_milliliter(row.liver) >= 0.0);
        REQUIRE(fdg::in_becquerels_per_milliliter(row.kidney) >= 0.0);
        REQUIRE(fdg::in_becquerels(row.urinary_bladder) >= 0.0);
    }
}

TEST_CASE("FDG/PET candidate is deterministic and convergent") {
    const std::vector<fdg::Frame> frames{{core::minutes(1), core::minutes(4)},
                                         {core::minutes(5), core::minutes(5)}};
    const auto first =
        fdg::simulate(fdg::source_disjoint_reference_candidate(), frames, core::seconds(0.2));
    const auto replay =
        fdg::simulate(fdg::source_disjoint_reference_candidate(), frames, core::seconds(0.2));
    const auto fine =
        fdg::simulate(fdg::source_disjoint_reference_candidate(), frames, core::seconds(0.1));
    REQUIRE(fdg::in_becquerels_per_milliliter(first.back().liver) ==
            fdg::in_becquerels_per_milliliter(replay.back().liver));
    const double reference = fdg::in_becquerels_per_milliliter(fine.back().kidney);
    REQUIRE(std::abs(fdg::in_becquerels_per_milliliter(first.back().kidney) - reference) /
                reference <
            1.0e-4);
}

TEST_CASE("decay convention is explicit and applied once") {
    const std::vector<fdg::Frame> frames{{core::minutes(60), core::minutes(1)}};
    auto corrected = fdg::source_disjoint_reference_candidate();
    auto uncorrected = corrected;
    uncorrected.decay_reference = fdg::DecayReference::scan_time_uncorrected;
    const auto a = fdg::simulate(corrected, frames, core::seconds(0.5));
    const auto b = fdg::simulate(uncorrected, frames, core::seconds(0.5));
    REQUIRE(fdg::in_becquerels_per_milliliter(b[0].liver) <
            fdg::in_becquerels_per_milliliter(a[0].liver));
}

TEST_CASE("invalid frames and administration fail closed") {
    const std::vector<fdg::Frame> overlapping{{core::seconds(0), core::seconds(10)},
                                              {core::seconds(5), core::seconds(10)}};
    REQUIRE_THROWS_AS(fdg::simulate(fdg::source_disjoint_reference_candidate(), overlapping),
                      std::invalid_argument);
    auto invalid = fdg::source_disjoint_reference_candidate();
    invalid.administration.injected_activity = fdg::becquerels(0.0);
    REQUIRE_THROWS_AS(fdg::simulate(invalid, {{core::seconds(0), core::seconds(1)}}),
                      std::invalid_argument);
}

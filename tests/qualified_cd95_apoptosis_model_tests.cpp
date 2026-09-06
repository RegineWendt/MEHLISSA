// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/cell/qualified_cd95_apoptosis_model.hpp>

#include <mehlissa/core/error.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace cell = mehlissa::models::cell;

[[nodiscard]] cell::QualifiedCd95Request request(const cell::KallenbergerCase source_case,
                                                 const double step = 0.01) {
    return {"bcq-cross-engine",
            source_case,
            {"CD95L", {16.6}, std::string{cell::unresolved_model_native_unit}},
            {240.0},
            {0.25},
            {step}};
}

[[nodiscard]] std::vector<std::array<double, cell::kallenberger_species_count + 1>>
load_copasi_reference(const std::filesystem::path& path) {
    std::ifstream stream{path};
    REQUIRE(stream.good());
    std::string line;
    REQUIRE(static_cast<bool>(std::getline(stream, line)));
    std::vector<std::array<double, cell::kallenberger_species_count + 1>> rows;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        std::array<double, cell::kallenberger_species_count + 1> row{};
        std::stringstream values{line};
        std::string token;
        for (auto& value : row) {
            REQUIRE(static_cast<bool>(std::getline(values, token, ',')));
            value = std::stod(token);
        }
        rows.push_back(row);
    }
    return rows;
}

[[nodiscard]] double
invariant_residual(const cell::KallenbergerState& state,
                   const std::initializer_list<cell::KallenbergerSpecies> members,
                   const double expected) {
    double actual = 0.0;
    for (const auto member : members) {
        actual += state.at(member).value;
    }
    return std::abs(actual - expected);
}

} // namespace

TEST_CASE("The qualified CD95 adapter locks source identity and unresolved units",
          "[m5][bcq][adapter][identity]") {
    const auto& cd95 = cell::kallenberger_minimal_definition(cell::KallenbergerCase::cd95_hela);
    const auto& wild =
        cell::kallenberger_minimal_definition(cell::KallenbergerCase::wild_type_hela);
    CHECK(cd95.source.accession == "BIOMD0000000523");
    CHECK(wild.source.accession == "BIOMD0000000524");
    CHECK(cd95.source.licence == "CC0-1.0");
    CHECK(wild.source.licence == "CC0-1.0");
    CHECK(cd95.initial_state.at(cell::KallenbergerSpecies::cd95).value == 116.0);
    CHECK(wild.initial_state.at(cell::KallenbergerSpecies::cd95).value == 12.0);

    auto invalid_stimulus = request(cell::KallenbergerCase::cd95_hela);
    invalid_stimulus.stimulus.initial_value.value = 17.0;
    CHECK_THROWS_AS(cell::QualifiedCd95ApoptosisAdapter{}.evaluate(invalid_stimulus),
                    mehlissa::core::MehlissaError);
    auto invented_units = request(cell::KallenbergerCase::cd95_hela);
    invented_units.stimulus.unit_semantics = "nanomolar";
    CHECK_THROWS_AS(cell::QualifiedCd95ApoptosisAdapter{}.evaluate(invented_units),
                    mehlissa::core::MehlissaError);
}

TEST_CASE("The qualified CD95 mechanism preserves every source-derived invariant",
          "[m5][bcq][adapter][conservation]") {
    const cell::QualifiedCd95ApoptosisAdapter adapter;
    for (const auto [source_case, fadd, p55, bid, nes, er] :
         {std::tuple{cell::KallenbergerCase::cd95_hela, 93.0, 155.0, 236.0, 973.0, 5178.0},
          std::tuple{cell::KallenbergerCase::wild_type_hela, 90.0, 127.0, 224.0, 1909.0, 3316.0}}) {
        const auto result = adapter.evaluate(request(source_case));
        REQUIRE(result.samples.size() == 961);
        for (const auto& sample : result.samples) {
            CHECK(invariant_residual(
                      sample.state,
                      {cell::KallenbergerSpecies::fadd, cell::KallenbergerSpecies::disc,
                       cell::KallenbergerSpecies::discp55, cell::KallenbergerSpecies::p30,
                       cell::KallenbergerSpecies::p43},
                      fadd) <= 1.0e-8);
            CHECK(invariant_residual(
                      sample.state,
                      {cell::KallenbergerSpecies::p55free, cell::KallenbergerSpecies::discp55,
                       cell::KallenbergerSpecies::p30, cell::KallenbergerSpecies::p43,
                       cell::KallenbergerSpecies::p18, cell::KallenbergerSpecies::p18inactive},
                      p55) <= 1.0e-8);
            CHECK(invariant_residual(
                      sample.state,
                      {cell::KallenbergerSpecies::bid, cell::KallenbergerSpecies::tbid},
                      bid) <= 1.0e-8);
            CHECK(invariant_residual(
                      sample.state,
                      {cell::KallenbergerSpecies::prnes_mcherry, cell::KallenbergerSpecies::prnes},
                      nes) <= 1.0e-8);
            CHECK(invariant_residual(
                      sample.state,
                      {cell::KallenbergerSpecies::prer_mgfp, cell::KallenbergerSpecies::prer},
                      er) <= 1.0e-8);
        }
    }
}

TEST_CASE("The MEHLISSA CD95 trajectories agree with the independent COPASI archive",
          "[m5][bcq][cross-engine]") {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    const auto archive =
        root / "results/bcq1/kallenberger-minimal/20260906T071228Z-5dd09984d838/trajectories";
    const cell::QualifiedCd95ApoptosisAdapter adapter;
    for (const auto [source_case, file] :
         {std::pair{cell::KallenbergerCase::cd95_hela, std::string{"BIOMD0000000523-primary.csv"}},
          std::pair{cell::KallenbergerCase::wild_type_hela,
                    std::string{"BIOMD0000000524-primary.csv"}}}) {
        const auto expected = load_copasi_reference(archive / file);
        const auto actual = adapter.evaluate(request(source_case));
        REQUIRE(actual.samples.size() == expected.size());
        for (std::size_t row = 0; row < expected.size(); ++row) {
            CHECK(actual.samples[row].model_time.value == expected[row][0]);
            for (std::size_t species = 0; species < cell::kallenberger_species_count; ++species) {
                const auto reference = expected[row][species + 1];
                const auto error = std::abs(actual.samples[row].state.values[species] - reference);
                CHECK(error <= 1.0e-7 + 1.0e-7 * std::max(1.0, std::abs(reference)));
            }
        }
    }
}

TEST_CASE("The qualified CD95 RK4 path converges when its internal step is halved",
          "[m5][bcq][convergence]") {
    const cell::QualifiedCd95ApoptosisAdapter adapter;
    for (const auto source_case :
         {cell::KallenbergerCase::cd95_hela, cell::KallenbergerCase::wild_type_hela}) {
        const auto primary = adapter.evaluate(request(source_case, 0.01));
        const auto tightened = adapter.evaluate(request(source_case, 0.005));
        REQUIRE(primary.samples.size() == tightened.samples.size());
        for (std::size_t row = 0; row < primary.samples.size(); ++row) {
            for (std::size_t species = 0; species < cell::kallenberger_species_count; ++species) {
                const auto reference = tightened.samples[row].state.values[species];
                const auto error = std::abs(primary.samples[row].state.values[species] - reference);
                CHECK(error <= 1.0e-9 + 1.0e-8 * std::max(1.0, std::abs(reference)));
            }
        }
    }
}

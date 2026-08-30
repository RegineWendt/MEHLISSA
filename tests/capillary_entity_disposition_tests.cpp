// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/capillary_entity_disposition_profile.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>
#include <mehlissa/models/coupling/entity_disposition.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::capillary::CapillaryBed;
using mehlissa::models::capillary::CapillaryBedProfiles;
using mehlissa::models::coupling::EntityDispositionKind;
using mehlissa::models::coupling::EntityDispositionTransfer;

[[nodiscard]] mehlissa::models::capillary::CapillaryBedConfig load_capillary_config() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_bed_definition(
               {root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
                root / "data/schemas/capillary-bed-definition/2.0.0.schema.json"})
        .model;
}

[[nodiscard]] mehlissa::models::capillary::CapillaryEntityObservationProfile
load_observation_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_entity_observation_profile(
        {root / "examples/capillary-models/synthetic-nanodevice-observation-v1.json",
         root / "data/schemas/capillary-entity-observation-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::capillary::CapillaryEntityDispositionProfile
load_disposition_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_entity_disposition_profile(
        {root / "examples/capillary-models/synthetic-nanodevice-disposition-v1.json",
         root / "data/schemas/capillary-entity-disposition-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::coupling::EntityTransfer
entity_transfer(const std::uint64_t id, std::string entity_type = "nanodevice") {
    return {std::string{mehlissa::models::coupling::entity_transfer_contract_version},
            id,
            std::move(entity_type),
            "organ.synthetic",
            "capillary-departure",
            "capillary.synthetic.reference.v2",
            "arteriole-entry",
            0ns};
}

struct DispositionRun final {
    std::vector<mehlissa::models::coupling::EntityTransfer> returned;
    std::vector<EntityDispositionTransfer> terminal;
    std::vector<mehlissa::core::RandomStreamState> random_streams;
};

[[nodiscard]] DispositionRun run_dispositions(const std::chrono::nanoseconds step) {
    CapillaryBedProfiles profiles;
    profiles.entity_observation = load_observation_profile();
    profiles.entity_disposition = load_disposition_profile();
    mehlissa::core::ComponentHost host{std::uint64_t{20260830}};
    auto component = std::make_unique<CapillaryBed>(load_capillary_config(), std::move(profiles));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    for (std::uint64_t id = 1; id <= 256; ++id) {
        capillary->accept_entity(entity_transfer(id));
    }
    for (auto elapsed = 0ns; elapsed < 1s; elapsed += step) {
        host.advance(step);
    }
    return {capillary->take_outbound_entities(), capillary->take_outbound_entity_dispositions(),
            host.context().random_stream_states()};
}

} // namespace

TEST_CASE("A strict entity-disposition profile maps every terminal outcome to one owner",
          "[m4][capillary][entity-disposition][schema]") {
    const auto profile = load_disposition_profile();
    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-nanodevice-disposition-v1");
    CHECK(profile.compatible_observation_profile_id == "synthetic-nanodevice-observation-v1");
    CHECK(profile.random_stream_name == "capillary.entity-disposition.synthetic-nanodevice");
    CHECK(profile.retention_target.model_id == "tissue.synthetic");
    CHECK(profile.retention_target.compartment_id == "capillary-lumen-retained");
    CHECK(profile.adhesion_target.compartment_id == "endothelial-surface-adhered");
    CHECK(profile.extravasation_target.compartment_id == "perivascular-interstitium");
    CHECK(profile.sources.size() == 3);
    CHECK(profile.limitations.size() == 4);
}

TEST_CASE("Sampled capillary dispositions are conservative deterministic and step independent",
          "[m4][capillary][entity-disposition][determinism][ownership]") {
    const auto step = GENERATE(100ms, 250ms, 500ms);
    const auto result = run_dispositions(step);
    const auto reference = run_dispositions(100ms);

    REQUIRE(result.returned.size() + result.terminal.size() == 256);
    CHECK(result.returned == reference.returned);
    CHECK(result.terminal == reference.terminal);
    CHECK(result.random_streams == reference.random_streams);
    REQUIRE(result.random_streams.size() == 1);
    CHECK(result.random_streams.front().draw_count == 256);

    mehlissa::models::coupling::TerminalEntityStore store{
        "tissue.synthetic",
        {"capillary-lumen-retained", "endothelial-surface-adhered", "perivascular-interstitium"}};
    for (const auto& transfer : result.terminal) {
        store.accept_entity_disposition(transfer);
    }
    CHECK(store.resident_entity_count() == result.terminal.size());
    CHECK(result.returned.size() == 184);
    CHECK(store.resident_entity_count(EntityDispositionKind::retained) == 17);
    CHECK(store.resident_entity_count(EntityDispositionKind::adhered) == 25);
    CHECK(store.resident_entity_count(EntityDispositionKind::extravasated) == 30);
    for (const auto& transfer : result.terminal) {
        CHECK(transfer.decided_at == 1s);
        CHECK(transfer.selection_draw > 0.0);
        CHECK(transfer.selection_draw < 1.0);
        CHECK(transfer.outcome_probability > 0.0);
        CHECK(store.contains_entity(transfer.entity_id));
    }
}

TEST_CASE("Unmatched entities still pass through without consuming a disposition draw",
          "[m4][capillary][entity-disposition][unmatched]") {
    CapillaryBedProfiles profiles;
    profiles.entity_observation = load_observation_profile();
    profiles.entity_disposition = load_disposition_profile();
    mehlissa::core::ComponentHost host{std::uint64_t{19}};
    auto component = std::make_unique<CapillaryBed>(load_capillary_config(), std::move(profiles));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(1, "biosensor"));
    host.advance(1s);

    REQUIRE(capillary->take_outbound_entities().size() == 1);
    CHECK(capillary->take_outbound_entity_dispositions().empty());
    REQUIRE(host.context().random_stream_states().size() == 1);
    CHECK(host.context().random_stream_states().front().draw_count == 0);
}

TEST_CASE("Entity disposition rejects missing observation and ambiguous targets",
          "[m4][capillary][entity-disposition][validation]") {
    CapillaryBedProfiles missing_observation;
    missing_observation.entity_disposition = load_disposition_profile();
    CHECK_THROWS_AS(CapillaryBed(load_capillary_config(), std::move(missing_observation)),
                    mehlissa::core::MehlissaError);

    auto wrong_observation = load_disposition_profile();
    wrong_observation.compatible_observation_profile_id = "observation.other";
    CapillaryBedProfiles wrong_profiles;
    wrong_profiles.entity_observation = load_observation_profile();
    wrong_profiles.entity_disposition = std::move(wrong_observation);
    CHECK_THROWS_AS(CapillaryBed(load_capillary_config(), std::move(wrong_profiles)),
                    mehlissa::core::MehlissaError);

    auto ambiguous = load_disposition_profile();
    ambiguous.adhesion_target.compartment_id = ambiguous.retention_target.compartment_id;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_entity_disposition_profile(ambiguous),
        mehlissa::core::MehlissaError);
}

TEST_CASE("Terminal stores reject invalid targets kinds and duplicate ownership",
          "[m4][coupling][entity-disposition][validation]") {
    mehlissa::models::coupling::TerminalEntityStore store{"tissue.synthetic",
                                                          {"capillary-lumen-retained"}};
    EntityDispositionTransfer transfer{
        std::string{mehlissa::models::coupling::entity_disposition_contract_version},
        7,
        "nanodevice",
        EntityDispositionKind::retained,
        "synthetic-nanodevice-disposition-v1",
        "capillary.synthetic.reference.v2",
        "terminal-disposition",
        "tissue.synthetic",
        "capillary-lumen-retained",
        1s,
        0.75,
        0.05};
    store.accept_entity_disposition(transfer);
    CHECK(store.resident_entity_count() == 1);
    CHECK_THROWS_AS(store.accept_entity_disposition(transfer), mehlissa::core::MehlissaError);

    auto wrong_target = transfer;
    wrong_target.entity_id = 8;
    wrong_target.target_model_id = "tissue.other";
    CHECK_THROWS_AS(store.accept_entity_disposition(wrong_target), mehlissa::core::MehlissaError);

    auto invalid_kind = transfer;
    invalid_kind.entity_id = 9;
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    invalid_kind.disposition = static_cast<EntityDispositionKind>(255);
    CHECK_THROWS_AS(mehlissa::models::coupling::validate_entity_disposition(invalid_kind),
                    mehlissa::core::MehlissaError);
}

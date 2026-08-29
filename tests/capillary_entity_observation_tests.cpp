// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/capillary_bed.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation.hpp>
#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>
#include <mehlissa/models/capillary/capillary_recruitment_profile.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::capillary::CapillaryBed;
using mehlissa::models::capillary::CapillaryBedProfiles;
using mehlissa::models::capillary::CapillaryEntityObservationProfile;

[[nodiscard]] mehlissa::models::capillary::CapillaryBedConfig load_capillary_config() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_bed_definition(
               {
                   root / "examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json",
                   root / "data/schemas/capillary-bed-definition/2.0.0.schema.json",
               })
        .model;
}

[[nodiscard]] CapillaryEntityObservationProfile load_observation_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_entity_observation_profile({
        root / "examples/capillary-models/synthetic-nanodevice-observation-v1.json",
        root / "data/schemas/capillary-entity-observation-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::capillary::CapillaryRecruitmentProfile load_recruitment_profile() {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::capillary::load_capillary_recruitment_profile({
        root / "examples/capillary-models/synthetic-recruitment-fixed-flow-v1.json",
        root / "data/schemas/capillary-recruitment-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] mehlissa::models::coupling::EntityTransfer
entity_transfer(const std::uint64_t id, std::string entity_type = "nanodevice") {
    return {
        std::string{mehlissa::models::coupling::entity_transfer_contract_version},
        id,
        std::move(entity_type),
        "organ.synthetic",
        "capillary-departure",
        "capillary.synthetic.reference.v2",
        "arteriole-entry",
        0ns,
    };
}

[[nodiscard]] std::unique_ptr<CapillaryBed>
observed_bed(CapillaryEntityObservationProfile profile = load_observation_profile()) {
    CapillaryBedProfiles profiles;
    profiles.entity_observation = std::move(profile);
    return std::make_unique<CapillaryBed>(load_capillary_config(), std::move(profiles));
}

} // namespace

TEST_CASE("A strict capillary entity-observation profile loads with scoped evidence",
          "[m4][capillary][entity-observation][schema]") {
    const auto profile = load_observation_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "synthetic-nanodevice-observation-v1");
    CHECK(profile.compatible_model_id == "capillary.synthetic.reference.v2");
    CHECK(profile.unmatched_entity_policy == "observe_residence_only");
    CHECK(profile.maximum_buffered_records == 1024);
    REQUIRE(profile.entity_rules.size() == 1);
    CHECK(profile.entity_rules.front().entity_type == "nanodevice");
    CHECK(profile.entity_rules.front().retention_rate_per_second == 0.1);
    CHECK(profile.sources.size() == 2);
    CHECK(profile.limitations.size() == 4);
}

TEST_CASE("Capillary positions expose local axial progress and accumulated residence",
          "[m4][capillary][entity-observation][position]") {
    mehlissa::core::ComponentHost host{std::uint64_t{61}};
    auto component = observed_bed();
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(42));

    host.advance(100ms);

    const auto positions = capillary->entity_positions();
    REQUIRE(positions.size() == 1);
    const auto& position = positions.front();
    CHECK(position.entity_id == 42);
    CHECK(position.entity_type == "nanodevice");
    CHECK(position.region_id == "feeding-arteriole");
    CHECK(position.region_kind == "arteriole");
    CHECK(mehlissa::core::in_meters(position.axial_position) == Catch::Approx(0.0004));
    CHECK(position.axial_fraction == Catch::Approx(0.5));
    CHECK(position.accumulated_residence_time == 100ms);
}

TEST_CASE("Residence-sensitive entity likelihoods are normalized and do not change routing",
          "[m4][capillary][entity-observation][likelihood]") {
    mehlissa::core::ComponentHost host{std::uint64_t{62}};
    auto component = observed_bed();
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(42));

    host.advance(1s);

    const auto outbound = capillary->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().entity_id == 42);
    CHECK(outbound.front().target_model_id == "organ.synthetic");
    CHECK(outbound.front().target_port_id == "capillary-return");
    CHECK(outbound.front().emitted_at == 1s);

    CHECK(capillary->has_entity_observation_profile());
    CHECK(capillary->entity_observation_profile_id() == "synthetic-nanodevice-observation-v1");
    const auto records = capillary->take_entity_observation_records();
    REQUIRE(records.size() == 1);
    const auto& record = records.front();
    CHECK(record.entity_id == 42);
    CHECK(record.profile_id == "synthetic-nanodevice-observation-v1");
    CHECK(record.reported_at == 1s);
    CHECK(record.region_residence_times[0] == 200ms);
    CHECK(record.region_residence_times[1] == 600ms);
    CHECK(record.region_residence_times[2] == 200ms);
    CHECK(mehlissa::models::capillary::total_residence_time(record) == 1s);
    CHECK(record.interaction_rule_applied);
    CHECK(record.pass_through_likelihood == Catch::Approx(std::exp(-0.36)));
    CHECK(record.retention_likelihood == Catch::Approx((1.0 - std::exp(-0.36)) / 6.0));
    CHECK(record.adhesion_likelihood == Catch::Approx((1.0 - std::exp(-0.36)) / 3.0));
    CHECK(record.extravasation_likelihood == Catch::Approx((1.0 - std::exp(-0.36)) / 2.0));
    CHECK(mehlissa::models::capillary::has_normalized_outcome_likelihoods(record));
    CHECK(mehlissa::models::capillary::total_outcome_likelihood(record) == Catch::Approx(1.0));
}

TEST_CASE("Unmatched entities retain residence observations and certain pass-through",
          "[m4][capillary][entity-observation][unmatched]") {
    mehlissa::core::ComponentHost host{std::uint64_t{63}};
    auto component = observed_bed();
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(43, "biosensor"));

    host.advance(1s);

    REQUIRE(capillary->take_outbound_entities().size() == 1);
    const auto records = capillary->take_entity_observation_records();
    REQUIRE(records.size() == 1);
    CHECK_FALSE(records.front().interaction_rule_applied);
    CHECK(records.front().pass_through_likelihood == 1.0);
    CHECK(records.front().retention_likelihood == 0.0);
    CHECK(mehlissa::models::capillary::total_residence_time(records.front()) == 1s);
}

TEST_CASE("Recruitment events preserve exact regional entity residence",
          "[m4][capillary][entity-observation][recruitment]") {
    auto recruitment = load_recruitment_profile();
    recruitment.states[1].effective_at = 400ms;
    CapillaryBedProfiles profiles;
    profiles.recruitment = std::move(recruitment);
    profiles.entity_observation = load_observation_profile();

    mehlissa::core::ComponentHost host{std::uint64_t{65}};
    auto component = std::make_unique<CapillaryBed>(load_capillary_config(), std::move(profiles));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(44));

    while (capillary->resident_entity_count() != 0) {
        host.advance(350ms);
    }

    const auto outbound = capillary->take_outbound_entities();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound.front().emitted_at == 1400ms);
    const auto records = capillary->take_entity_observation_records();
    REQUIRE(records.size() == 1);
    CHECK(records.front().region_residence_times[0] == 200ms);
    CHECK(records.front().region_residence_times[1] == 1000ms);
    CHECK(records.front().region_residence_times[2] == 200ms);
    CHECK(mehlissa::models::capillary::total_residence_time(records.front()) == 1400ms);
}

TEST_CASE("Entity observation buffering is bounded and reports dropped records",
          "[m4][capillary][entity-observation][bounded]") {
    auto profile = load_observation_profile();
    profile.maximum_buffered_records = 1;
    mehlissa::core::ComponentHost host{std::uint64_t{64}};
    auto component = observed_bed(std::move(profile));
    auto* capillary = component.get();
    host.add(std::move(component));
    host.initialize();
    capillary->accept_entity(entity_transfer(1));
    capillary->accept_entity(entity_transfer(2));

    host.advance(1s);

    CHECK(capillary->take_outbound_entities().size() == 2);
    CHECK(capillary->entity_observation_record_count() == 1);
    CHECK(capillary->dropped_entity_observation_record_count() == 1);
}

TEST_CASE("Entity observation rejects incompatible and invalid rate profiles",
          "[m4][capillary][entity-observation][validation]") {
    auto wrong_model = load_observation_profile();
    wrong_model.compatible_model_id = "capillary.other";
    CapillaryBedProfiles wrong_profiles;
    wrong_profiles.entity_observation = std::move(wrong_model);
    CHECK_THROWS_AS(CapillaryBed(load_capillary_config(), std::move(wrong_profiles)),
                    mehlissa::core::MehlissaError);

    auto duplicate_type = load_observation_profile();
    duplicate_type.entity_rules.push_back(duplicate_type.entity_rules.front());
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_entity_observation_profile(duplicate_type),
        mehlissa::core::MehlissaError);

    auto negative_rate = load_observation_profile();
    negative_rate.entity_rules.front().adhesion_rate_per_second = -0.1;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_entity_observation_profile(negative_rate),
        mehlissa::core::MehlissaError);

    auto zero_rates = load_observation_profile();
    zero_rates.entity_rules.front() = {"nanodevice", 0.0, 0.0, 0.0};
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_capillary_entity_observation_profile(zero_rates),
        mehlissa::core::MehlissaError);
}

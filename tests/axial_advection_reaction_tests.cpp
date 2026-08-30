// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/capillary/axial_advection_reaction_profile.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <filesystem>

namespace {

[[nodiscard]] std::filesystem::path test_root() { return {MEHLISSA_TEST_ROOT}; }

[[nodiscard]] mehlissa::models::capillary::AxialAdvectionReactionProfile load_profile() {
    return mehlissa::models::capillary::load_axial_advection_reaction_profile(
        {test_root() / "examples/capillary-models/pulmonary-shared-axial-transport-v1.json",
         test_root() / "data/schemas/axial-advection-reaction-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::capillary::CapillaryBedDefinition load_definition() {
    return mehlissa::models::capillary::load_capillary_bed_definition(
        {test_root() / "examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json",
         test_root() / "data/schemas/capillary-bed-definition/3.0.0.schema.json"});
}

} // namespace

TEST_CASE("A strict shared axial profile closes against the pulmonary capillary card",
          "[m4][channel][axial][schema]") {
    const auto profile = load_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "pulmonary-shared-axial-transport-v1");
    CHECK(profile.implementation_kind ==
          mehlissa::models::capillary::axial_advection_reaction_case_kind);
    CHECK(profile.compatible_capillary_definition_id == "pulmonary-healthy-adult-rest-supine-v1");
    CHECK(profile.capillary_region_id == "alveolar-capillary-network");
    CHECK(profile.particle.sample_count == 200'000);
    CHECK(profile.coarse_field_cell_count == 256);
    CHECK(profile.refined_field.cell_count == 512);
    CHECK(profile.sources.size() == 5);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("Particles and finite volumes pass one advection diffusion and surface-reaction case",
          "[m4][channel][axial][comparison]") {
    const auto profile = load_profile();
    const auto result = mehlissa::models::capillary::verify_axial_advection_reaction_profile(
        profile, load_definition());

    CAPTURE(result.reference.receiver_amount_fraction, result.reference.active_amount_fraction,
            result.reference.bulk_reacted_amount_fraction,
            result.reference.wall_reacted_amount_fraction,
            result.particle.receiver_observation_count, result.particle.receiver_amount_fraction,
            result.particle_reference_standardized_error, result.coarse_field.time_step_count,
            result.coarse_field.receiver_amount_fraction, result.coarse_relative_reference_error,
            result.refined_field.time_step_count, result.refined_field.receiver_amount_fraction,
            result.refined_relative_reference_error, result.relative_refinement_difference);
    CHECK(std::abs(result.reference.effective_wall_reaction_rate_per_second - 0.0634920634920635) <=
          1.0e-15);
    CHECK(result.reference.bulk_reacted_amount_fraction > 0.0);
    CHECK(result.reference.wall_reacted_amount_fraction > 0.0);
    CHECK(result.particle.bulk_reacted_count > 0);
    CHECK(result.particle.wall_reacted_count > 0);
    CHECK(result.particle.receiver_observation_count >= 50'000);
    CHECK(result.coarse_field.final_field.size() == 256);
    CHECK(result.refined_field.final_field.size() == 512);
    CHECK(std::abs(result.particle_reference_standardized_error) <= 4.0);
    CHECK(result.coarse_relative_reference_error <= 0.03);
    CHECK(result.refined_relative_reference_error <= 0.015);
    CHECK(result.relative_refinement_difference <= 0.03);
    CHECK(result.passes);
}

TEST_CASE("Every axial resolution conserves active reacted and escaped signal",
          "[m4][channel][axial][conservation]") {
    const auto profile = load_profile();
    const auto result = mehlissa::models::capillary::verify_axial_advection_reaction_profile(
        profile, load_definition());
    const auto repeated = mehlissa::models::capillary::verify_axial_advection_reaction_profile(
        profile, load_definition());

    CHECK(std::abs(result.reference.conservation_residual) <= 1.0e-12);
    CHECK(std::abs(result.particle.conservation_residual) <= 1.0e-12);
    CHECK(std::abs(result.coarse_field.conservation_residual) <= 1.0e-10);
    CHECK(std::abs(result.refined_field.conservation_residual) <= 1.0e-10);
    CHECK(result.coarse_field.escaped_left_amount_fraction +
              result.coarse_field.escaped_right_amount_fraction <=
          1.0e-8);
    CHECK(result.refined_field.escaped_left_amount_fraction +
              result.refined_field.escaped_right_amount_fraction <=
          1.0e-8);
    CHECK(result.particle.receiver_observation_count ==
          repeated.particle.receiver_observation_count);
    CHECK(result.particle.bulk_reacted_count == repeated.particle.bulk_reacted_count);
    CHECK(result.particle.wall_reacted_count == repeated.particle.wall_reacted_count);
    CHECK(result.refined_field.receiver_amount_fraction ==
          repeated.refined_field.receiver_amount_fraction);
}

TEST_CASE("The shared axial case rejects incompatible anatomy and invalid refinement",
          "[m4][channel][axial][validation]") {
    const auto definition = load_definition();
    auto wrong_radius = load_profile();
    wrong_radius.channel.lumen_radius = mehlissa::core::meters(4.0e-6);
    CHECK_THROWS_AS(mehlissa::models::capillary::verify_axial_advection_reaction_profile(
                        wrong_radius, definition),
                    mehlissa::core::MehlissaError);

    auto wrong_speed = load_profile();
    wrong_speed.channel.mean_advection_speed = mehlissa::core::meters_per_second(0.001);
    CHECK_THROWS_AS(mehlissa::models::capillary::verify_axial_advection_reaction_profile(
                        wrong_speed, definition),
                    mehlissa::core::MehlissaError);

    auto invalid_grid = load_profile();
    invalid_grid.coarse_field_cell_count = 300;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_axial_advection_reaction_profile(invalid_grid),
        mehlissa::core::MehlissaError);

    auto invalid_particle = load_profile();
    invalid_particle.particle.sample_count = 1;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_axial_advection_reaction_profile(invalid_particle),
        mehlissa::core::MehlissaError);
}

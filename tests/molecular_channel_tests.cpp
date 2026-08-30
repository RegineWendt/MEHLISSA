// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/analytical_diffusion_channel.hpp>
#include <mehlissa/models/capillary/brownian_particle_channel_profile.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/molecular_channel_comparison.hpp>
#include <mehlissa/models/capillary/molecular_channel_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <string>

namespace {

using mehlissa::models::capillary::BrownianParticleChannelProfile;
using mehlissa::models::capillary::CapillaryBedDefinition;
using mehlissa::models::capillary::MolecularChannelProfile;

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] CapillaryBedDefinition load_pulmonary_capillary_definition() {
    const auto root = project_root();
    return mehlissa::models::capillary::load_capillary_bed_definition({
        root / "examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json",
        root / "data/schemas/capillary-bed-definition/3.0.0.schema.json",
    });
}

[[nodiscard]] MolecularChannelProfile load_channel_profile() {
    const auto root = project_root();
    return mehlissa::models::capillary::load_molecular_channel_profile({
        root / "examples/capillary-models/pulmonary-synthetic-tracer-diffusion-v1.json",
        root / "data/schemas/molecular-channel-profile/1.0.0.schema.json",
    });
}

[[nodiscard]] BrownianParticleChannelProfile load_particle_profile() {
    const auto root = project_root();
    return mehlissa::models::capillary::load_brownian_particle_channel_profile({
        root / "examples/capillary-models/pulmonary-synthetic-tracer-brownian-v1.json",
        root / "data/schemas/brownian-particle-channel-profile/1.0.0.schema.json",
    });
}

} // namespace

TEST_CASE("A strict molecular-channel profile loads with explicit evidence scope",
          "[m4][channel][schema]") {
    const auto profile = load_channel_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "pulmonary-synthetic-tracer-diffusion-v1");
    CHECK(profile.implementation_kind ==
          mehlissa::models::capillary::analytical_free_diffusion_3d_kind);
    CHECK(profile.reference_case.compatible_capillary_definition_id ==
          "pulmonary-healthy-adult-rest-supine-v1");
    CHECK(profile.validity.evidence_class == "software_test_surrogate");
    CHECK(profile.sources.size() == 4);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("The pulmonary capillary card supplies a bounded local channel reference",
          "[m4][channel][pulmonary][reference]") {
    const auto profile = load_channel_profile();
    const auto definition = load_pulmonary_capillary_definition();
    const auto request =
        mehlissa::models::capillary::make_capillary_reference_request(profile, definition);

    CHECK(request.context_model_id == "capillary.lung.healthy-adult-rest-supine.v1");
    CHECK(request.region_id == "alveolar-capillary-network");
    CHECK(mehlissa::core::in_meters(request.transmitter_receiver_separation) ==
          Catch::Approx(3.15e-6));
    const auto expected_receiver_volume =
        4.0 * std::numbers::pi * 0.315e-6 * 0.315e-6 * 0.315e-6 / 3.0;
    CHECK(mehlissa::core::in_cubic_meters(request.passive_receiver_volume) ==
          Catch::Approx(expected_receiver_volume));
    CHECK(std::chrono::duration<double>{request.observation_time}.count() ==
          Catch::Approx(0.00165375));
    CHECK(request.observation_time < std::chrono::duration_cast<std::chrono::nanoseconds>(
                                         std::chrono::duration<double>{0.859}));
}

TEST_CASE("The analytical implementation satisfies the free-diffusion impulse response",
          "[m4][channel][analytical]") {
    const auto profile = load_channel_profile();
    const auto definition = load_pulmonary_capillary_definition();
    const auto channel = mehlissa::models::capillary::make_molecular_channel(profile);
    const auto request =
        mehlissa::models::capillary::make_capillary_reference_request(profile, definition);

    REQUIRE(channel->kind() == mehlissa::models::capillary::analytical_free_diffusion_3d_kind);
    const auto response = channel->evaluate(request);

    CHECK(response.request_id == request.request_id);
    CHECK(response.signal_id == request.signal_id);
    CHECK(response.channel_model_id == "channel.diffusion.synthetic-tracer.pulmonary.v1");
    CHECK(mehlissa::core::in_moles_per_cubic_meter(response.expected_receiver_concentration) ==
          Catch::Approx(0.00235525912640186));
    CHECK(mehlissa::core::in_moles(response.expected_receiver_amount) ==
          Catch::Approx(3.08360659607539e-22));
    CHECK(response.expected_receiver_fraction == Catch::Approx(0.000308360659607539));
    CHECK(mehlissa::core::in_moles(response.expected_receiver_amount) ==
          Catch::Approx(
              mehlissa::core::in_moles_per_cubic_meter(response.expected_receiver_concentration) *
              mehlissa::core::in_cubic_meters(request.passive_receiver_volume)));
}

TEST_CASE("Channel contracts reject incompatible contexts and invalid receiver approximations",
          "[m4][channel][validation]") {
    const auto profile = load_channel_profile();
    auto incompatible_definition = load_pulmonary_capillary_definition();
    incompatible_definition.definition_id = "another-capillary-definition";
    CHECK_THROWS_AS(mehlissa::models::capillary::make_capillary_reference_request(
                        profile, incompatible_definition),
                    mehlissa::core::MehlissaError);

    const mehlissa::models::capillary::AnalyticalDiffusionChannel channel{profile.channel};
    auto request = mehlissa::models::capillary::make_capillary_reference_request(
        profile, load_pulmonary_capillary_definition());
    const auto separation = mehlissa::core::in_meters(request.transmitter_receiver_separation);
    const auto oversized_radius = separation * 0.2;
    request.passive_receiver_volume = mehlissa::core::cubic_meters(
        4.0 * std::numbers::pi * oversized_radius * oversized_radius * oversized_radius / 3.0);
    CHECK_THROWS_AS(channel.evaluate(request), mehlissa::core::MehlissaError);

    auto invalid_profile = profile;
    invalid_profile.reference_case.receiver_radius_fraction_of_separation = 0.2;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_molecular_channel_profile(invalid_profile),
        mehlissa::core::MehlissaError);
}

TEST_CASE("A strict Brownian particle profile loads with a predeclared comparison gate",
          "[m4][channel][particle][schema]") {
    const auto profile = load_particle_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.profile_id == "pulmonary-synthetic-tracer-brownian-v1");
    CHECK(profile.implementation_kind ==
          mehlissa::models::capillary::brownian_particle_endpoint_3d_kind);
    CHECK(profile.compatible_analytical_profile_id == "pulmonary-synthetic-tracer-diffusion-v1");
    CHECK(profile.channel.sample_count == 2'000'000);
    CHECK(profile.comparison_gate.minimum_receiver_observation_count == 200);
    CHECK(profile.sources.size() == 4);
    CHECK(profile.limitations.size() == 5);
}

TEST_CASE("Brownian endpoints reproducibly agree with the analytical pulmonary case",
          "[m4][channel][particle][comparison]") {
    const auto analytical_profile = load_channel_profile();
    const auto particle_profile = load_particle_profile();
    const auto definition = load_pulmonary_capillary_definition();
    const auto analytical = mehlissa::models::capillary::make_molecular_channel(analytical_profile);
    const auto particle = mehlissa::models::capillary::make_brownian_particle_channel(
        particle_profile, analytical_profile);
    const mehlissa::models::capillary::MolecularChannel& interchangeable_particle = particle;
    const auto request = mehlissa::models::capillary::make_capillary_reference_request(
        analytical_profile, definition);

    CHECK(interchangeable_particle.kind() ==
          mehlissa::models::capillary::brownian_particle_endpoint_3d_kind);
    const auto first = mehlissa::models::capillary::compare_molecular_channels(
        *analytical, particle, request, particle_profile.comparison_gate);
    const auto repeated = mehlissa::models::capillary::compare_molecular_channels(
        *analytical, particle, request, particle_profile.comparison_gate);

    CHECK(first.particle.sample_count == 2'000'000);
    CHECK(first.particle.receiver_observation_count >= 200);
    CHECK(first.particle.receiver_observation_count ==
          repeated.particle.receiver_observation_count);
    CHECK(first.particle.response.expected_receiver_fraction ==
          repeated.particle.response.expected_receiver_fraction);
    CHECK(first.particle.receiver_fraction_standard_error > 0.0);
    CHECK(std::abs(first.standardized_error) <= 4.0);
    CHECK(first.relative_fraction_error <= 0.15);
    CHECK(first.passes);
    CHECK(mehlissa::core::in_moles(first.particle.response.expected_receiver_amount) ==
          Catch::Approx(mehlissa::core::in_moles(request.emitted_amount) *
                        first.particle.response.expected_receiver_fraction));
}

TEST_CASE("Particle comparison rejects incompatible profiles and invalid gates",
          "[m4][channel][particle][validation]") {
    const auto analytical_profile = load_channel_profile();
    auto incompatible = load_particle_profile();
    incompatible.channel.diffusion_coefficient = mehlissa::core::square_meters_per_second(2.0e-9);
    CHECK_THROWS_AS(mehlissa::models::capillary::make_brownian_particle_channel(incompatible,
                                                                                analytical_profile),
                    mehlissa::core::MehlissaError);

    auto invalid_gate = load_particle_profile();
    invalid_gate.comparison_gate.maximum_relative_fraction_error = 1.5;
    CHECK_THROWS_AS(
        mehlissa::models::capillary::validate_brownian_particle_channel_profile(invalid_gate),
        mehlissa::core::MehlissaError);
}

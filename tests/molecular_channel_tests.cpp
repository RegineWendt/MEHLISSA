// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/capillary/analytical_diffusion_channel.hpp>
#include <mehlissa/models/capillary/capillary_bed_definition.hpp>
#include <mehlissa/models/capillary/molecular_channel_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <numbers>
#include <string>

namespace {

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

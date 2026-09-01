// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/local_communication_profile.hpp>
#include <mehlissa/models/iot/molecular_detection.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>
#include <mehlissa/models/iot/one_hop_link.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <string>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::iot::CommunicationMetrics;
using mehlissa::models::iot::LocalCommunicationProfile;
using mehlissa::models::iot::MolecularDetectionEvent;
using mehlissa::models::iot::Nanodevice;
using mehlissa::models::iot::OneHopCommunicationSession;
using mehlissa::models::iot::OneHopDeliveryStatus;
using mehlissa::models::iot::OneHopDropReason;
using mehlissa::models::iot::ScheduledOneHopLink;

[[nodiscard]] std::filesystem::path project_root() {
    return std::filesystem::path{MEHLISSA_TEST_ROOT};
}

[[nodiscard]] LocalCommunicationProfile load_communication_profile() {
    const auto root = project_root();
    return mehlissa::models::iot::load_local_communication_profile(
        {root / "examples/iot-models/synthetic-local-communication-v1.json",
         root / "data/schemas/local-communication-profile/1.0.0.schema.json"});
}

[[nodiscard]] Nanodevice load_device(const std::string_view file_name) {
    const auto root = project_root();
    const auto profile = mehlissa::models::iot::load_nanodevice_profile(
        {root / "examples/iot-models" / file_name,
         root / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
    return Nanodevice{profile.device};
}

[[nodiscard]] MolecularDetectionEvent detection(const std::string& id,
                                                const std::chrono::milliseconds time) {
    return {std::string{mehlissa::models::iot::molecular_detection_contract_version},
            id,
            "cell.receptor-ligand.synthetic.v1",
            "synthetic-binding-step-v1",
            "locator.synthetic.1",
            "synthetic-ligand",
            "synthetic-cell-surface",
            time,
            0.7362632708334493};
}

} // namespace

TEST_CASE("A strict local-communication profile binds adapter link metrics and scope",
          "[m6][iot][link][schema]") {
    const auto profile = load_communication_profile();

    CHECK(profile.schema_version == "1.0.0");
    CHECK(profile.detection_adapter.source_device_id == "locator.synthetic.1");
    CHECK(profile.detection_adapter.target_device_id == "collector.synthetic.1");
    CHECK(profile.link.link_id == "link.synthetic.locator-collector");
    CHECK(profile.link.latency == 25ms);
    CHECK(profile.link.repeating_outcomes.size() == 3);
    CHECK(profile.reference_case.expected_latency == profile.link.latency);
    CHECK(profile.validity.evidence_class == "software_test_surrogate");
    CHECK(profile.sources.size() == 2);
    CHECK(profile.limitations.size() == 4);
}

TEST_CASE("Scheduled one-hop outcomes produce separate exact communication metrics",
          "[m6][iot][link][metrics][reference]") {
    const auto profile = load_communication_profile();
    auto locator = load_device("synthetic-locator-v1.json");
    auto collector = load_device("synthetic-collector-v1.json");
    ScheduledOneHopLink link{profile.link};
    OneHopCommunicationSession session{link};

    const auto first = session.exchange(
        locator, collector,
        mehlissa::models::iot::make_detection_message_request(
            profile.detection_adapter, detection("detection.m6-2.metrics.1", 100ms)));
    CHECK(first.transmission.status == OneHopDeliveryStatus::delivered);
    CHECK(first.transmission.drop_reason == OneHopDropReason::none);
    CHECK(first.transmission.completed_at == 125ms);

    const auto second = session.exchange(
        locator, collector,
        mehlissa::models::iot::make_detection_message_request(
            profile.detection_adapter, detection("detection.m6-2.metrics.2", 200ms)));
    CHECK(second.transmission.status == OneHopDeliveryStatus::dropped);
    CHECK(second.transmission.drop_reason == OneHopDropReason::loss);

    const auto third = session.exchange(
        locator, collector,
        mehlissa::models::iot::make_detection_message_request(
            profile.detection_adapter, detection("detection.m6-2.metrics.3", 300ms)));
    CHECK(third.transmission.status == OneHopDeliveryStatus::dropped);
    CHECK(third.transmission.drop_reason == OneHopDropReason::corruption);

    const CommunicationMetrics& metrics = session.metrics();
    CHECK(metrics.attempted_messages == 3);
    CHECK(metrics.delivered_messages == 1);
    CHECK(metrics.lost_messages == 1);
    CHECK(metrics.corrupted_messages == 1);
    CHECK(metrics.expired_messages == 0);
    CHECK(metrics.attempted_bytes == 960);
    CHECK(metrics.delivered_bytes == 320);
    CHECK(metrics.total_delivery_latency == 25ms);
    CHECK(metrics.maximum_delivery_latency == 25ms);
    CHECK(mehlissa::models::iot::mean_delivery_latency(metrics) == 25ms);
    CHECK(mehlissa::models::iot::delivery_fraction(metrics) == Catch::Approx(1.0 / 3.0));
    CHECK(mehlissa::models::iot::drop_fraction(metrics) == Catch::Approx(2.0 / 3.0));
    CHECK(mehlissa::models::iot::channel_loss_fraction(metrics) == Catch::Approx(1.0 / 3.0));
    CHECK(mehlissa::models::iot::corruption_fraction(metrics) == Catch::Approx(1.0 / 3.0));
    CHECK(mehlissa::core::in_joules(metrics.transmitter_energy) == Catch::Approx(1.5e-6));
    CHECK(mehlissa::core::in_joules(metrics.receiver_energy) == Catch::Approx(0.25e-6));
    CHECK(mehlissa::core::in_joules(metrics.link_energy) == Catch::Approx(0.3e-6));
    CHECK(collector.reception_count() == 1);
}

TEST_CASE("One-hop validity expiry is a reported drop rather than a receiver mutation",
          "[m6][iot][link][expiry]") {
    const auto profile = load_communication_profile();
    auto link_config = profile.link;
    link_config.latency = 2s;
    link_config.repeating_outcomes = {mehlissa::models::iot::ScheduledLinkOutcome::delivered};
    ScheduledOneHopLink link{link_config};
    OneHopCommunicationSession session{link};
    auto locator = load_device("synthetic-locator-v1.json");
    auto collector = load_device("synthetic-collector-v1.json");

    const auto result = session.exchange(
        locator, collector,
        mehlissa::models::iot::make_detection_message_request(
            profile.detection_adapter, detection("detection.m6-2.expired", 100ms)));

    CHECK(result.transmission.status == OneHopDeliveryStatus::dropped);
    CHECK(result.transmission.drop_reason == OneHopDropReason::expired);
    CHECK(session.metrics().expired_messages == 1);
    CHECK(collector.reception_count() == 0);
    CHECK(mehlissa::core::in_joules(session.metrics().receiver_energy) == 0.0);
}

TEST_CASE("Detection and link contracts reject incompatible source and configuration",
          "[m6][iot][link][validation]") {
    const auto profile = load_communication_profile();
    auto wrong_source = detection("detection.m6-2.wrong-source", 100ms);
    wrong_source.detector_device_id = "locator.other";
    CHECK_THROWS_AS(mehlissa::models::iot::make_detection_message_request(profile.detection_adapter,
                                                                          wrong_source),
                    mehlissa::core::MehlissaError);

    auto invalid_link = profile.link;
    invalid_link.repeating_outcomes.clear();
    CHECK_THROWS_AS(ScheduledOneHopLink{invalid_link}, mehlissa::core::MehlissaError);

    auto inconsistent = profile;
    inconsistent.reference_case.expected_latency = 30ms;
    CHECK_THROWS_AS(mehlissa::models::iot::validate_local_communication_profile(inconsistent),
                    mehlissa::core::MehlissaError);
}

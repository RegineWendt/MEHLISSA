// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/iot/nanodevice.hpp>
#include <mehlissa/models/iot/nanodevice_profile.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

using namespace std::chrono_literals;
using mehlissa::models::iot::LocalMessageKind;
using mehlissa::models::iot::Nanodevice;
using mehlissa::models::iot::NanodeviceCapability;
using mehlissa::models::iot::NanodeviceLifecycleState;
using mehlissa::models::iot::NanodeviceProfile;

[[nodiscard]] NanodeviceProfile load_profile(const std::string_view file_name) {
    const auto root = std::filesystem::path{MEHLISSA_TEST_ROOT};
    return mehlissa::models::iot::load_nanodevice_profile(
        {root / "examples/iot-models" / file_name,
         root / "data/schemas/nanodevice-profile/1.0.0.schema.json"});
}

[[nodiscard]] mehlissa::models::iot::LocalMessageRequest detection_request() {
    return {"message.m6-1.detection.1",
            LocalMessageKind::detection,
            "collector.synthetic.1",
            "experiment.m6-1.local-message",
            "cell.binding.threshold.1",
            100ms,
            1s,
            1,
            64,
            "application/vnd.mehlissa.detection+json;version=1.0.0",
            R"({"signal_id":"marker.synthetic.1","detected":true})"};
}

} // namespace

TEST_CASE("Strict profiles compose specialized device capabilities and resources",
          "[m6][iot][nanodevice][schema]") {
    const auto locator = load_profile("synthetic-locator-v1.json");
    const auto collector = load_profile("synthetic-collector-v1.json");

    CHECK(locator.schema_version == "1.0.0");
    CHECK(locator.device.device_type == "nanolocator.synthetic");
    CHECK(locator.device.payloads.front().unit_count == 1);
    CHECK(locator.validity.evidence_class == "software_test_surrogate");
    CHECK(collector.device.device_type == "nanocollector.synthetic");
    CHECK(collector.device.payloads.empty());
    CHECK(collector.sources.size() == 1);
    CHECK(collector.limitations.size() == 3);
}

TEST_CASE("A locator emits and a collector receives one traceable local message",
          "[m6][iot][nanodevice][message]") {
    Nanodevice locator{load_profile("synthetic-locator-v1.json").device};
    Nanodevice collector{load_profile("synthetic-collector-v1.json").device};

    const auto message = locator.emit_local_message(detection_request());
    CHECK(message.contract_version == mehlissa::models::iot::local_message_contract_version);
    CHECK(message.source_device_id == locator.device_id());
    CHECK(message.target_device_id == collector.device_id());
    CHECK(message.correlation_id == "experiment.m6-1.local-message");
    CHECK(message.source_event_id == "cell.binding.threshold.1");
    CHECK(message.kind == LocalMessageKind::detection);
    CHECK(locator.transmission_count() == 1);
    CHECK(mehlissa::core::in_joules(locator.remaining_energy()) == Catch::Approx(1.5e-6));

    collector.receive_local_message(message, 150ms);
    CHECK(collector.reception_count() == 1);
    CHECK(collector.used_message_storage_bytes() == 64);
    CHECK(mehlissa::core::in_joules(collector.remaining_energy()) == Catch::Approx(0.75e-6));

    const auto collected = collector.take_received_messages();
    REQUIRE(collected.size() == 1);
    CHECK(collected.front() == message);
    CHECK(collector.used_message_storage_bytes() == 0);
}

TEST_CASE("Nanodevice lifecycle rejects premature use and records depletion and failure",
          "[m6][iot][nanodevice][lifecycle]") {
    auto dormant_config = load_profile("synthetic-locator-v1.json").device;
    dormant_config.initial_state = NanodeviceLifecycleState::dormant;
    dormant_config.resources.initial_energy = dormant_config.resources.transmit_energy_per_message;
    Nanodevice device{dormant_config};

    CHECK_THROWS_AS(device.emit_local_message(detection_request()), mehlissa::core::MehlissaError);
    device.activate(50ms);
    CHECK(device.state() == NanodeviceLifecycleState::active);

    static_cast<void>(device.emit_local_message(detection_request()));
    CHECK(device.state() == NanodeviceLifecycleState::depleted);
    CHECK(device.state_changed_at() == 100ms);
    CHECK(mehlissa::core::in_joules(device.remaining_energy()) == 0.0);
    CHECK_THROWS_AS(device.emit_local_message(detection_request()), mehlissa::core::MehlissaError);

    device.fail(200ms);
    CHECK(device.state() == NanodeviceLifecycleState::failed);
    CHECK(device.state_changed_at() == 200ms);
    CHECK_THROWS_AS(device.activate(300ms), mehlissa::core::MehlissaError);
}

TEST_CASE("Local endpoints reject invalid routing expiry duplicates and resource overruns",
          "[m6][iot][nanodevice][budget]") {
    Nanodevice locator{load_profile("synthetic-locator-v1.json").device};
    Nanodevice collector{load_profile("synthetic-collector-v1.json").device};

    auto oversized = detection_request();
    oversized.size_bytes = 129;
    CHECK_THROWS_AS(locator.emit_local_message(oversized), mehlissa::core::MehlissaError);
    CHECK(locator.transmission_count() == 0);

    const auto message = locator.emit_local_message(detection_request());
    CHECK_THROWS_AS(collector.receive_local_message(message, 2s), mehlissa::core::MehlissaError);
    CHECK(collector.reception_count() == 0);

    auto wrong_target = message;
    wrong_target.message_id = "message.m6-1.wrong-target";
    wrong_target.target_device_id = "collector.other";
    CHECK_THROWS_AS(collector.receive_local_message(wrong_target, 200ms),
                    mehlissa::core::MehlissaError);

    collector.receive_local_message(message, 200ms);
    CHECK_THROWS_AS(collector.receive_local_message(message, 250ms), mehlissa::core::MehlissaError);
    CHECK(collector.reception_count() == 1);
}

TEST_CASE("Semantic validation rejects ambiguous payloads and impossible capabilities",
          "[m6][iot][nanodevice][validation]") {
    auto duplicate_capability = load_profile("synthetic-locator-v1.json").device;
    duplicate_capability.capabilities.push_back(NanodeviceCapability::transmit);
    CHECK_THROWS_AS(mehlissa::models::iot::validate_nanodevice_config(duplicate_capability),
                    mehlissa::core::MehlissaError);

    auto relay_without_receive = load_profile("synthetic-locator-v1.json").device;
    relay_without_receive.capabilities.push_back(NanodeviceCapability::relay);
    CHECK_THROWS_AS(mehlissa::models::iot::validate_nanodevice_config(relay_without_receive),
                    mehlissa::core::MehlissaError);

    auto ambiguous_payload = load_profile("synthetic-locator-v1.json").device;
    ambiguous_payload.payloads.front().amount = mehlissa::core::moles(1.0e-12);
    CHECK_THROWS_AS(mehlissa::models::iot::validate_nanodevice_config(ambiguous_payload),
                    mehlissa::core::MehlissaError);

    auto duplicate_source = load_profile("synthetic-locator-v1.json");
    duplicate_source.sources.push_back(duplicate_source.sources.front());
    CHECK_THROWS_AS(mehlissa::models::iot::validate_nanodevice_profile(duplicate_source),
                    mehlissa::core::MehlissaError);
}

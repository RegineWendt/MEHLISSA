// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/core/error.hpp>
#include <mehlissa/core/quantity.hpp>
#include <mehlissa/models/body/compartment_transport.hpp>
#include <mehlissa/models/body/vascular_graph.hpp>
#include <mehlissa/models/cosimulation/body_organ_coupler.hpp>
#include <mehlissa/models/coupling/conserved_transfer.hpp>
#include <mehlissa/models/organ/lung_model_definition.hpp>
#include <mehlissa/models/organ/pulmonary_parallel_beds.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace {

using namespace std::chrono_literals;
using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;
using mehlissa::models::coupling::ConservedTransfer;
using mehlissa::models::coupling::PopulationTransfer;
using mehlissa::models::coupling::SubstanceAmountTransfer;
using mehlissa::models::coupling::TransferHeader;
using mehlissa::models::coupling::VolumeFlowTransfer;

enum class CandidateRole : std::uint8_t { coarse, detailed };

struct ModelFile final {
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

struct OrganCandidate final {
    CandidateRole role{};
    ModelFile file;
};

struct ConservedPayload final {
    std::string population_type;
    std::uint64_t population_count{};
    std::string substance_id;
    double substance_amount_mmol{};
    double volume_flow_m3_per_s{};
    std::chrono::milliseconds flow_interval{};
};

struct ResolutionScenario final {
    std::string id;
    ModelFile body;
    mehlissa::models::cosimulation::BodyOrganRoute route;
    std::string injection_segment_id;
    std::uint64_t entity_count{};
    std::uint64_t master_seed{};
    std::chrono::milliseconds synchronization_step{};
    std::chrono::milliseconds maximum_duration{};
    ConservedPayload payload;
    std::vector<OrganCandidate> candidates;
};

struct CandidateResult final {
    CandidateRole role{};
    std::string definition_id;
    std::string model_id;
    std::chrono::milliseconds completion_time{};
    std::size_t regional_bed_count{};
    std::vector<std::uint64_t> returned_entity_ids;
    std::uint64_t completed_round_trip_count{};
    std::size_t returned_conserved_transfer_count{};
    bool exact_return_segment{};
    bool ownership_closed{};
    bool conserved_payload_exact{};
    bool accepted{};
};

[[nodiscard]] std::filesystem::path root() { return std::filesystem::path{MEHLISSA_TEST_ROOT}; }

[[nodiscard]] Json read_json(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        throw std::runtime_error{"Cannot open comparison input: " + path.string()};
    }
    return Json::parse(stream);
}

[[nodiscard]] CompiledSchema load_schema() {
    return jsoncons::jsonschema::make_json_schema(read_json(
        root() / "data" / "schemas" / "body-organ-resolution-comparison" / "1.0.0.schema.json"));
}

[[nodiscard]] std::chrono::milliseconds milliseconds(const Json& object,
                                                     const std::string_view key) {
    const auto value = object.at(key).as<std::uint64_t>();
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        throw std::runtime_error{"Scenario duration exceeds milliseconds representation"};
    }
    return std::chrono::milliseconds{static_cast<std::int64_t>(value)};
}

[[nodiscard]] ModelFile decode_model_file(const Json& object) {
    return {
        root() / object.at("definition_path").as<std::string>(),
        root() / object.at("schema_path").as<std::string>(),
    };
}

[[nodiscard]] CandidateRole decode_role(const std::string_view role) {
    return role == "coarse" ? CandidateRole::coarse : CandidateRole::detailed;
}

[[nodiscard]] ResolutionScenario load_scenario() {
    const auto document =
        read_json(root() / "examples" / "scenarios" / "body-lung-resolution-comparison-v1.json");
    load_schema().validate(document);

    const auto& identity = document.at("scenario");
    const auto& route = document.at("route");
    const auto& injection = document.at("injection");
    const auto& simulation = document.at("simulation");
    const auto& payload = document.at("conserved_payload");
    ResolutionScenario result{
        identity.at("id").as<std::string>(),
        decode_model_file(document.at("body")),
        {
            route.at("body_departure_segment_id").as<std::string>(),
            route.at("body_departure_port_id").as<std::string>(),
            route.at("organ_entry_port_id").as<std::string>(),
            route.at("organ_exit_port_id").as<std::string>(),
            route.at("body_return_port_id").as<std::string>(),
            route.at("body_return_segment_id").as<std::string>(),
        },
        injection.at("segment_id").as<std::string>(),
        injection.at("entity_count").as<std::uint64_t>(),
        simulation.at("master_seed").as<std::uint64_t>(),
        milliseconds(simulation, "synchronization_step_ms"),
        milliseconds(simulation, "maximum_duration_ms"),
        {
            payload.at("population_type").as<std::string>(),
            payload.at("population_count").as<std::uint64_t>(),
            payload.at("substance_id").as<std::string>(),
            payload.at("substance_amount_mmol").as<double>(),
            payload.at("volume_flow_m3_per_s").as<double>(),
            milliseconds(payload, "flow_interval_ms"),
        },
        {},
    };

    for (const auto& candidate : document.at("organ_candidates").array_range()) {
        result.candidates.push_back({
            decode_role(candidate.at("role").as<std::string_view>()),
            decode_model_file(candidate),
        });
    }

    const auto coarse_count =
        std::ranges::count(result.candidates, CandidateRole::coarse, &OrganCandidate::role);
    const auto detailed_count =
        std::ranges::count(result.candidates, CandidateRole::detailed, &OrganCandidate::role);
    if (coarse_count != 1 || detailed_count != 1 ||
        result.injection_segment_id != result.route.body_departure_segment_id ||
        result.maximum_duration % result.synchronization_step != 0ms) {
        throw std::runtime_error{
            "Resolution comparison requires one candidate per role, one departure segment, and "
            "an integral synchronization grid"};
    }
    return result;
}

[[nodiscard]] TransferHeader input_header(const std::string& transfer_id,
                                          const std::string& body_model_id,
                                          const std::string& organ_model_id,
                                          const ResolutionScenario& scenario) {
    return {
        std::string{mehlissa::models::coupling::conserved_transfer_contract_version},
        transfer_id,
        body_model_id,
        scenario.route.body_departure_port_id,
        organ_model_id,
        scenario.route.organ_entry_port_id,
        0ms,
    };
}

[[nodiscard]] std::vector<ConservedTransfer> make_payload(const ResolutionScenario& scenario,
                                                          const std::string& body_model_id,
                                                          const std::string& organ_model_id) {
    return {
        PopulationTransfer{
            input_header("scenario-population", body_model_id, organ_model_id, scenario),
            scenario.payload.population_type, scenario.payload.population_count},
        SubstanceAmountTransfer{
            input_header("scenario-substance", body_model_id, organ_model_id, scenario),
            scenario.payload.substance_id,
            mehlissa::core::millimoles(scenario.payload.substance_amount_mmol)},
        VolumeFlowTransfer{
            input_header("scenario-flow", body_model_id, organ_model_id, scenario),
            mehlissa::core::cubic_meters_per_second(scenario.payload.volume_flow_m3_per_s),
            scenario.payload.flow_interval},
    };
}

[[nodiscard]] bool payload_matches(std::vector<ConservedTransfer> returned,
                                   std::vector<ConservedTransfer> expected,
                                   const std::string& organ_model_id,
                                   const std::string& body_model_id,
                                   const ResolutionScenario& scenario,
                                   const std::chrono::milliseconds completion_time) {
    if (returned.size() != expected.size()) {
        return false;
    }
    for (auto& transfer : expected) {
        auto& header = mehlissa::models::coupling::transfer_header(transfer);
        header.source_model_id = organ_model_id;
        header.source_port_id = scenario.route.organ_exit_port_id;
        header.target_model_id = body_model_id;
        header.target_port_id = scenario.route.body_return_port_id;
        header.emitted_at = completion_time;
    }
    const auto by_id = [](const ConservedTransfer& left, const ConservedTransfer& right) {
        return mehlissa::models::coupling::transfer_header(left).transfer_id <
               mehlissa::models::coupling::transfer_header(right).transfer_id;
    };
    std::ranges::sort(returned, by_id);
    std::ranges::sort(expected, by_id);
    return returned == expected;
}

[[nodiscard]] CandidateResult run_candidate(const ResolutionScenario& scenario,
                                            const OrganCandidate& candidate) {
    auto graph = mehlissa::models::body::load_vascular_graph(
        {scenario.body.definition_path, scenario.body.schema_path});
    const auto body_model_id = graph.model_id;
    auto definition = mehlissa::models::organ::load_lung_model_definition(
        {candidate.file.definition_path, candidate.file.schema_path});
    if ((candidate.role == CandidateRole::coarse &&
         definition.model.variant !=
             mehlissa::models::organ::LungModelVariant::effective_compartment) ||
        (candidate.role == CandidateRole::detailed &&
         definition.model.variant !=
             mehlissa::models::organ::LungModelVariant::pulmonary_zero_dimensional)) {
        throw std::runtime_error{"Scenario candidate role does not match its model variant"};
    }

    definition.model.return_target_model_id = body_model_id;
    definition.model.return_target_port_id = scenario.route.body_return_port_id;
    auto body = std::make_unique<mehlissa::models::body::CompartmentTransport>(
        std::move(graph), std::vector<mehlissa::models::body::InjectionEvent>{
                              {0ms, scenario.injection_segment_id, scenario.entity_count}});
    auto organ = mehlissa::models::organ::make_lung_model(definition.model);
    auto* body_observer = body.get();
    auto* organ_observer = organ.get();
    const std::string organ_model_id{organ_observer->model_id()};
    std::size_t regional_bed_count{};
    if (const auto* parallel =
            dynamic_cast<const mehlissa::models::organ::PulmonaryParallelBedsModel*>(
                organ_observer);
        parallel != nullptr) {
        regional_bed_count = parallel->state().beds.size();
    }

    mehlissa::core::ComponentHost host{scenario.master_seed};
    host.add(std::move(body));
    host.add(std::move(organ));
    host.initialize();

    auto entity_locations = body_observer->particle_locations();
    std::vector<std::uint64_t> expected_entity_ids;
    expected_entity_ids.reserve(entity_locations.size());
    for (const auto& location : entity_locations) {
        expected_entity_ids.push_back(location.particle_id);
    }
    std::ranges::sort(expected_entity_ids);
    if (expected_entity_ids.size() != scenario.entity_count) {
        throw std::runtime_error{"Scenario injection did not create the requested entity count"};
    }

    mehlissa::models::cosimulation::BodyOrganCoupler coupler{*body_observer, *organ_observer,
                                                             scenario.route};
    for (const auto entity_id : expected_entity_ids) {
        coupler.send_to_organ(entity_id, 0ms);
    }

    const auto expected_payload = make_payload(scenario, body_model_id, organ_model_id);
    for (const auto& transfer : expected_payload) {
        organ_observer->accept_conserved_transfer(transfer);
    }

    std::vector<ConservedTransfer> returned_payload;
    while (host.context().clock().now() < scenario.maximum_duration &&
           (coupler.completed_round_trip_count() < scenario.entity_count ||
            returned_payload.size() < expected_payload.size())) {
        host.advance(scenario.synchronization_step);
        static_cast<void>(coupler.receive_from_organ(host.context().clock().now()));
        auto outbound = organ_observer->take_outbound_conserved_transfers();
        returned_payload.insert(returned_payload.end(), std::make_move_iterator(outbound.begin()),
                                std::make_move_iterator(outbound.end()));
    }

    entity_locations = body_observer->particle_locations();
    std::vector<std::uint64_t> returned_entity_ids;
    returned_entity_ids.reserve(entity_locations.size());
    auto exact_return_segment = true;
    for (const auto& location : entity_locations) {
        returned_entity_ids.push_back(location.particle_id);
        exact_return_segment =
            exact_return_segment && location.segment_id == scenario.route.body_return_segment_id;
    }
    std::ranges::sort(returned_entity_ids);
    const auto completion_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(host.context().clock().now());
    const auto ownership_closed = body_observer->outside_body_particle_count() == 0 &&
                                  coupler.in_flight_count() == 0 &&
                                  coupler.pending_return_count() == 0 &&
                                  organ_observer->resident_conserved_transfer_count() == 0;
    const auto exact_payload = payload_matches(returned_payload, expected_payload, organ_model_id,
                                               body_model_id, scenario, completion_time);
    const auto accepted = returned_entity_ids == expected_entity_ids && exact_return_segment &&
                          ownership_closed && exact_payload &&
                          coupler.completed_round_trip_count() == scenario.entity_count;
    return {
        candidate.role,
        definition.definition_id,
        organ_model_id,
        completion_time,
        regional_bed_count,
        std::move(returned_entity_ids),
        coupler.completed_round_trip_count(),
        returned_payload.size(),
        exact_return_segment,
        ownership_closed,
        exact_payload,
        accepted,
    };
}

} // namespace

TEST_CASE("One body-lung scenario retains its meaning across coarse and five-lobe models",
          "[m3][cosimulation][resolution-comparison][scenario]") {
    const auto scenario = load_scenario();
    REQUIRE(scenario.candidates.size() == 2);

    std::vector<CandidateResult> results;
    for (const auto& candidate : scenario.candidates) {
        results.push_back(run_candidate(scenario, candidate));
    }
    REQUIRE(results.size() == 2);
    const auto coarse = std::ranges::find(results, CandidateRole::coarse, &CandidateResult::role);
    const auto detailed =
        std::ranges::find(results, CandidateRole::detailed, &CandidateResult::role);
    REQUIRE(coarse != results.end());
    REQUIRE(detailed != results.end());

    CHECK(coarse->accepted);
    CHECK(detailed->accepted);
    CHECK(coarse->returned_entity_ids == detailed->returned_entity_ids);
    CHECK(coarse->completed_round_trip_count == detailed->completed_round_trip_count);
    CHECK(coarse->returned_conserved_transfer_count == 3);
    CHECK(detailed->returned_conserved_transfer_count == 3);
    CHECK(coarse->completion_time == 2000ms);
    CHECK(detailed->completion_time == 6400ms);
    CHECK(coarse->regional_bed_count == 0);
    CHECK(detailed->regional_bed_count == 5);
}

TEST_CASE("A resolution candidate cannot override the shared body-organ route",
          "[m3][cosimulation][resolution-comparison][schema]") {
    auto document =
        read_json(root() / "examples" / "scenarios" / "body-lung-resolution-comparison-v1.json");
    document.at("organ_candidates").at(std::size_t{0})["route"] = document.at("route");
    CHECK_THROWS(load_schema().validate(document));
}

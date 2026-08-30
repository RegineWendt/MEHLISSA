// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/capillary/capillary_bed_definition.hpp>

#include <mehlissa/core/error.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::capillary {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

[[noreturn]] void invalid(const core::ErrorCode code, const std::string& message) {
    throw core::MehlissaError{code, message};
}

[[nodiscard]] Json read_json(const std::filesystem::path& path, const std::string_view role) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) {
        invalid(core::ErrorCode::input_unreadable,
                "Cannot open " + std::string{role} + ": " + path.string());
    }
    try {
        return Json::parse(stream);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::json_invalid, "Invalid JSON in " + std::string{role} + " '" +
                                                   path.string() + "': " + error.what());
    }
}

[[nodiscard]] CompiledSchema compile_schema(const Json& document,
                                            const std::filesystem::path& path) {
    try {
        return jsoncons::jsonschema::make_json_schema(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::schema_invalid,
                "Invalid capillary-bed schema '" + path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary-bed definition does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] CapillaryRegionKind decode_kind(const std::string_view kind) {
    if (kind == "arteriole") {
        return CapillaryRegionKind::arteriole;
    }
    if (kind == "capillary") {
        return CapillaryRegionKind::capillary;
    }
    if (kind == "venule") {
        return CapillaryRegionKind::venule;
    }
    invalid(core::ErrorCode::data_invalid, "Unsupported capillary-region kind");
}

[[nodiscard]] std::optional<double> optional_number(const Json& object,
                                                    const std::string_view key) {
    if (!object.contains(key)) {
        return std::nullopt;
    }
    return object.at(key).as<double>();
}

[[nodiscard]] CapillaryEvidenceQuantity decode_evidence_quantity(const Json& document) {
    const auto& uncertainty = document.at("uncertainty");
    CapillaryEvidenceQuantity result{
        document.at("value_si").as<double>(),
        document.at("unit").as<std::string>(),
        {uncertainty.at("kind").as<std::string>(),
         optional_number(uncertainty, "standard_deviation_si"),
         optional_number(uncertainty, "lower_si"), optional_number(uncertainty, "upper_si")},
        {},
        document.at("role").as<std::string>(),
        document.at("derivation").as<std::string>(),
    };
    for (const auto& source_id : document.at("source_ids").array_range()) {
        result.source_ids.push_back(source_id.as<std::string>());
    }
    return result;
}

[[nodiscard]] CapillaryBedQualification decode_qualification(const Json& document) {
    const auto& capillary = document.at("capillary");
    const auto& boundary = document.at("boundary_regions");
    return {
        document.at("geometry_semantics").as<std::string>(),
        decode_evidence_quantity(document.at("reference_flow")),
        decode_evidence_quantity(capillary.at("functional_blood_volume")),
        decode_evidence_quantity(capillary.at("morphometric_lumen_volume")),
        decode_evidence_quantity(capillary.at("morphometric_surface_area")),
        decode_evidence_quantity(capillary.at("equivalent_diameter")),
        decode_evidence_quantity(capillary.at("representative_path_length")),
        decode_evidence_quantity(capillary.at("reference_transit_time")),
        boundary.at("semantics").as<std::string>(),
        decode_evidence_quantity(boundary.at("volume_each")),
        document.at("consistency_tolerance_fraction").as<double>(),
    };
}

[[nodiscard]] CapillaryBedDefinition decode(const Json& document) {
    const auto& identity = document.at("definition");
    const auto& component = document.at("component");
    const auto& network = document.at("network");
    const auto& validity = document.at("validity");

    CapillaryBedConfig config{
        component.at("name").as<std::string>(),
        component.at("model_id").as<std::string>(),
        component.at("entry_port_id").as<std::string>(),
        component.at("exit_port_id").as<std::string>(),
        component.at("return_target_model_id").as<std::string>(),
        component.at("return_target_port_id").as<std::string>(),
        network.at("total_parallel_path_count").as<std::uint64_t>(),
        network.at("perfused_path_count").as<std::uint64_t>(),
        core::cubic_meters_per_second(network.at("volume_flow_rate_m3_s").as<double>()),
        {},
    };
    for (const auto& region : network.at("regions").array_range()) {
        config.regions.push_back({
            region.at("id").as<std::string>(),
            decode_kind(region.at("kind").as<std::string_view>()),
            core::meters(region.at("length_m").as<double>()),
            core::meters(region.at("diameter_m").as<double>()),
            region.at("parallel_vessel_count").as<std::uint64_t>(),
        });
    }

    CapillaryBedDefinition result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("version").as<std::string>(),
        identity.at("title").as<std::string>(),
        std::move(config),
        {validity.at("population").as<std::string>(),
         validity.at("physiological_state").as<std::string>(),
         validity.at("evidence_class").as<std::string>(),
         validity.at("description").as<std::string>()},
        {},
        {},
    };
    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back(
            {source.at("id").as<std::string>(), source.at("citation").as<std::string>(),
             source.at("location").as<std::string>(), source.at("license").as<std::string>(),
             source.at("role").as<std::string>()});
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    if (document.contains("qualification")) {
        result.qualification = decode_qualification(document.at("qualification"));
    }
    return result;
}

[[nodiscard]] bool approximately_equal(const double left, const double right,
                                       const double tolerance_fraction) noexcept {
    const auto scale = std::max(std::abs(left), std::abs(right));
    return std::abs(left - right) <= tolerance_fraction * scale;
}

void validate_evidence_quantity(const CapillaryEvidenceQuantity& quantity,
                                const std::unordered_set<std::string>& source_ids) {
    if (!std::isfinite(quantity.value_si) || quantity.value_si <= 0.0) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary qualification quantities must be positive and finite");
    }
    for (const auto& source_id : quantity.source_ids) {
        if (!source_ids.contains(source_id)) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary qualification references an unknown source identifier");
        }
    }
}

void validate_qualification(const CapillaryBedDefinition& definition,
                            const std::unordered_set<std::string>& source_ids) {
    if (!definition.qualification.has_value()) {
        return;
    }
    const auto& qualification = *definition.qualification;
    if (!std::isfinite(qualification.consistency_tolerance_fraction) ||
        qualification.consistency_tolerance_fraction <= 0.0 ||
        qualification.consistency_tolerance_fraction > 1.0e-3) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary qualification requires a bounded consistency tolerance");
    }
    const std::array quantities{
        &qualification.reference_flow,
        &qualification.functional_blood_volume,
        &qualification.morphometric_lumen_volume,
        &qualification.morphometric_surface_area,
        &qualification.equivalent_diameter,
        &qualification.representative_path_length,
        &qualification.reference_transit_time,
        &qualification.boundary_region_volume_each,
    };
    for (const auto* quantity : quantities) {
        validate_evidence_quantity(*quantity, source_ids);
    }

    const auto& model = definition.model;
    const auto& capillary = model.regions.at(1);
    const auto single_area = std::numbers::pi * core::in_meters(capillary.diameter) *
                             core::in_meters(capillary.diameter) / 4.0;
    const auto perfused_volume = single_area * static_cast<double>(model.perfused_path_count) *
                                 core::in_meters(capillary.length);
    const auto morphometric_capacity = single_area *
                                       static_cast<double>(model.total_parallel_path_count) *
                                       core::in_meters(capillary.length);
    const auto reference_flow = core::in_cubic_meters_per_second(model.volume_flow_rate);
    const auto transit = perfused_volume / reference_flow;
    const auto tolerance = qualification.consistency_tolerance_fraction;
    if (qualification.reference_flow.unit != "m3/s" ||
        qualification.functional_blood_volume.unit != "m3" ||
        qualification.morphometric_lumen_volume.unit != "m3" ||
        qualification.morphometric_surface_area.unit != "m2" ||
        qualification.equivalent_diameter.unit != "m" ||
        qualification.representative_path_length.unit != "m" ||
        qualification.reference_transit_time.unit != "s" ||
        qualification.boundary_region_volume_each.unit != "m3") {
        invalid(core::ErrorCode::data_invalid,
                "Capillary qualification quantities require their declared SI units");
    }
    if (!approximately_equal(reference_flow, qualification.reference_flow.value_si, tolerance) ||
        !approximately_equal(perfused_volume, qualification.functional_blood_volume.value_si,
                             tolerance) ||
        !approximately_equal(morphometric_capacity,
                             qualification.morphometric_lumen_volume.value_si, tolerance) ||
        !approximately_equal(core::in_meters(capillary.diameter),
                             qualification.equivalent_diameter.value_si, tolerance) ||
        !approximately_equal(core::in_meters(capillary.length),
                             qualification.representative_path_length.value_si, tolerance) ||
        !approximately_equal(transit, qualification.reference_transit_time.value_si, tolerance)) {
        invalid(core::ErrorCode::data_invalid,
                "Capillary qualification does not close against executable geometry and flow");
    }
    for (const auto index : {std::size_t{0}, std::size_t{2}}) {
        const auto& region = model.regions.at(index);
        const auto area = std::numbers::pi * core::in_meters(region.diameter) *
                          core::in_meters(region.diameter) / 4.0;
        const auto volume = area * static_cast<double>(region.parallel_vessel_count) *
                            core::in_meters(region.length);
        if (!approximately_equal(volume, qualification.boundary_region_volume_each.value_si,
                                 tolerance)) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary boundary-region numerical volume does not close");
        }
    }
}

void validate_definition(const CapillaryBedDefinition& definition) {
    if (definition.schema_version != "2.0.0" &&
        definition.schema_version != capillary_bed_definition_schema_version) {
        invalid(core::ErrorCode::data_invalid,
                "Unsupported capillary-bed definition schema version");
    }
    std::unordered_set<std::string> source_ids;
    for (const auto& source : definition.sources) {
        if (!source_ids.insert(source.id).second) {
            invalid(core::ErrorCode::data_invalid,
                    "Capillary-bed source identifiers must be unique");
        }
    }
    if (definition.schema_version == capillary_bed_definition_schema_version &&
        !definition.qualification.has_value()) {
        invalid(core::ErrorCode::data_invalid,
                "Schema 3.0.0 capillary definitions require qualification metadata");
    }
    validate_qualification(definition, source_ids);
    static_cast<void>(CapillaryBed{definition.model});
}

} // namespace

CapillaryBedDefinition
load_capillary_bed_definition(const CapillaryBedDefinitionLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "capillary-bed schema");
    const auto schema = compile_schema(schema_document, request.schema_path);
    const auto definition_document = read_json(request.definition_path, "capillary-bed definition");
    validate_document(definition_document, schema, request.definition_path);
    auto definition = decode(definition_document);
    validate_definition(definition);
    return definition;
}

} // namespace mehlissa::models::capillary

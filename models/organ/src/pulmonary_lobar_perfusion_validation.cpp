// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/organ/pulmonary_lobar_perfusion_validation.hpp>

#include <mehlissa/core/error.hpp>
#include <mehlissa/models/organ/lung_model_factory.hpp>
#include <mehlissa/models/organ/pulmonary_parallel_beds.hpp>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace mehlissa::models::organ {
namespace {

using Json = jsoncons::json;
using CompiledSchema = jsoncons::jsonschema::json_schema<Json>;

constexpr double fraction_sum_tolerance = 0.005;

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
        invalid(core::ErrorCode::schema_invalid, "Invalid lobar-perfusion validation schema '" +
                                                     path.string() + "': " + error.what());
    }
}

void validate_document(const Json& document, const CompiledSchema& schema,
                       const std::filesystem::path& path) {
    try {
        schema.validate(document);
    } catch (const std::exception& error) {
        invalid(core::ErrorCode::data_invalid,
                "Lobar-perfusion validation does not satisfy its schema '" + path.string() +
                    "': " + error.what());
    }
}

[[nodiscard]] PulmonaryLobarSide decode_side(const std::string_view side) {
    return side == "right" ? PulmonaryLobarSide::right : PulmonaryLobarSide::left;
}

[[nodiscard]] PulmonaryLobarPerfusionValidationCase decode(const Json& document) {
    const auto& identity = document.at("validation");
    const auto& protocol = document.at("protocol");
    PulmonaryLobarPerfusionValidationCase result{
        document.at("schema_version").as<std::string>(),
        identity.at("id").as<std::string>(),
        identity.at("title").as<std::string>(),
        identity.at("model_definition_id").as<std::string>(),
        protocol.at("maximum_absolute_lobar_error_percentage_points").as<double>(),
        protocol.at("maximum_rmse_percentage_points").as<double>(),
        protocol.at("maximum_right_lung_error_percentage_points").as<double>(),
        {},
        {},
        {},
    };

    for (const auto& source : document.at("sources").array_range()) {
        result.sources.push_back({
            source.at("id").as<std::string>(),
            source.at("citation").as<std::string>(),
            source.at("url").as<std::string>(),
            source.at("license").as<std::string>(),
            source.at("measurement_method").as<std::string>(),
            source.at("cohort").as<std::string>(),
            source.at("sample_size").as<std::size_t>(),
            source.at("body_position").as<std::string>(),
            source.at("data_access").as<std::string>(),
            source.at("cohort_independence").as<std::string>(),
        });
    }
    for (const auto& series : document.at("series").array_range()) {
        PulmonaryLobarPerfusionReferenceSeries decoded_series{
            series.at("id").as<std::string>(),
            series.at("source_id").as<std::string>(),
            series.at("reconstruction").as<std::string>(),
            series.at("normalization_policy").as<std::string>(),
            {},
        };
        for (const auto& bed : series.at("perfusion_fractions").array_range()) {
            decoded_series.beds.push_back({
                bed.at("bed_id").as<std::string>(),
                decode_side(bed.at("side").as<std::string_view>()),
                bed.at("value").as<double>(),
            });
        }
        result.series.push_back(std::move(decoded_series));
    }
    for (const auto& limitation : document.at("limitations").array_range()) {
        result.limitations.push_back(limitation.as<std::string>());
    }
    return result;
}

void validate_semantics(const PulmonaryLobarPerfusionValidationCase& validation) {
    if (validation.schema_version != pulmonary_lobar_perfusion_validation_schema_version ||
        !std::isfinite(validation.maximum_absolute_lobar_error_percentage_points) ||
        !std::isfinite(validation.maximum_rmse_percentage_points) ||
        !std::isfinite(validation.maximum_right_lung_error_percentage_points) ||
        validation.maximum_absolute_lobar_error_percentage_points <= 0.0 ||
        validation.maximum_rmse_percentage_points <= 0.0 ||
        validation.maximum_right_lung_error_percentage_points <= 0.0) {
        invalid(core::ErrorCode::data_invalid,
                "Lobar-perfusion validation has an unsupported version or invalid threshold");
    }

    std::unordered_set<std::string> source_ids;
    for (const auto& source : validation.sources) {
        if (!source_ids.insert(source.id).second || source.sample_size == 0) {
            invalid(core::ErrorCode::data_invalid,
                    "Lobar-perfusion sources require unique IDs and positive sample sizes");
        }
    }

    std::unordered_set<std::string> series_ids;
    for (const auto& series : validation.series) {
        if (!series_ids.insert(series.id).second || !source_ids.contains(series.source_id) ||
            series.beds.size() != 5 ||
            series.normalization_policy != "renormalize_reported_rounded_fractions_to_unit_sum") {
            invalid(core::ErrorCode::data_invalid,
                    "Lobar-perfusion series require unique IDs, a declared source, five beds, "
                    "and the explicit rounding normalization policy");
        }

        std::unordered_set<std::string> bed_ids;
        auto fraction_sum = 0.0;
        for (const auto& bed : series.beds) {
            const auto side_matches_id =
                (bed.bed_id.starts_with("right-") && bed.side == PulmonaryLobarSide::right) ||
                (bed.bed_id.starts_with("left-") && bed.side == PulmonaryLobarSide::left);
            if (!bed_ids.insert(bed.bed_id).second || !std::isfinite(bed.reported_fraction) ||
                bed.reported_fraction <= 0.0 || !side_matches_id) {
                invalid(core::ErrorCode::data_invalid,
                        "Lobar-perfusion bed IDs must be unique, side-consistent, and fractions "
                        "positive");
            }
            fraction_sum += bed.reported_fraction;
        }
        if (std::abs(fraction_sum - 1.0) > fraction_sum_tolerance) {
            invalid(core::ErrorCode::data_invalid,
                    "Reported lobar-perfusion fractions exceed the allowed rounding tolerance");
        }
    }
}

[[nodiscard]] const PulmonaryParallelBedState&
find_bed(const std::vector<PulmonaryParallelBedState>& beds, const std::string& id) {
    const auto match = std::ranges::find_if(beds, [&id](const auto& bed) { return bed.id == id; });
    if (match == beds.end()) {
        invalid(core::ErrorCode::data_invalid,
                "The executable pulmonary model is missing validation bed '" + id + "'");
    }
    return *match;
}

void verify_source_independence(const PulmonaryLobarPerfusionValidationCase& validation,
                                const LungModelDefinition& definition) {
    for (const auto& source : validation.sources) {
        const auto reused = std::ranges::any_of(
            definition.sources, [&source](const auto& model) { return source.url == model.url; });
        if (reused) {
            invalid(core::ErrorCode::data_invalid,
                    "Lobar-perfusion validation reuses a model calibration source");
        }
    }
}

} // namespace

PulmonaryLobarPerfusionValidationCase load_pulmonary_lobar_perfusion_validation_case(
    const PulmonaryLobarPerfusionValidationLoadRequest& request) {
    const auto schema_document = read_json(request.schema_path, "lobar-perfusion schema");
    const auto validation_document = read_json(request.validation_path, "lobar-perfusion case");
    const auto schema = compile_schema(schema_document, request.schema_path);
    validate_document(validation_document, schema, request.validation_path);
    auto result = decode(validation_document);
    validate_semantics(result);
    return result;
}

PulmonaryLobarPerfusionValidationReport evaluate_pulmonary_lobar_perfusion_validation(
    const PulmonaryLobarPerfusionValidationCase& validation,
    const LungModelDefinition& model_definition) {
    validate_semantics(validation);
    if (validation.model_definition_id != model_definition.definition_id ||
        model_definition.model.variant != LungModelVariant::pulmonary_zero_dimensional) {
        invalid(core::ErrorCode::data_invalid,
                "Lobar-perfusion validation targets a different model definition or variant");
    }
    verify_source_independence(validation, model_definition);

    auto model = make_lung_model(model_definition.model);
    const auto* parallel = dynamic_cast<const PulmonaryParallelBedsModel*>(model.get());
    if (parallel == nullptr) {
        invalid(core::ErrorCode::data_invalid,
                "Lobar-perfusion validation requires an executable parallel-bed model");
    }
    const auto state = parallel->state();
    if (state.beds.size() != 5) {
        invalid(core::ErrorCode::data_invalid,
                "Lobar-perfusion validation requires exactly five executable beds");
    }

    PulmonaryLobarPerfusionValidationReport report{
        validation.validation_id,
        model_definition.definition_id,
        true,
        true,
        validation.series.size(),
        0,
        {},
    };

    for (const auto& series : validation.series) {
        auto reported_total = 0.0;
        for (const auto& bed : series.beds) {
            reported_total += bed.reported_fraction;
        }

        PulmonaryLobarPerfusionSeriesResult series_result{
            series.id, reported_total, 0.0, 0.0, 0.0, 0.0, 0.0, 0, true, {}};
        auto squared_error_sum = 0.0;
        for (const auto& reference : series.beds) {
            const auto& predicted_bed = find_bed(state.beds, reference.bed_id);
            const auto normalized_reference = reference.reported_fraction / reported_total;
            const auto predicted = predicted_bed.perfusion_fraction.si_value();
            const auto residual = 100.0 * (predicted - normalized_reference);
            const auto absolute_error = std::abs(residual);
            const auto accepted =
                absolute_error <= validation.maximum_absolute_lobar_error_percentage_points;
            series_result.maximum_absolute_error_percentage_points =
                std::max(series_result.maximum_absolute_error_percentage_points, absolute_error);
            squared_error_sum += residual * residual;
            series_result.accepted_bed_count += accepted ? 1U : 0U;
            series_result.beds.push_back({reference.bed_id, reference.side,
                                          reference.reported_fraction, normalized_reference,
                                          predicted, residual, absolute_error, accepted});
            if (reference.side == PulmonaryLobarSide::right) {
                series_result.reference_right_lung_fraction += normalized_reference;
                series_result.predicted_right_lung_fraction += predicted;
            }
        }
        series_result.root_mean_square_error_percentage_points =
            std::sqrt(squared_error_sum / static_cast<double>(series.beds.size()));
        series_result.right_lung_error_percentage_points =
            100.0 * std::abs(series_result.predicted_right_lung_fraction -
                             series_result.reference_right_lung_fraction);
        series_result.accepted = series_result.accepted_bed_count == series.beds.size() &&
                                 series_result.root_mean_square_error_percentage_points <=
                                     validation.maximum_rmse_percentage_points &&
                                 series_result.right_lung_error_percentage_points <=
                                     validation.maximum_right_lung_error_percentage_points;
        report.accepted_series_count += series_result.accepted ? 1U : 0U;
        report.all_series_pass = report.all_series_pass && series_result.accepted;
        report.series.push_back(std::move(series_result));
    }
    return report;
}

} // namespace mehlissa::models::organ

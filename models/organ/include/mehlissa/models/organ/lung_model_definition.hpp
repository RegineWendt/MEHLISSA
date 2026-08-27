// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP
#define MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP

#include <mehlissa/models/organ/lung_model_factory.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mehlissa::models::organ {

inline constexpr auto supported_lung_model_definition_schema_version = "1.0.0";

struct LungModelValidity final {
    std::string population;
    std::string physiological_state;
    std::string evidence_class;
    std::string description;
};

struct LungModelSource final {
    std::string id;
    std::string citation;
    std::string url;
    std::string license;
    std::string role;
};

struct ExternalPulmonaryDataReference final {
    std::string source_id;
    std::filesystem::path path;
    std::string sha256;
    std::string format;
    std::string coordinate_system;
    std::string length_unit;
    std::string flow_unit;
    std::vector<std::string> transformations;
};

struct LungModelDefinition final {
    std::string schema_version;
    std::string definition_id;
    std::string definition_version;
    std::string title;
    LungModelConfig model;
    LungModelValidity validity;
    std::vector<LungModelSource> sources;
    std::vector<std::string> limitations;
    std::optional<ExternalPulmonaryDataReference> external_data;
};

struct LungModelDefinitionLoadRequest final {
    std::filesystem::path definition_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] LungModelDefinition
load_lung_model_definition(const LungModelDefinitionLoadRequest& request);

} // namespace mehlissa::models::organ

#endif // MEHLISSA_MODELS_ORGAN_LUNG_MODEL_DEFINITION_HPP

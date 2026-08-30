// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_DISPOSITION_PROFILE_HPP
#define MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_DISPOSITION_PROFILE_HPP

#include <mehlissa/models/capillary/capillary_entity_observation_profile.hpp>
#include <mehlissa/models/coupling/entity_disposition.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::capillary {

inline constexpr auto capillary_entity_disposition_profile_schema_version = "1.0.0";

struct EntityDispositionTarget final {
    std::string model_id;
    std::string compartment_id;
};

struct CapillaryEntityDispositionProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string compatible_model_id;
    std::string compatible_observation_profile_id;
    std::string random_stream_name;
    std::string source_port_id;
    EntityDispositionTarget retention_target;
    EntityDispositionTarget adhesion_target;
    EntityDispositionTarget extravasation_target;
    CapillaryEntityObservationValidity validity;
    std::vector<CapillaryEntityObservationSource> sources;
    std::vector<std::string> limitations;
};

struct CapillaryEntityDispositionProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

void validate_capillary_entity_disposition_profile(
    const CapillaryEntityDispositionProfile& profile);

[[nodiscard]] CapillaryEntityDispositionProfile load_capillary_entity_disposition_profile(
    const CapillaryEntityDispositionProfileLoadRequest& request);

[[nodiscard]] const EntityDispositionTarget&
disposition_target(const CapillaryEntityDispositionProfile& profile,
                   coupling::EntityDispositionKind kind) noexcept;

} // namespace mehlissa::models::capillary

#endif // MEHLISSA_MODELS_CAPILLARY_CAPILLARY_ENTITY_DISPOSITION_PROFILE_HPP

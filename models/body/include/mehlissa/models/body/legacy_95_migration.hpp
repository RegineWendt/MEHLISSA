// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_LEGACY_95_MIGRATION_HPP
#define MEHLISSA_MODELS_BODY_LEGACY_95_MIGRATION_HPP

#include <mehlissa/models/body/vascular_graph.hpp>

#include <filesystem>

namespace mehlissa::models::body {

struct Legacy95MigrationRequest final {
    std::filesystem::path vasculature_path;
    std::filesystem::path transitions_path;
};

// Converts the released BVS/MEHLISSA 95-segment source data into the validated
// SI graph. The historical source files are never modified. The sole data
// correction is the documented supine jugular split at legacy vessel 9.
[[nodiscard]] VascularGraph migrate_legacy_95(const Legacy95MigrationRequest& request);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_LEGACY_95_MIGRATION_HPP

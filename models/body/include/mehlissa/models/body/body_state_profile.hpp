// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_BODY_STATE_PROFILE_HPP
#define MEHLISSA_MODELS_BODY_BODY_STATE_PROFILE_HPP

#include <mehlissa/models/body/vascular_graph.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace mehlissa::models::body {

inline constexpr auto supported_body_state_profile_schema_version = "1.0.0";

struct StateTransitionOverride final {
    std::string segment_id;
    std::vector<Transition> transitions;
};

struct BodyStateProfile final {
    std::string schema_version;
    std::string profile_id;
    std::string profile_version;
    std::string title;
    std::string compatible_model_id;
    std::string compatible_model_version;
    ModelValidity validity;
    std::string cardiac_output_anchor_segment_id;
    double cardiac_output_multiplier{};
    std::vector<StateTransitionOverride> transition_overrides;
    std::vector<DataSource> sources;
    std::vector<std::string> limitations;
};

struct BodyStateProfileLoadRequest final {
    std::filesystem::path profile_path;
    std::filesystem::path schema_path;
};

[[nodiscard]] BodyStateProfile load_body_state_profile(const BodyStateProfileLoadRequest& request);
[[nodiscard]] VascularGraph apply_body_state_profile(const VascularGraph& base_graph,
                                                     const BodyStateProfile& profile);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_BODY_STATE_PROFILE_HPP

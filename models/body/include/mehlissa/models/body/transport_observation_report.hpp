// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP
#define MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP

#include <filesystem>

namespace mehlissa::models::body {

class CompartmentTransport;

inline constexpr auto supported_transport_observation_report_schema_version = "1.0.0";

struct TransportObservationReportWriteRequest final {
    std::filesystem::path output_path;
    std::filesystem::path schema_path;
};

void write_transport_observation_report(const CompartmentTransport& transport,
                                        const TransportObservationReportWriteRequest& request);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP

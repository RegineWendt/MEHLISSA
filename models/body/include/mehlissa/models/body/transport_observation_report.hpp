// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP
#define MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP

#include <cstdint>
#include <filesystem>

namespace mehlissa::models::body {

class CompartmentTransport;

inline constexpr auto supported_transport_observation_report_schema_version = "1.0.0";

struct TransportObservationReportWriteRequest final {
    std::filesystem::path output_path;
    std::filesystem::path schema_path;
};

struct TransportObservationReportWriteMetrics final {
    std::uint64_t document_encoding_ns{};
    std::uint64_t schema_validation_ns{};
    std::uint64_t serialization_and_write_ns{};
    std::uint64_t output_bytes{};
};

TransportObservationReportWriteMetrics
write_transport_observation_report(const CompartmentTransport& transport,
                                   const TransportObservationReportWriteRequest& request);

} // namespace mehlissa::models::body

#endif // MEHLISSA_MODELS_BODY_TRANSPORT_OBSERVATION_REPORT_HPP

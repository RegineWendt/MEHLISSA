# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

execute_process(
    COMMAND "${REFERENCE_EXECUTABLE}" "${ACTUAL_RESULT}"
    RESULT_VARIABLE result
    ERROR_VARIABLE error_output
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Determinism reference failed with status ${result}: ${error_output}")
endif()

file(SHA256 "${EXPECTED_RESULT}" expected_sha256)
file(SHA256 "${ACTUAL_RESULT}" actual_sha256)
file(REMOVE "${ACTUAL_RESULT}")

if(NOT actual_sha256 STREQUAL expected_sha256)
    message(
        FATAL_ERROR
        "Determinism mismatch: expected ${expected_sha256}, got ${actual_sha256}"
    )
endif()

message(STATUS "Determinism reference SHA-256: ${actual_sha256}")

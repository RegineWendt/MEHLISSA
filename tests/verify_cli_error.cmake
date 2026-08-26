# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

execute_process(
    COMMAND
        "${MEHLISSA_EXECUTABLE}" validate
        --experiment "${INVALID_EXPERIMENT}"
        --schema "${EXPERIMENT_SCHEMA}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

if(NOT result EQUAL 3)
    message(FATAL_ERROR "Expected CLI exit status 3, got ${result}: ${error_output}")
endif()

if(NOT error_output MATCHES "\\[MEHLISSA-E2004\\]")
    message(FATAL_ERROR "Expected MEHLISSA-E2004 in CLI error output: ${error_output}")
endif()

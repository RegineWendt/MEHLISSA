# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

if(NOT DEFINED MEHLISSA_EXECUTABLE OR NOT DEFINED MEHLISSA_ROOT OR
   NOT DEFINED MEHLISSA_OUTPUT_ROOT)
    message(FATAL_ERROR "UX-4 test requires executable, repository root, and output root")
endif()

set(CAMPAIGN "${MEHLISSA_ROOT}/examples/campaigns/fp9-collector-count-v1.json")
file(REMOVE_RECURSE "${MEHLISSA_OUTPUT_ROOT}")

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" campaign validate --file "${CAMPAIGN}"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE VALIDATE_STATUS
    OUTPUT_VARIABLE VALIDATE_OUTPUT
    ERROR_VARIABLE VALIDATE_ERROR
)
if(NOT VALIDATE_STATUS EQUAL 0 OR NOT VALIDATE_OUTPUT MATCHES "6 derived runs")
    message(FATAL_ERROR "UX-4 campaign validation failed: ${VALIDATE_OUTPUT}${VALIDATE_ERROR}")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" campaign run --file "${CAMPAIGN}"
            --output "${MEHLISSA_OUTPUT_ROOT}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE RUN_STATUS
    OUTPUT_VARIABLE RUN_OUTPUT
    ERROR_VARIABLE RUN_ERROR
)
if(NOT RUN_STATUS EQUAL 0 OR NOT RUN_OUTPUT MATCHES "derived_runs=6")
    message(FATAL_ERROR "UX-4 campaign execution failed: ${RUN_OUTPUT}${RUN_ERROR}")
endif()

set(AGGREGATE "${MEHLISSA_OUTPUT_ROOT}/campaign-result.json")
set(CSV "${MEHLISSA_OUTPUT_ROOT}/campaign-results.csv")
if(NOT EXISTS "${AGGREGATE}" OR NOT EXISTS "${CSV}")
    message(FATAL_ERROR "UX-4 aggregate JSON or CSV is missing")
endif()
file(GLOB MANIFESTS "${MEHLISSA_OUTPUT_ROOT}/manifests/*.json")
file(GLOB RESULTS "${MEHLISSA_OUTPUT_ROOT}/runs/*/result.json")
list(LENGTH MANIFESTS MANIFEST_COUNT)
list(LENGTH RESULTS RESULT_COUNT)
if(NOT MANIFEST_COUNT EQUAL 6 OR NOT RESULT_COUNT EQUAL 6)
    message(FATAL_ERROR "Expected six immutable manifests and results, got ${MANIFEST_COUNT}/${RESULT_COUNT}")
endif()

file(READ "${AGGREGATE}" AGGREGATE_JSON)
string(JSON RUN_COUNT GET "${AGGREGATE_JSON}" run_count)
string(JSON RUN_ARRAY_LENGTH LENGTH "${AGGREGATE_JSON}" runs)
if(NOT RUN_COUNT EQUAL 6 OR NOT RUN_ARRAY_LENGTH EQUAL 6)
    message(FATAL_ERROR "UX-4 aggregate does not contain six runs")
endif()
string(JSON BASELINE_SEED GET "${AGGREGATE_JSON}" runs 4 seed)
string(JSON COMPARISON_SEED GET "${AGGREGATE_JSON}" runs 5 seed)
string(JSON BASELINE_ROLE GET "${AGGREGATE_JSON}" runs 4 role)
string(JSON COMPARISON_ROLE GET "${AGGREGATE_JSON}" runs 5 role)
if(NOT BASELINE_SEED EQUAL COMPARISON_SEED OR NOT BASELINE_ROLE STREQUAL "baseline" OR
   NOT COMPARISON_ROLE STREQUAL "comparison")
    message(FATAL_ERROR "UX-4 paired comparison does not preserve a shared seed and explicit roles")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" campaign run --file "${CAMPAIGN}"
            --output "${MEHLISSA_OUTPUT_ROOT}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE OVERWRITE_STATUS
    OUTPUT_VARIABLE OVERWRITE_OUTPUT
    ERROR_VARIABLE OVERWRITE_ERROR
)
if(OVERWRITE_STATUS EQUAL 0 OR NOT OVERWRITE_ERROR MATCHES "MEHLISSA-E3001")
    message(FATAL_ERROR "UX-4 did not reject an existing campaign directory")
endif()

file(READ "${CAMPAIGN}" UNSAFE_CAMPAIGN)
string(REPLACE "run.collector_count" "run.master_seed" UNSAFE_CAMPAIGN "${UNSAFE_CAMPAIGN}")
set(UNSAFE_FILE "${MEHLISSA_OUTPUT_ROOT}.unsafe.json")
file(WRITE "${UNSAFE_FILE}" "${UNSAFE_CAMPAIGN}")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" campaign validate --file "${UNSAFE_FILE}"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE UNSAFE_STATUS
    OUTPUT_VARIABLE UNSAFE_OUTPUT
    ERROR_VARIABLE UNSAFE_ERROR
)
file(REMOVE "${UNSAFE_FILE}")
if(UNSAFE_STATUS EQUAL 0 OR NOT UNSAFE_ERROR MATCHES "MEHLISSA-E2")
    message(FATAL_ERROR "UX-4 did not reject an unapproved parameter override")
endif()

message(STATUS "UX-4 campaign workflow passed")

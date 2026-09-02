# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

if(NOT DEFINED MEHLISSA_EXECUTABLE OR NOT DEFINED MEHLISSA_ROOT OR
   NOT DEFINED MEHLISSA_OUTPUT_ROOT)
    message(FATAL_ERROR "UX-2 CLI test requires executable, repository root, and output root")
endif()

file(REMOVE_RECURSE "${MEHLISSA_OUTPUT_ROOT}")
file(MAKE_DIRECTORY "${MEHLISSA_OUTPUT_ROOT}")

function(require_success label)
    set(options)
    set(one_value OUTPUT ERROR RESULT)
    cmake_parse_arguments(ARG "${options}" "${one_value}" "" ${ARGN})
    if(NOT ARG_RESULT EQUAL 0)
        message(FATAL_ERROR "${label} failed (${ARG_RESULT}):\n${ARG_OUTPUT}\n${ARG_ERROR}")
    endif()
endfunction()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" model list --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE list_result
    OUTPUT_VARIABLE list_output
    ERROR_VARIABLE list_error
)
require_success("model list" RESULT "${list_result}" OUTPUT "${list_output}" ERROR "${list_error}")
if(NOT list_output MATCHES "organ.pulmonary-zero-dimensional" OR
   NOT list_output MATCHES "model_count=5")
    message(FATAL_ERROR "model list omitted an implemented family or returned the wrong count")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" model list --layer organ
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE layer_result
    OUTPUT_VARIABLE layer_output
    ERROR_VARIABLE layer_error
)
require_success("model list --layer" RESULT "${layer_result}" OUTPUT "${layer_output}"
                ERROR "${layer_error}")
if(NOT layer_output MATCHES "model_count=1" OR layer_output MATCHES "cell.receptor-response")
    message(FATAL_ERROR "model layer filter did not isolate the organ family")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" model describe --id organ.pulmonary-zero-dimensional
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE describe_result
    OUTPUT_VARIABLE describe_output
    ERROR_VARIABLE describe_error
)
require_success("model describe" RESULT "${describe_result}" OUTPUT "${describe_output}"
                ERROR "${describe_error}")
if(NOT describe_output MATCHES "Configurable parameters:" OR
   NOT describe_output MATCHES "hemodynamics.baseline_cardiac_output" OR
   NOT describe_output MATCHES "Limitations:" OR
   NOT describe_output MATCHES "Evidence:")
    message(FATAL_ERROR "model describe omitted parameters, evidence, or limitations")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" example list --model organ.pulmonary-zero-dimensional
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE examples_result
    OUTPUT_VARIABLE examples_output
    ERROR_VARIABLE examples_error
)
require_success("example list --model" RESULT "${examples_result}" OUTPUT "${examples_output}"
                ERROR "${examples_error}")
if(NOT examples_output MATCHES "lung.five-lobe-reference" OR
   NOT examples_output MATCHES "scenario.fp9-complete")
    message(FATAL_ERROR "example list did not expose direct and integrated lung starters")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" model describe --id does.not-exist
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE unknown_result
    OUTPUT_VARIABLE unknown_output
    ERROR_VARIABLE unknown_error
)
if(unknown_result EQUAL 0 OR NOT unknown_error MATCHES "MEHLISSA-E2005")
    message(FATAL_ERROR "unknown model id did not fail through the stable data error contract")
endif()

file(READ "${MEHLISSA_ROOT}/data/catalog/model-catalog-v1.json" escaped_catalog)
string(REPLACE
       "data/body-models/bvs95-dissertation-rest-v1.json"
       "../CMakeLists.txt"
       escaped_catalog
       "${escaped_catalog}")
set(escaped_catalog_path "${MEHLISSA_OUTPUT_ROOT}/escaped-catalog.json")
file(WRITE "${escaped_catalog_path}" "${escaped_catalog}")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" model list --catalog "${escaped_catalog_path}"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE escaped_result
    OUTPUT_VARIABLE escaped_output
    ERROR_VARIABLE escaped_error
)
if(escaped_result EQUAL 0 OR NOT escaped_error MATCHES "MEHLISSA-E2005" OR
   NOT escaped_error MATCHES "must stay inside the repository")
    message(FATAL_ERROR "catalog path escape did not fail through the stable data error contract")
endif()

file(REMOVE_RECURSE "${MEHLISSA_OUTPUT_ROOT}")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" example copy --id scenario.fp9-complete
            --output "${MEHLISSA_OUTPUT_ROOT}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE copy_result
    OUTPUT_VARIABLE copy_output
    ERROR_VARIABLE copy_error
)
require_success("example copy" RESULT "${copy_result}" OUTPUT "${copy_output}" ERROR "${copy_error}")
set(copied_profile "${MEHLISSA_OUTPUT_ROOT}/fp9-lung-level-a-v1.json")
if(NOT EXISTS "${copied_profile}" OR NOT EXISTS "${copied_profile}.license")
    message(FATAL_ERROR "example copy omitted the configuration or its license sidecar")
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files
            "${MEHLISSA_ROOT}/examples/scenarios/fp9-lung-level-a-v1.json" "${copied_profile}"
    RESULT_VARIABLE compare_result
)
if(NOT compare_result EQUAL 0)
    message(FATAL_ERROR "copied starter configuration differs from the catalog source")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" example copy --id scenario.fp9-complete
            --output "${MEHLISSA_OUTPUT_ROOT}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE overwrite_result
    OUTPUT_VARIABLE overwrite_output
    ERROR_VARIABLE overwrite_error
)
if(overwrite_result EQUAL 0 OR NOT overwrite_error MATCHES "MEHLISSA-E3001")
    message(FATAL_ERROR "example copy did not fail safely for an existing destination")
endif()

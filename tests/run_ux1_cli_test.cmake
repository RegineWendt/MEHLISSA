# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

if(NOT DEFINED MEHLISSA_EXECUTABLE OR NOT DEFINED MEHLISSA_ROOT OR
   NOT DEFINED MEHLISSA_OUTPUT_ROOT)
    message(FATAL_ERROR "UX-1 CLI test requires executable, repository root, and output root")
endif()

function(require_success label)
    set(options)
    set(one_value OUTPUT ERROR RESULT)
    cmake_parse_arguments(ARG "${options}" "${one_value}" "" ${ARGN})
    if(NOT ARG_RESULT EQUAL 0)
        message(FATAL_ERROR "${label} failed (${ARG_RESULT}):\n${ARG_OUTPUT}\n${ARG_ERROR}")
    endif()
endfunction()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario run --help
    RESULT_VARIABLE help_result
    OUTPUT_VARIABLE help_output
    ERROR_VARIABLE help_error
)
require_success("scenario run --help" RESULT "${help_result}" OUTPUT "${help_output}"
                ERROR "${help_error}")
if(NOT help_error MATCHES "mehlissa scenario run")
    message(FATAL_ERROR "scenario run --help did not show the usability commands")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario list --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE list_result
    OUTPUT_VARIABLE list_output
    ERROR_VARIABLE list_error
)
require_success("scenario list" RESULT "${list_result}" OUTPUT "${list_output}" ERROR "${list_error}")
if(NOT list_output MATCHES "fp9-lung-level-a-v1")
    message(FATAL_ERROR "scenario list did not expose the FP9 lung scenario")
endif()

set(profile "${MEHLISSA_ROOT}/examples/scenarios/fp9-lung-level-a-v1.json")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario validate --file "${profile}"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE validate_result
    OUTPUT_VARIABLE validate_output
    ERROR_VARIABLE validate_error
)
require_success("scenario validate" RESULT "${validate_result}" OUTPUT "${validate_output}"
                ERROR "${validate_error}")
if(NOT validate_output MATCHES "13 artifacts")
    message(FATAL_ERROR "scenario validate did not report the complete artifact set")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario validate
            --file "${MEHLISSA_ROOT}/examples/scenarios/fp9-lung-level-a-cluster-v1.json"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE invalid_result
    OUTPUT_VARIABLE invalid_output
    ERROR_VARIABLE invalid_error
)
if(invalid_result EQUAL 0 OR NOT invalid_error MATCHES "MEHLISSA-E2005")
    message(FATAL_ERROR "scenario validate did not reject a non-scenario JSON artifact predictably")
endif()

file(REMOVE_RECURSE "${MEHLISSA_OUTPUT_ROOT}")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario run --file "${profile}"
            --repository-root "${MEHLISSA_ROOT}" --output "${MEHLISSA_OUTPUT_ROOT}"
    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE run_output
    ERROR_VARIABLE run_error
)
require_success("scenario run" RESULT "${run_result}" OUTPUT "${run_output}" ERROR "${run_error}")
if(NOT run_output MATCHES "scenario_status=completed")
    message(FATAL_ERROR "scenario run did not report completion")
endif()

file(GLOB run_directories LIST_DIRECTORIES true "${MEHLISSA_OUTPUT_ROOT}/*")
list(LENGTH run_directories run_directory_count)
if(NOT run_directory_count EQUAL 1)
    message(FATAL_ERROR "scenario run must create exactly one unique run directory")
endif()
list(GET run_directories 0 run_directory)
foreach(name result.json provenance.json run.log.jsonl summary.txt)
    if(NOT EXISTS "${run_directory}/${name}")
        message(FATAL_ERROR "scenario run did not create ${name}")
    endif()
endforeach()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" result summarize --file "${run_directory}/result.json"
            --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE summary_result
    OUTPUT_VARIABLE summary_output
    ERROR_VARIABLE summary_error
)
require_success("result summarize" RESULT "${summary_result}" OUTPUT "${summary_output}"
                ERROR "${summary_error}")
if(NOT summary_output MATCHES "Molecular target detected: yes" OR
   NOT summary_output MATCHES "Clinical validation claim: no")
    message(FATAL_ERROR "result summarize omitted the outcome or validity qualification")
endif()

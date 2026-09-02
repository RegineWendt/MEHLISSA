# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

if(NOT DEFINED MEHLISSA_EXECUTABLE OR NOT DEFINED MEHLISSA_ROOT OR
   NOT DEFINED MEHLISSA_OUTPUT_ROOT)
    message(FATAL_ERROR "UX-3 CLI test requires executable, repository root, and output root")
endif()

function(require_success label)
    set(options)
    set(one_value OUTPUT ERROR RESULT)
    cmake_parse_arguments(ARG "${options}" "${one_value}" "" ${ARGN})
    if(NOT ARG_RESULT EQUAL 0)
        message(FATAL_ERROR "${label} failed (${ARG_RESULT}):\n${ARG_OUTPUT}\n${ARG_ERROR}")
    endif()
endfunction()

file(REMOVE_RECURSE "${MEHLISSA_OUTPUT_ROOT}")
file(MAKE_DIRECTORY "${MEHLISSA_OUTPUT_ROOT}")
set(run_root "${MEHLISSA_OUTPUT_ROOT}/runs")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" scenario run
            --file "${MEHLISSA_ROOT}/examples/scenarios/fp9-lung-level-a-v1.json"
            --output "${run_root}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE run_result OUTPUT_VARIABLE run_output ERROR_VARIABLE run_error
)
require_success("scenario run for UX-3" RESULT "${run_result}" OUTPUT "${run_output}"
                ERROR "${run_error}")
file(GLOB run_directories LIST_DIRECTORIES true "${run_root}/*")
list(GET run_directories 0 run_directory)

set(report_directory "${MEHLISSA_OUTPUT_ROOT}/report")
execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" result report --file "${run_directory}/result.json"
            --output "${report_directory}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE report_result OUTPUT_VARIABLE report_output ERROR_VARIABLE report_error
)
require_success("result report" RESULT "${report_result}" OUTPUT "${report_output}"
                ERROR "${report_error}")
foreach(name result.json summary.txt overview.csv runtime-stages.csv analysis-cases.csv report.html)
    if(NOT EXISTS "${report_directory}/${name}")
        message(FATAL_ERROR "result report omitted ${name}")
    endif()
endforeach()
file(READ "${report_directory}/report.html" html)
if(NOT html MATCHES "Research software only" OR NOT html MATCHES "no clinical validation claim" OR
   NOT html MATCHES "href=\"result.json\"" OR NOT html MATCHES "Evidence and reproducibility")
    message(FATAL_ERROR "HTML report omitted the non-claim, evidence, or machine-result link")
endif()
file(READ "${report_directory}/overview.csv" overview)
file(READ "${report_directory}/runtime-stages.csv" stages)
file(READ "${report_directory}/analysis-cases.csv" cases)
if(NOT overview MATCHES "metric,value,unit" OR NOT overview MATCHES "clinical_validation_claim,false" OR
   NOT stages MATCHES "ordinal,stage,time_ns" OR NOT cases MATCHES "case_id,target_present,detected")
    message(FATAL_ERROR "stable CSV exports omitted required headers or the clinical non-claim")
endif()

execute_process(
    COMMAND "${MEHLISSA_EXECUTABLE}" result report --file "${run_directory}/result.json"
            --output "${report_directory}" --repository-root "${MEHLISSA_ROOT}"
    RESULT_VARIABLE overwrite_result OUTPUT_VARIABLE overwrite_output ERROR_VARIABLE overwrite_error
)
if(overwrite_result EQUAL 0 OR NOT overwrite_error MATCHES "MEHLISSA-E3001")
    message(FATAL_ERROR "result report did not refuse to overwrite an existing bundle")
endif()

# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_executable(
    mehlissa
    apps_main.cpp
    campaign_cli.cpp
    discovery_cli.cpp
    reporting_cli.cpp
    scenario_cli.cpp
)
target_link_libraries(
    mehlissa
    PRIVATE
        MEHLISSA::body_model
        MEHLISSA::core
        MEHLISSA::experiment
        MEHLISSA::fingerprinting_scenario
        jsoncons::jsoncons
)
mehlissa_configure_target(mehlissa)

add_test(
    NAME mehlissa_cli_minimal_experiment
    COMMAND
        mehlissa run
        --experiment ${PROJECT_SOURCE_DIR}/examples/experiments/minimal.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/experiment/1.0.0.schema.json
        --checkpoint-schema ${PROJECT_SOURCE_DIR}/data/schemas/checkpoint/1.0.0.schema.json
)
set_tests_properties(
    mehlissa_cli_minimal_experiment
    PROPERTIES
        PASS_REGULAR_EXPRESSION "experiment_id=minimal-deterministic-run"
)

add_test(
    NAME mehlissa_cli_rejects_invalid_experiment
    COMMAND
        mehlissa validate
        --experiment ${PROJECT_SOURCE_DIR}/tests/data/experiments/missing-seed.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/experiment/1.0.0.schema.json
)
set_tests_properties(mehlissa_cli_rejects_invalid_experiment PROPERTIES WILL_FAIL TRUE)

add_test(
    NAME mehlissa_cli_validates_body_model
    COMMAND
        mehlissa validate-body
        --model ${PROJECT_SOURCE_DIR}/examples/body-models/synthetic-branching-circuit.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/vascular-graph/1.0.0.schema.json
)

add_test(
    NAME mehlissa_cli_migrates_legacy_95
    COMMAND
        mehlissa migrate-legacy-95
        --vasculature ${PROJECT_SOURCE_DIR}/mehlissa2.0/data/95_vasculature.csv
        --transitions ${PROJECT_SOURCE_DIR}/mehlissa2.0/data/95_transitions.csv
        --output ${CMAKE_CURRENT_BINARY_DIR}/bvs95-dissertation-rest.generated.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/vascular-graph/1.0.0.schema.json
)
set_tests_properties(
    mehlissa_cli_migrates_legacy_95
    PROPERTIES PASS_REGULAR_EXPRESSION "bvs95-dissertation-rest.*95 segments"
)

add_test(
    NAME mehlissa_cli_runs_bvs_reference
    COMMAND
        mehlissa reference-bvs
        --model ${PROJECT_SOURCE_DIR}/data/body-models/bvs95-dissertation-rest-v1.json
        --output ${CMAKE_CURRENT_BINARY_DIR}/bvs95-m2.4-reference.generated.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/vascular-graph/1.0.0.schema.json
        --report-schema ${PROJECT_SOURCE_DIR}/data/schemas/bvs-reference-report/1.0.0.schema.json
)
set_tests_properties(
    mehlissa_cli_runs_bvs_reference
    PROPERTIES PASS_REGULAR_EXPRESSION "M2.4 BVS reference passed"
)

add_test(
    NAME mehlissa_cli_applies_body_state_without_rebuild
    COMMAND
        mehlissa apply-body-state
        --model ${PROJECT_SOURCE_DIR}/data/body-models/bvs95-dissertation-rest-v1.json
        --profile ${PROJECT_SOURCE_DIR}/data/body-states/bvs95-supine-cycle-exercise-1.9x-v1.json
        --output ${CMAKE_CURRENT_BINARY_DIR}/bvs95-exercise.generated.json
        --schema ${PROJECT_SOURCE_DIR}/data/schemas/vascular-graph/1.0.0.schema.json
        --profile-schema ${PROJECT_SOURCE_DIR}/data/schemas/body-state-profile/1.0.0.schema.json
)
set_tests_properties(
    mehlissa_cli_applies_body_state_without_rebuild
    PROPERTIES PASS_REGULAR_EXPRESSION "supine-cycle-exercise-1.9x x 1.900000.*95 segments"
)
set_tests_properties(
    mehlissa_cli_validates_body_model
    PROPERTIES PASS_REGULAR_EXPRESSION "synthetic-branching-circuit.*4 segments"
)

add_test(
    NAME mehlissa_cli_ux1_workflow
    COMMAND
        ${CMAKE_COMMAND}
        -DMEHLISSA_EXECUTABLE=$<TARGET_FILE:mehlissa>
        -DMEHLISSA_ROOT=${PROJECT_SOURCE_DIR}
        -DMEHLISSA_OUTPUT_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ux1-runs
        -P ${PROJECT_SOURCE_DIR}/tests/run_ux1_cli_test.cmake
)
set_tests_properties(mehlissa_cli_ux1_workflow PROPERTIES LABELS "UX-1;M7;cli")

add_test(
    NAME mehlissa_cli_ux2_discovery
    COMMAND
        ${CMAKE_COMMAND}
        -DMEHLISSA_EXECUTABLE=$<TARGET_FILE:mehlissa>
        -DMEHLISSA_ROOT=${PROJECT_SOURCE_DIR}
        -DMEHLISSA_OUTPUT_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ux2-example
        -P ${PROJECT_SOURCE_DIR}/tests/run_ux2_cli_test.cmake
)
set_tests_properties(mehlissa_cli_ux2_discovery PROPERTIES LABELS "UX-2;cli")

add_test(
    NAME mehlissa_cli_ux3_reporting
    COMMAND
        ${CMAKE_COMMAND}
        -DMEHLISSA_EXECUTABLE=$<TARGET_FILE:mehlissa>
        -DMEHLISSA_ROOT=${PROJECT_SOURCE_DIR}
        -DMEHLISSA_OUTPUT_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ux3-reporting
        -P ${PROJECT_SOURCE_DIR}/tests/run_ux3_cli_test.cmake
)
set_tests_properties(mehlissa_cli_ux3_reporting PROPERTIES LABELS "UX-3;cli")

add_test(
    NAME mehlissa_cli_ux4_campaigns
    COMMAND
        ${CMAKE_COMMAND}
        -DMEHLISSA_EXECUTABLE=$<TARGET_FILE:mehlissa>
        -DMEHLISSA_ROOT=${PROJECT_SOURCE_DIR}
        -DMEHLISSA_OUTPUT_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ux4-campaign
        -P ${PROJECT_SOURCE_DIR}/tests/run_ux4_cli_test.cmake
)
set_tests_properties(mehlissa_cli_ux4_campaigns PROPERTIES LABELS "UX-4;cli")

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME mehlissa_python_ux5_api
        COMMAND
            ${CMAKE_COMMAND} -E env "PYTHONPATH=${PROJECT_SOURCE_DIR}/python"
            ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/test_python_api.py
            --executable $<TARGET_FILE:mehlissa>
            --root ${PROJECT_SOURCE_DIR}
            --output ${CMAKE_CURRENT_BINARY_DIR}/ux5-python
    )
    set_tests_properties(mehlissa_python_ux5_api PROPERTIES LABELS "UX-5;python;cli")

    add_test(
        NAME mehlissa_python_ux6_1_workbench
        COMMAND
            ${CMAKE_COMMAND} -E env "PYTHONPATH=${PROJECT_SOURCE_DIR}/python"
            ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/test_workbench.py
            --executable $<TARGET_FILE:mehlissa>
            --root ${PROJECT_SOURCE_DIR}
    )
    set_tests_properties(
        mehlissa_python_ux6_1_workbench
        PROPERTIES LABELS "UX-6.1;python;workbench"
    )
else()
    message(WARNING "Python 3 was not found; the UX-5 and UX-6.1 Python tests are disabled")
endif()

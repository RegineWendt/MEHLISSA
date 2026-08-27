# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_executable(mehlissa apps_main.cpp)
target_link_libraries(
    mehlissa
    PRIVATE
        MEHLISSA::body_model
        MEHLISSA::core
        MEHLISSA::experiment
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
set_tests_properties(
    mehlissa_cli_validates_body_model
    PROPERTIES PASS_REGULAR_EXPRESSION "synthetic-branching-circuit.*4 segments"
)

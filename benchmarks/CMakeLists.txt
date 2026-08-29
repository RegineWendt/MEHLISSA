# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_executable(mehlissa_body_transport_benchmark body_transport_benchmark.cpp)
target_link_libraries(
    mehlissa_body_transport_benchmark
    PRIVATE
        MEHLISSA::body_model
        MEHLISSA::core
        MEHLISSA::experiment
        jsoncons::jsoncons
)
target_compile_definitions(mehlissa_body_transport_benchmark PRIVATE JSONCONS_NO_DEPRECATED)
if(WIN32)
    target_link_libraries(mehlissa_body_transport_benchmark PRIVATE advapi32 psapi)
endif()
mehlissa_configure_target(mehlissa_body_transport_benchmark)

if(BUILD_TESTING)
    add_test(
        NAME mehlissa_body_transport_benchmark_smoke
        COMMAND
            mehlissa_body_transport_benchmark
            --manifest ${PROJECT_SOURCE_DIR}/examples/benchmarks/synthetic-o2-smoke.json
            --manifest-schema ${PROJECT_SOURCE_DIR}/data/schemas/body-transport-benchmark/1.0.0.schema.json
            --result-schema ${PROJECT_SOURCE_DIR}/data/schemas/body-transport-benchmark-report/1.0.0.schema.json
            --observation-schema ${PROJECT_SOURCE_DIR}/data/schemas/transport-observation-report/1.0.0.schema.json
            --output ${CMAKE_CURRENT_BINARY_DIR}/synthetic-o2-smoke.result.json
            --observation-output ${CMAKE_CURRENT_BINARY_DIR}/synthetic-o2-smoke.observation.json
    )
    set_tests_properties(
        mehlissa_body_transport_benchmark_smoke
        PROPERTIES PASS_REGULAR_EXPRESSION "benchmark_id=synthetic-o2-smoke policy=O2 population=64"
    )

    add_test(
        NAME mehlissa_body_transport_benchmark_rejects_invalid_manifest
        COMMAND
            mehlissa_body_transport_benchmark
            --manifest ${CMAKE_CURRENT_SOURCE_DIR}/invalid-manifest.json
            --manifest-schema ${PROJECT_SOURCE_DIR}/data/schemas/body-transport-benchmark/1.0.0.schema.json
            --result-schema ${PROJECT_SOURCE_DIR}/data/schemas/body-transport-benchmark-report/1.0.0.schema.json
            --observation-schema ${PROJECT_SOURCE_DIR}/data/schemas/transport-observation-report/1.0.0.schema.json
            --output ${CMAKE_CURRENT_BINARY_DIR}/invalid.result.json
            --observation-output ${CMAKE_CURRENT_BINARY_DIR}/invalid.observation.json
    )
    set_tests_properties(
        mehlissa_body_transport_benchmark_rejects_invalid_manifest
        PROPERTIES WILL_FAIL TRUE
    )

    find_package(Python3 3.11 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME mehlissa_rq4_campaign_orchestration_tests
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_rq4_campaign.py
        )
        set_tests_properties(
            mehlissa_rq4_campaign_orchestration_tests
            PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()

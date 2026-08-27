# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_executable(mehlissa_core_smoke_tests smoke_tests.cpp)
target_link_libraries(mehlissa_core_smoke_tests PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa_core_smoke_tests)
add_test(NAME mehlissa_core_smoke_tests COMMAND mehlissa_core_smoke_tests)

add_executable(mehlissa_determinism_reference determinism_reference.cpp)
target_link_libraries(mehlissa_determinism_reference PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa_determinism_reference)
add_test(
    NAME mehlissa_cross_platform_determinism
    COMMAND
        ${CMAKE_COMMAND}
        -DREFERENCE_EXECUTABLE=$<TARGET_FILE:mehlissa_determinism_reference>
        -DACTUAL_RESULT=${CMAKE_CURRENT_BINARY_DIR}/determinism-reference.actual.json
        -DEXPECTED_RESULT=${CMAKE_CURRENT_SOURCE_DIR}/data/determinism/reference-v1.json
        -P ${CMAKE_CURRENT_SOURCE_DIR}/verify_determinism.cmake
)

if(MEHLISSA_ENABLE_CATCH2_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(
        mehlissa_core_tests
        component_host_tests.cpp
        error_tests.cpp
        position_tests.cpp
        quantity_tests.cpp
        random_stream_tests.cpp
        simulation_clock_tests.cpp
        simulation_context_tests.cpp
    )
    target_link_libraries(
        mehlissa_core_tests
        PRIVATE
            MEHLISSA::core
            Catch2::Catch2WithMain
    )
    mehlissa_configure_target(mehlissa_core_tests)

    include(Catch)
    catch_discover_tests(mehlissa_core_tests DISCOVERY_MODE PRE_TEST)

    if(MEHLISSA_ENABLE_EXPERIMENT_IO)
        add_executable(
            mehlissa_experiment_tests
            checkpoint_manifest_tests.cpp
            experiment_manifest_tests.cpp
            provenance_tests.cpp
            run_log_tests.cpp
        )
        target_link_libraries(
            mehlissa_experiment_tests
            PRIVATE
                MEHLISSA::experiment
                Catch2::Catch2WithMain
        )
        target_compile_definitions(
            mehlissa_experiment_tests
            PRIVATE MEHLISSA_TEST_ROOT="${PROJECT_SOURCE_DIR}"
        )
        mehlissa_configure_target(mehlissa_experiment_tests)
        catch_discover_tests(mehlissa_experiment_tests DISCOVERY_MODE PRE_TEST)
    endif()

    if(MEHLISSA_BUILD_BODY_MODELS)
        add_executable(
            mehlissa_body_model_tests
            body_state_profile_tests.cpp
            bvs_reference_tests.cpp
            compartment_transport_tests.cpp
            legacy_95_migration_tests.cpp
            vascular_graph_tests.cpp
        )
        target_link_libraries(
            mehlissa_body_model_tests
            PRIVATE
                MEHLISSA::body_model
                Catch2::Catch2WithMain
        )
        target_compile_definitions(
            mehlissa_body_model_tests
            PRIVATE MEHLISSA_TEST_ROOT="${PROJECT_SOURCE_DIR}"
        )
        mehlissa_configure_target(mehlissa_body_model_tests)
        catch_discover_tests(mehlissa_body_model_tests DISCOVERY_MODE PRE_TEST)
    endif()
endif()

if(MEHLISSA_BUILD_APPS AND MEHLISSA_ENABLE_EXPERIMENT_IO)
    add_test(
        NAME mehlissa_cli_reports_structured_error
        COMMAND
            ${CMAKE_COMMAND}
            -DMEHLISSA_EXECUTABLE=$<TARGET_FILE:mehlissa>
            -DINVALID_EXPERIMENT=${PROJECT_SOURCE_DIR}/tests/data/experiments/missing-seed.json
            -DEXPERIMENT_SCHEMA=${PROJECT_SOURCE_DIR}/data/schemas/experiment/1.0.0.schema.json
            -P ${CMAKE_CURRENT_SOURCE_DIR}/verify_cli_error.cmake
    )
endif()

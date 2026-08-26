# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_executable(mehlissa_core_smoke_tests smoke_tests.cpp)
target_link_libraries(mehlissa_core_smoke_tests PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa_core_smoke_tests)
add_test(NAME mehlissa_core_smoke_tests COMMAND mehlissa_core_smoke_tests)

if(MEHLISSA_ENABLE_CATCH2_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(
        mehlissa_core_tests
        position_tests.cpp
        random_stream_tests.cpp
        simulation_clock_tests.cpp
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
        add_executable(mehlissa_experiment_tests experiment_manifest_tests.cpp)
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
endif()

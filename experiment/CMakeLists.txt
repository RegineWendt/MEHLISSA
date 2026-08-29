# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)
find_package(Git QUIET)
find_path(PICOSHA2_INCLUDE_DIR NAMES picosha2.h REQUIRED)

set(MEHLISSA_GIT_COMMIT "unknown")
set(MEHLISSA_GIT_DIRTY 0)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE MEHLISSA_GIT_RESULT
        OUTPUT_VARIABLE MEHLISSA_GIT_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(MEHLISSA_GIT_RESULT EQUAL 0)
        set(MEHLISSA_GIT_COMMIT "${MEHLISSA_GIT_OUTPUT}")
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --porcelain --untracked-files=normal
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE MEHLISSA_STATUS_RESULT
        OUTPUT_VARIABLE MEHLISSA_STATUS_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(MEHLISSA_STATUS_RESULT EQUAL 0 AND NOT MEHLISSA_STATUS_OUTPUT STREQUAL "")
        set(MEHLISSA_GIT_DIRTY 1)
    endif()
endif()

add_library(
    mehlissa_experiment
    src/checkpoint_manifest.cpp
    src/experiment_manifest.cpp
    src/fingerprint_timer_baseline.cpp
    src/provenance.cpp
    src/run_log.cpp
)
add_library(MEHLISSA::experiment ALIAS mehlissa_experiment)

target_include_directories(
    mehlissa_experiment
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_include_directories(mehlissa_experiment SYSTEM PRIVATE ${PICOSHA2_INCLUDE_DIR})
target_link_libraries(
    mehlissa_experiment
    PUBLIC MEHLISSA::core
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(
    mehlissa_experiment
    PRIVATE
        JSONCONS_NO_DEPRECATED
        MEHLISSA_BUILD_TYPE="$<CONFIG>"
        MEHLISSA_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
        MEHLISSA_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
        MEHLISSA_GIT_COMMIT="${MEHLISSA_GIT_COMMIT}"
        MEHLISSA_GIT_DIRTY=${MEHLISSA_GIT_DIRTY}
        MEHLISSA_PLATFORM_ARCHITECTURE="${CMAKE_SYSTEM_PROCESSOR}"
        MEHLISSA_PLATFORM_OS="${CMAKE_SYSTEM_NAME}"
        MEHLISSA_SOFTWARE_VERSION="${PROJECT_VERSION}"
)

mehlissa_configure_target(mehlissa_experiment)

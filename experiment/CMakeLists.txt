# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(mehlissa_experiment src/experiment_manifest.cpp)
add_library(MEHLISSA::experiment ALIAS mehlissa_experiment)

target_include_directories(
    mehlissa_experiment
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_experiment
    PUBLIC MEHLISSA::core
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_experiment PRIVATE JSONCONS_NO_DEPRECATED)

mehlissa_configure_target(mehlissa_experiment)

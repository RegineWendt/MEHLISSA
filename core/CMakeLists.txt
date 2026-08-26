# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_library(
    mehlissa_core
    src/component_host.cpp
    src/position.cpp
    src/random_stream.cpp
    src/simulation_clock.cpp
    src/simulation_context.cpp
)
add_library(MEHLISSA::core ALIAS mehlissa_core)

target_include_directories(
    mehlissa_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

mehlissa_configure_target(mehlissa_core)

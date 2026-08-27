# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_body_model
    src/bvs_reference.cpp
    src/compartment_transport.cpp
    src/legacy_95_migration.cpp
    src/vascular_graph.cpp
)
add_library(MEHLISSA::body_model ALIAS mehlissa_body_model)

target_include_directories(
    mehlissa_body_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_body_model
    PUBLIC MEHLISSA::core
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_body_model PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_body_model)

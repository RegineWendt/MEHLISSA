# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_cell_model
    src/analytical_receptor_ligand_model.cpp
    src/receptor_ligand_profile.cpp
)
add_library(MEHLISSA::cell_model ALIAS mehlissa_cell_model)

target_include_directories(
    mehlissa_cell_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_cell_model
    PUBLIC MEHLISSA::core
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_cell_model PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_cell_model)

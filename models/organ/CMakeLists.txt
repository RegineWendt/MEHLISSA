# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_library(mehlissa_organ_model src/lung_compartment.cpp)
add_library(MEHLISSA::organ_model ALIAS mehlissa_organ_model)

target_include_directories(
    mehlissa_organ_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(mehlissa_organ_model PUBLIC MEHLISSA::model_coupling)
mehlissa_configure_target(mehlissa_organ_model)

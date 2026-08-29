# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_library(
    mehlissa_cosimulation
    src/body_organ_coupler.cpp
    src/organ_capillary_coupler.cpp
)
add_library(MEHLISSA::cosimulation ALIAS mehlissa_cosimulation)

target_include_directories(
    mehlissa_cosimulation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_cosimulation
    PUBLIC
        MEHLISSA::body_model
        MEHLISSA::model_coupling
)
mehlissa_configure_target(mehlissa_cosimulation)

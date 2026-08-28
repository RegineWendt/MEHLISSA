# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_organ_model
    src/lung_compartment.cpp
    src/lung_model_definition.cpp
    src/lung_model_factory.cpp
    src/pulmonary_circulation.cpp
    src/pulmonary_parallel_beds.cpp
    src/pulmonary_zero_dimensional.cpp
    src/pulmonary_zero_dimensional_validation.cpp
)
add_library(MEHLISSA::organ_model ALIAS mehlissa_organ_model)

target_include_directories(
    mehlissa_organ_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_organ_model
    PUBLIC MEHLISSA::model_coupling
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_organ_model PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_organ_model)

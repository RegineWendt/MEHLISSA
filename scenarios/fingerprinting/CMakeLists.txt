# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_fingerprinting_scenario
    src/level_a_composer.cpp
    src/level_b_detection.cpp
    src/result_report.cpp
    src/runtime_coordinator.cpp
    src/scenario_profile.cpp
)
add_library(MEHLISSA::fingerprinting_scenario ALIAS mehlissa_fingerprinting_scenario)

target_include_directories(
    mehlissa_fingerprinting_scenario
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_fingerprinting_scenario
    PUBLIC
        MEHLISSA::experiment
    PRIVATE
        MEHLISSA::body_model
        MEHLISSA::organ_model
        MEHLISSA::capillary_model
        MEHLISSA::cell_model
        MEHLISSA::iot_model
        MEHLISSA::cosimulation
        MEHLISSA::cell_cosimulation
        MEHLISSA::iot_cosimulation
        jsoncons::jsoncons
)
target_compile_definitions(mehlissa_fingerprinting_scenario PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_fingerprinting_scenario)

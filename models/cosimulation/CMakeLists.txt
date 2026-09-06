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

if(MEHLISSA_BUILD_CELL_MODELS)
    find_package(jsoncons CONFIG REQUIRED)
    add_library(
        mehlissa_cell_cosimulation
        src/capillary_cell_signal_coupler.cpp
        src/capillary_cell_signal_profile.cpp
        src/cell_state_feedback.cpp
        src/dynamic_capillary_tissue_cell_model.cpp
    )
    add_library(MEHLISSA::cell_cosimulation ALIAS mehlissa_cell_cosimulation)
    target_include_directories(
        mehlissa_cell_cosimulation
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(
        mehlissa_cell_cosimulation
        PUBLIC
            MEHLISSA::cell_model
            MEHLISSA::model_coupling
        PRIVATE jsoncons::jsoncons
    )
    target_compile_definitions(mehlissa_cell_cosimulation PRIVATE JSONCONS_NO_DEPRECATED)
    mehlissa_configure_target(mehlissa_cell_cosimulation)

    if(BUILD_TESTING)
        add_executable(
            mehlissa_dccq1_qualification_runner
            tools/dccq1_qualification_runner.cpp
        )
        target_link_libraries(
            mehlissa_dccq1_qualification_runner
            PRIVATE MEHLISSA::cell_cosimulation
        )
        mehlissa_configure_target(mehlissa_dccq1_qualification_runner)
    endif()
endif()

if(MEHLISSA_BUILD_CELL_MODELS AND MEHLISSA_BUILD_IOT_MODELS)
    add_library(
        mehlissa_iot_cosimulation
        src/receptor_detection_adapter.cpp
    )
    add_library(MEHLISSA::iot_cosimulation ALIAS mehlissa_iot_cosimulation)
    target_include_directories(
        mehlissa_iot_cosimulation
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(
        mehlissa_iot_cosimulation
        PUBLIC
            MEHLISSA::cell_model
            MEHLISSA::iot_model
    )
    mehlissa_configure_target(mehlissa_iot_cosimulation)
endif()

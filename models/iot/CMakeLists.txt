# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_iot_model
    src/local_communication_profile.cpp
    src/local_message.cpp
    src/molecular_detection.cpp
    src/nanodevice.cpp
    src/nanodevice_profile.cpp
    src/one_hop_link.cpp
)
add_library(MEHLISSA::iot_model ALIAS mehlissa_iot_model)

target_include_directories(
    mehlissa_iot_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_iot_model
    PUBLIC MEHLISSA::core
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_iot_model PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_iot_model)

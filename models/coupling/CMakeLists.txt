# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_library(
    mehlissa_model_coupling
    src/cell_state_event.cpp
    src/conserved_transfer.cpp
    src/entity_disposition.cpp
    src/entity_transfer.cpp
    src/extracellular_signal.cpp
)
add_library(MEHLISSA::model_coupling ALIAS mehlissa_model_coupling)

target_include_directories(
    mehlissa_model_coupling
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(mehlissa_model_coupling PUBLIC MEHLISSA::core)
mehlissa_configure_target(mehlissa_model_coupling)

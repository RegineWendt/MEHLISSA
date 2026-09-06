# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_library(mehlissa_fdg_pet_scenario src/fdg_pet_model.cpp)
add_library(MEHLISSA::fdg_pet_scenario ALIAS mehlissa_fdg_pet_scenario)
target_include_directories(mehlissa_fdg_pet_scenario PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(mehlissa_fdg_pet_scenario PUBLIC MEHLISSA::core)
mehlissa_configure_target(mehlissa_fdg_pet_scenario)

add_executable(mehlissa_fdg_pet_reference_runner src/reference_runner.cpp)
target_link_libraries(mehlissa_fdg_pet_reference_runner PRIVATE MEHLISSA::fdg_pet_scenario)
mehlissa_configure_target(mehlissa_fdg_pet_reference_runner)

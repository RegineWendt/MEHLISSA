# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_cell_model
    src/apoptosis_population.cpp
    src/apoptosis_population_profile.cpp
    src/analytical_receptor_ligand_model.cpp
    src/apoptosis_response.cpp
    src/apoptosis_response_profile.cpp
    src/drug_delivery.cpp
    src/drug_delivery_profile.cpp
    src/intracellular_response_network.cpp
    src/intracellular_response_profile.cpp
    src/receptor_ligand_profile.cpp
    src/qualified_cd95_apoptosis_model.cpp
    src/stochastic_receptor_ligand_model.cpp
    src/stochastic_receptor_ligand_population.cpp
    src/stochastic_receptor_ligand_profile.cpp
    src/time_varying_receptor_ligand_model.cpp
    src/time_varying_receptor_ligand_profile.cpp
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

if(BUILD_TESTING)
    add_executable(
        mehlissa_bcq1_qualification_runner
        tools/bcq1_qualification_runner.cpp
    )
    target_link_libraries(
        mehlissa_bcq1_qualification_runner
        PRIVATE MEHLISSA::cell_model
    )
    mehlissa_configure_target(mehlissa_bcq1_qualification_runner)
endif()

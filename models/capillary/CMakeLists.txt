# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

find_package(jsoncons CONFIG REQUIRED)

add_library(
    mehlissa_capillary_model
    src/capillary_bed.cpp
    src/capillary_bed_definition.cpp
    src/analytical_diffusion_channel.cpp
    src/brownian_particle_channel.cpp
    src/brownian_particle_channel_profile.cpp
    src/capillary_entity_observation.cpp
    src/capillary_entity_observation_profile.cpp
    src/capillary_exchange.cpp
    src/capillary_exchange_profile.cpp
    src/capillary_recruitment_profile.cpp
    src/molecular_channel_profile.cpp
    src/molecular_channel_comparison.cpp
    src/radial_finite_volume_channel.cpp
    src/radial_finite_volume_channel_profile.cpp
    src/radial_finite_volume_channel_verification.cpp
    src/trajectory_brownian_channel.cpp
    src/trajectory_brownian_channel_profile.cpp
    src/trajectory_brownian_channel_verification.cpp
)
add_library(MEHLISSA::capillary_model ALIAS mehlissa_capillary_model)

target_include_directories(
    mehlissa_capillary_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
    mehlissa_capillary_model
    PUBLIC MEHLISSA::model_coupling
    PRIVATE jsoncons::jsoncons
)
target_compile_definitions(mehlissa_capillary_model PRIVATE JSONCONS_NO_DEPRECATED)
mehlissa_configure_target(mehlissa_capillary_model)

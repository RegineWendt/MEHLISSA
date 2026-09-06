# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.28)

project(
    MEHLISSA
    VERSION 0.1.0
    DESCRIPTION "Medical Holistic Simulation Architecture for Nanonetworks in Humans"
    LANGUAGES CXX
)

include(CTest)
include(cmake/MehlissaTargetOptions.cmake)

option(MEHLISSA_BUILD_APPS "Build MEHLISSA command-line applications" ON)
option(MEHLISSA_ENABLE_EXPERIMENT_IO "Build JSON experiment loading and validation" ON)
option(MEHLISSA_BUILD_BODY_MODELS "Build the validated body-model layer" ON)
option(MEHLISSA_BUILD_MODEL_COUPLING "Build the cross-model coupling contracts" ON)
option(MEHLISSA_BUILD_ORGAN_MODELS "Build the organ-model layer" ON)
option(MEHLISSA_BUILD_CAPILLARY_MODELS "Build the capillary-model layer" ON)
option(MEHLISSA_BUILD_CELL_MODELS "Build the cell-model layer" ON)
option(MEHLISSA_BUILD_IOT_MODELS "Build the Nano-IoT model layer" ON)
option(MEHLISSA_BUILD_COSIMULATION "Build body-organ co-simulation adapters" ON)
option(MEHLISSA_BUILD_FINGERPRINTING_SCENARIO "Build the M7 fingerprinting vertical-slice package" ON)
option(MEHLISSA_BUILD_FDG_PET_SCENARIO "Build the MRSQ FDG/PET reference-scenario package" ON)
option(MEHLISSA_BUILD_BENCHMARKS "Build the reproducible benchmark drivers" ON)
option(MEHLISSA_ENABLE_CATCH2_TESTS "Build the full Catch2 unit-test suite" ON)
option(MEHLISSA_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MEHLISSA_ENABLE_CLANG_TIDY "Run clang-tidy while compiling" OFF)
option(MEHLISSA_ENABLE_SANITIZERS "Enable address and undefined-behaviour sanitizers" OFF)

add_subdirectory(core)

if(MEHLISSA_ENABLE_EXPERIMENT_IO)
    add_subdirectory(experiment)
endif()

if(MEHLISSA_BUILD_BODY_MODELS)
    add_subdirectory(models/body)
endif()

if(MEHLISSA_BUILD_MODEL_COUPLING)
    add_subdirectory(models/coupling)
endif()

if(MEHLISSA_BUILD_ORGAN_MODELS)
    if(NOT MEHLISSA_BUILD_MODEL_COUPLING)
        message(FATAL_ERROR "MEHLISSA organ models require model-coupling contracts")
    endif()
    add_subdirectory(models/organ)
endif()

if(MEHLISSA_BUILD_CAPILLARY_MODELS)
    if(NOT MEHLISSA_BUILD_MODEL_COUPLING)
        message(FATAL_ERROR "MEHLISSA capillary models require model-coupling contracts")
    endif()
    add_subdirectory(models/capillary)
endif()

if(MEHLISSA_BUILD_CELL_MODELS)
    add_subdirectory(models/cell)
endif()

if(MEHLISSA_BUILD_IOT_MODELS)
    add_subdirectory(models/iot)
endif()

if(MEHLISSA_BUILD_COSIMULATION)
    if(NOT MEHLISSA_BUILD_BODY_MODELS OR NOT MEHLISSA_BUILD_MODEL_COUPLING)
        message(FATAL_ERROR "MEHLISSA co-simulation requires body models and coupling contracts")
    endif()
    add_subdirectory(models/cosimulation)
endif()

if(MEHLISSA_BUILD_FINGERPRINTING_SCENARIO)
    if(NOT MEHLISSA_ENABLE_EXPERIMENT_IO OR NOT MEHLISSA_BUILD_BODY_MODELS OR
       NOT MEHLISSA_BUILD_MODEL_COUPLING OR NOT MEHLISSA_BUILD_ORGAN_MODELS OR
       NOT MEHLISSA_BUILD_CAPILLARY_MODELS OR NOT MEHLISSA_BUILD_CELL_MODELS OR
       NOT MEHLISSA_BUILD_IOT_MODELS OR NOT MEHLISSA_BUILD_COSIMULATION)
        message(FATAL_ERROR "The M7 fingerprinting scenario requires all M2-M6 model layers")
    endif()
    add_subdirectory(scenarios/fingerprinting)
endif()

if(MEHLISSA_BUILD_FDG_PET_SCENARIO)
    add_subdirectory(scenarios/fdg_pet)
endif()

if(MEHLISSA_BUILD_BENCHMARKS)
    if(NOT MEHLISSA_ENABLE_EXPERIMENT_IO OR NOT MEHLISSA_BUILD_BODY_MODELS)
        message(FATAL_ERROR "MEHLISSA benchmarks require experiment I/O and body models")
    endif()
    add_subdirectory(benchmarks)
endif()

if(MEHLISSA_BUILD_APPS)
    if(NOT MEHLISSA_ENABLE_EXPERIMENT_IO)
        message(FATAL_ERROR "MEHLISSA applications require experiment I/O")
    endif()
    if(NOT MEHLISSA_BUILD_BODY_MODELS)
        message(FATAL_ERROR "MEHLISSA applications require the body-model layer")
    endif()
    if(NOT MEHLISSA_BUILD_FINGERPRINTING_SCENARIO)
        message(FATAL_ERROR "MEHLISSA applications require the fingerprinting scenario package")
    endif()
    add_subdirectory(apps)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

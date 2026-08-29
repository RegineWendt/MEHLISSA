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
option(MEHLISSA_BUILD_COSIMULATION "Build body-organ co-simulation adapters" ON)
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

if(MEHLISSA_BUILD_COSIMULATION)
    if(NOT MEHLISSA_BUILD_BODY_MODELS OR NOT MEHLISSA_BUILD_MODEL_COUPLING)
        message(FATAL_ERROR "MEHLISSA co-simulation requires body models and coupling contracts")
    endif()
    add_subdirectory(models/cosimulation)
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
    add_subdirectory(apps)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

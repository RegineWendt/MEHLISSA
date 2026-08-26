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
option(MEHLISSA_ENABLE_CATCH2_TESTS "Build the full Catch2 unit-test suite" ON)
option(MEHLISSA_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MEHLISSA_ENABLE_CLANG_TIDY "Run clang-tidy while compiling" OFF)
option(MEHLISSA_ENABLE_SANITIZERS "Enable address and undefined-behaviour sanitizers" OFF)

add_subdirectory(core)

if(MEHLISSA_BUILD_APPS)
    add_subdirectory(apps)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

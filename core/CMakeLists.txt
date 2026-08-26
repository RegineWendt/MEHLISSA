add_library(
    mehlissa_core
    src/position.cpp
    src/random_stream.cpp
    src/simulation_clock.cpp
)
add_library(MEHLISSA::core ALIAS mehlissa_core)

target_include_directories(
    mehlissa_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

mehlissa_configure_target(mehlissa_core)

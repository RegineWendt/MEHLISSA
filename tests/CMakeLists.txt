add_executable(mehlissa_core_smoke_tests smoke_tests.cpp)
target_link_libraries(mehlissa_core_smoke_tests PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa_core_smoke_tests)
add_test(NAME mehlissa_core_smoke_tests COMMAND mehlissa_core_smoke_tests)

if(MEHLISSA_ENABLE_CATCH2_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(
        mehlissa_core_tests
        position_tests.cpp
        random_stream_tests.cpp
        simulation_clock_tests.cpp
    )
    target_link_libraries(
        mehlissa_core_tests
        PRIVATE
            MEHLISSA::core
            Catch2::Catch2WithMain
    )
    mehlissa_configure_target(mehlissa_core_tests)

    include(Catch)
    catch_discover_tests(mehlissa_core_tests DISCOVERY_MODE PRE_TEST)
endif()

add_executable(mehlissa apps_main.cpp)
target_link_libraries(mehlissa PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa)

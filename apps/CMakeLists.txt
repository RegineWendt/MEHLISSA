# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

add_executable(mehlissa apps_main.cpp)
target_link_libraries(mehlissa PRIVATE MEHLISSA::core)
mehlissa_configure_target(mehlissa)

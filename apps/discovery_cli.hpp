// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_APPS_DISCOVERY_CLI_HPP
#define MEHLISSA_APPS_DISCOVERY_CLI_HPP

namespace mehlissa::apps {

[[nodiscard]] bool handles_discovery_command(int argc, const char* const argv[]) noexcept;
[[nodiscard]] int execute_discovery_command(int argc, const char* const argv[]);
void print_discovery_usage();

} // namespace mehlissa::apps

#endif // MEHLISSA_APPS_DISCOVERY_CLI_HPP

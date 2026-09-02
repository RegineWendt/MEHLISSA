// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_APPS_SCENARIO_CLI_HPP
#define MEHLISSA_APPS_SCENARIO_CLI_HPP

namespace mehlissa::apps {

[[nodiscard]] bool handles_usability_command(int argc, const char* const argv[]) noexcept;
[[nodiscard]] int execute_usability_command(int argc, const char* const argv[]);
void print_usability_usage();

} // namespace mehlissa::apps

#endif // MEHLISSA_APPS_SCENARIO_CLI_HPP

// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_APPS_REPORTING_CLI_HPP
#define MEHLISSA_APPS_REPORTING_CLI_HPP

namespace mehlissa::apps {

[[nodiscard]] bool handles_reporting_command(int argc, const char* const argv[]) noexcept;
[[nodiscard]] int execute_reporting_command(int argc, const char* const argv[]);
void print_reporting_usage();

} // namespace mehlissa::apps

#endif // MEHLISSA_APPS_REPORTING_CLI_HPP

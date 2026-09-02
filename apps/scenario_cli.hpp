// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_APPS_SCENARIO_CLI_HPP
#define MEHLISSA_APPS_SCENARIO_CLI_HPP

#include <filesystem>

namespace mehlissa::apps {

struct ScenarioRunRequest final {
    std::filesystem::path file;
    std::filesystem::path output{"results"};
    std::filesystem::path repository_root;
    std::filesystem::path schema;
    std::filesystem::path result_schema;
    std::filesystem::path provenance_schema;
    std::filesystem::path log_schema;
};

struct ScenarioRunOutput final {
    std::filesystem::path directory;
    std::filesystem::path result;
    std::filesystem::path provenance;
    std::filesystem::path log;
    std::filesystem::path summary;
};

[[nodiscard]] bool handles_usability_command(int argc, const char* const argv[]) noexcept;
[[nodiscard]] int execute_usability_command(int argc, const char* const argv[]);
[[nodiscard]] ScenarioRunOutput run_scenario_workflow(const ScenarioRunRequest& request,
                                                      bool print_output = false);
void print_usability_usage();

} // namespace mehlissa::apps

#endif // MEHLISSA_APPS_SCENARIO_CLI_HPP

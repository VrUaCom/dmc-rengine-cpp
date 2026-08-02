#pragma once

#include <filesystem>

namespace dmc::rengine::cli {

void print_integration_help();
int try_run_integration_command(int argc, char** argv);

int run_list_tools();
int run_list_formats();
int run_integration_status();
int run_inspect_workspace(
    const std::filesystem::path& path,
    bool stage_context,
    bool menu_context);

} // namespace dmc::rengine::cli

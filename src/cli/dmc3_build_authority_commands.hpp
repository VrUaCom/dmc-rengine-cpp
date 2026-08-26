#pragma once

#include <filesystem>

namespace dmc::rengine::cli {

void print_dmc3_build_authority_help();

[[nodiscard]] int run_preflight_dmc3_game_test(
    const std::filesystem::path& executable_directory);

// Returns -1 when argv does not name a DMC3 build-authority command.
[[nodiscard]] int try_run_dmc3_build_authority_command(int argc, char** argv);

} // namespace dmc::rengine::cli

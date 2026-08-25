#pragma once

#include <filesystem>
#include <string_view>

namespace dmc::rengine::cli {

void print_dmc3_overlay_help();

[[nodiscard]] int run_build_dmc3_overlay(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& authored_file,
    const std::filesystem::path& output_directory);

// Returns -1 when argv does not name a DMC3 overlay command.
[[nodiscard]] int try_run_dmc3_overlay_command(int argc, char** argv);

} // namespace dmc::rengine::cli

#pragma once

#include <filesystem>
#include <string_view>

namespace dmc::rengine::cli {

void print_dmc3_retail_acquisition_help();

[[nodiscard]] int run_extract_dmc3_retail_member(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& output_file);

// Returns -1 when argv does not name a DMC3 retail-acquisition command.
[[nodiscard]] int try_run_dmc3_retail_acquisition_command(int argc, char** argv);

} // namespace dmc::rengine::cli

#pragma once

#include <filesystem>

namespace dmc::rengine::cli {

void print_relative_slot_help();

[[nodiscard]] int run_rebuild_relative_slot(
    const std::filesystem::path& parent_file,
    unsigned int slot_index,
    const std::filesystem::path& replacement_file,
    const std::filesystem::path& output_file);

// Returns -1 when argv does not name a relative-slot authoring command.
[[nodiscard]] int try_run_relative_slot_command(int argc, char** argv);

} // namespace dmc::rengine::cli

#pragma once

namespace dmc::rengine::cli {

void print_structure_help();

// Returns -1 when argv does not select the structure command.
[[nodiscard]] int try_run_structure_command(int argc, char** argv);

} // namespace dmc::rengine::cli

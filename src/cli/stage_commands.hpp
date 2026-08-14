#pragma once

namespace dmc::rengine::cli {

void print_stage_help();

// Returns -1 when argv does not select a stage command.
[[nodiscard]] int try_run_stage_command(int argc, char** argv);

} // namespace dmc::rengine::cli

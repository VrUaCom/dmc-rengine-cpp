#pragma once

namespace dmc::rengine::cli {

void print_hits_help();

// Returns -1 when argv does not select a HITS command.
[[nodiscard]] int try_run_hits_command(int argc, char** argv);

} // namespace dmc::rengine::cli

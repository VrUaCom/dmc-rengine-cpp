#pragma once

namespace dmc::rengine::cli {

void print_dmc3_l1_closure_help();

// Returns -1 when argv does not name a DMC3 L1 closure command.
[[nodiscard]] int try_run_dmc3_l1_closure_command(int argc, char** argv);

} // namespace dmc::rengine::cli

#pragma once

namespace dmc::rengine::cli {

void print_dmc3_build_authority_help();

// Returns -1 when argv does not name a DMC3 build-authority command.
[[nodiscard]] int try_run_dmc3_build_authority_command(int argc, char** argv);

} // namespace dmc::rengine::cli

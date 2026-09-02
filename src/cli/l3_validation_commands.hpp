#pragma once

namespace dmc::rengine::cli {

void print_l3_validation_help();

// Returns -1 when argv does not name an L3 validation command.
[[nodiscard]] int try_run_l3_validation_command(int argc, char** argv);

} // namespace dmc::rengine::cli

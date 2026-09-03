#pragma once

namespace dmc::rengine::cli {

void print_container_inspect_help();

// Returns -1 when argv does not name a container inspection command.
[[nodiscard]] int try_run_container_inspect_command(int argc, char** argv);

} // namespace dmc::rengine::cli

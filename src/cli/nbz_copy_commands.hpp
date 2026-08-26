#pragma once

namespace dmc::rengine::cli {

void print_nbz_copy_help();

// Returns -1 when argv does not name an NBZ copy-authoring command.
[[nodiscard]] int try_run_nbz_copy_command(int argc, char** argv);

} // namespace dmc::rengine::cli

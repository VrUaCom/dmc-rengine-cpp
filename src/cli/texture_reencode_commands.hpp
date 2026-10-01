#pragma once

namespace dmc::rengine::cli {

void print_texture_reencode_help();

// Returns -1 when argv does not select the texture-reencode command.
[[nodiscard]] int try_run_texture_reencode_command(int argc, char** argv);

} // namespace dmc::rengine::cli

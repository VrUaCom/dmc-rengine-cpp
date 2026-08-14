#pragma once

namespace dmc::rengine::cli {

void print_vanilla_help();
[[nodiscard]] int try_run_vanilla_command(int argc, char** argv);

} // namespace dmc::rengine::cli

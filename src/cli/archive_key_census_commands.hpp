#pragma once

namespace dmc::rengine::cli {

void print_archive_key_census_help();

// Returns -1 when argv does not name an archive-key census command.
[[nodiscard]] int try_run_archive_key_census_command(int argc, char** argv);

} // namespace dmc::rengine::cli

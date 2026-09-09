#pragma once

namespace dmc::rengine::cli {

void print_mod_writer_corpus_help();

// Returns -1 when argv does not name a MOD writer corpus command.
[[nodiscard]] int try_run_mod_writer_corpus_command(int argc, char** argv);

} // namespace dmc::rengine::cli

#include "mod_writer_corpus_commands.hpp"

#include "dmc_rengine/formats/mod_writer_corpus.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] bool write_receipt(
    const std::filesystem::path& path,
    const std::string& json) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return false;
    }
    stream << json;
    stream.flush();
    return static_cast<bool>(stream);
}

[[nodiscard]] int run_mod_writer_corpus(
    const std::filesystem::path& root,
    const std::optional<std::filesystem::path>& receipt_path) {
    const auto receipt =
        formats::mod::WriterCorpusRunner::run(root);

    std::cout << "MOD preserve-layout corpus gate\n"
              << "  root              : " << receipt.root << '\n'
              << "  scan completed    : "
              << (receipt.scan_completed ? "yes" : "no") << '\n'
              << "  MOD files         : " << receipt.mod_file_count << '\n'
              << "  passed            : " << receipt.passed_file_count << '\n'
              << "  failed            : " << receipt.failed_file_count << '\n'
              << "  total source bytes: " << receipt.total_bytes << '\n'
              << "  result            : "
              << (receipt.ok() ? "PASS" : "FAIL") << '\n';

    for (const auto& diagnostic : receipt.diagnostics) {
        std::cerr << "[corpus] " << diagnostic << '\n';
    }
    for (const auto& entry : receipt.entries) {
        if (entry.passed()) {
            continue;
        }
        std::cerr << "[failed] " << entry.relative_path
                  << " status=" << formats::mod::to_string(entry.status)
                  << '\n';
        for (const auto& code : entry.diagnostic_codes) {
            std::cerr << "  " << code << '\n';
        }
    }

    if (receipt_path.has_value()) {
        if (!write_receipt(*receipt_path, receipt.to_json())) {
            std::cerr << "mod-writer-corpus: unable to write receipt: "
                      << receipt_path->string() << '\n';
            return 4;
        }
        std::cout << "  receipt           : "
                  << receipt_path->string() << '\n';
    }

    if (!receipt.scan_completed || receipt.mod_file_count == 0U) {
        return 2;
    }
    return receipt.ok() ? 0 : 3;
}

} // namespace

void print_mod_writer_corpus_help() {
    std::cout
        << "  mod-writer-corpus <directory> [receipt.json]\n"
        << "                            Verify no-op MOD writer byte parity and reopen across a recursive corpus\n";
}

int try_run_mod_writer_corpus_command(int argc, char** argv) {
    if (argc < 2 ||
        std::string_view{argv[1]} != "mod-writer-corpus") {
        return -1;
    }
    if (argc < 3 || argc > 4) {
        std::cerr
            << "Usage: dmc-rengine mod-writer-corpus <directory> [receipt.json]\n";
        return 1;
    }

    std::optional<std::filesystem::path> receipt_path;
    if (argc == 4) {
        receipt_path = std::filesystem::path{argv[3]};
    }
    return run_mod_writer_corpus(
        std::filesystem::path{argv[2]}, receipt_path);
}

} // namespace dmc::rengine::cli

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats::mod {

enum class CorpusEntryStatus : std::uint8_t {
    passed,
    io_error,
    parse_failed,
    write_failed,
    byte_mismatch,
    reopen_failed,
};

[[nodiscard]] std::string_view to_string(CorpusEntryStatus status) noexcept;

struct CorpusEntryReceipt final {
    std::string relative_path;
    CorpusEntryStatus status{CorpusEntryStatus::io_error};
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t byte_count{};
    std::uint64_t modified_byte_count{};
    bool parse_ok{false};
    bool writer_ok{false};
    bool byte_identical{false};
    bool reopen_ok{false};
    std::vector<std::string> diagnostic_codes;

    [[nodiscard]] bool passed() const noexcept;
};

struct CorpusReceipt final {
    std::string root;
    bool scan_completed{false};
    std::uint64_t mod_file_count{};
    std::uint64_t passed_file_count{};
    std::uint64_t failed_file_count{};
    std::uint64_t total_bytes{};
    std::vector<std::string> diagnostics;
    std::vector<CorpusEntryReceipt> entries;

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] std::string to_json() const;
};

class WriterCorpusRunner final {
public:
    // Recursively scans `root` for case-insensitive .MOD files, sorts them by
    // relative path for deterministic receipts, then performs canonical
    // parse -> preserve-layout no-op write -> byte equality -> reopen checks.
    // The runner never writes modified resource bytes back to the corpus.
    [[nodiscard]] static CorpusReceipt run(
        const std::filesystem::path& root);
};

} // namespace dmc::rengine::formats::mod

#include "dmc_rengine/formats/mod_writer_corpus.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_writer.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::mod {
namespace {

[[nodiscard]] bool is_mod_path(const std::filesystem::path& path) {
    auto extension = path.extension().string();
    if (extension.size() != 4U) {
        return false;
    }
    for (auto& character : extension) {
        character = static_cast<char>(std::tolower(
            static_cast<unsigned char>(character)));
    }
    return extension == ".mod";
}

[[nodiscard]] std::string relative_name(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
    std::error_code error;
    const auto relative = std::filesystem::relative(path, root, error);
    if (!error && !relative.empty()) {
        return relative.lexically_normal().generic_string();
    }
    return path.filename().generic_string();
}

[[nodiscard]] bool read_binary_file(
    const std::filesystem::path& path,
    std::vector<std::byte>& bytes,
    std::string& error_message) {
    std::error_code error;
    const auto raw_size = std::filesystem::file_size(path, error);
    if (error) {
        error_message = "file_size failed: " + error.message();
        return false;
    }
    if (raw_size > static_cast<std::uintmax_t>(
            std::numeric_limits<std::size_t>::max()) ||
        raw_size > static_cast<std::uintmax_t>(
            std::numeric_limits<std::streamsize>::max())) {
        error_message = "file is too large for the in-memory corpus runner";
        return false;
    }

    bytes.assign(static_cast<std::size_t>(raw_size), std::byte{0});
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        error_message = "unable to open file";
        return false;
    }
    if (!bytes.empty()) {
        stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
        if (!stream || stream.gcount() !=
                static_cast<std::streamsize>(bytes.size())) {
            error_message = "short read while loading file";
            bytes.clear();
            return false;
        }
    }
    return true;
}

void append_diagnostic_codes(
    std::vector<std::string>& destination,
    const std::vector<ParseDiagnostic>& diagnostics) {
    destination.reserve(destination.size() + diagnostics.size());
    for (const auto& diagnostic : diagnostics) {
        destination.push_back(diagnostic.code);
    }
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string output;
    output.reserve(value.size() + 8U);
    for (const auto character : value) {
        switch (character) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default: {
            const auto value8 = static_cast<unsigned char>(character);
            if (value8 < 0x20U) {
                output += "\\u00";
                output.push_back(digits[(value8 >> 4U) & 0x0FU]);
                output.push_back(digits[value8 & 0x0FU]);
            } else {
                output.push_back(character);
            }
            break;
        }
        }
    }
    return output;
}

void write_string_array(
    std::ostringstream& output,
    const std::vector<std::string>& values,
    const std::string_view indent) {
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) output << ',';
        output << '\n' << indent << "  \""
               << escape_json(values[index]) << '\"';
    }
    if (!values.empty()) {
        output << '\n' << indent;
    }
    output << ']';
}

} // namespace

std::string_view to_string(const CorpusEntryStatus status) noexcept {
    switch (status) {
    case CorpusEntryStatus::passed: return "passed";
    case CorpusEntryStatus::io_error: return "io_error";
    case CorpusEntryStatus::parse_failed: return "parse_failed";
    case CorpusEntryStatus::write_failed: return "write_failed";
    case CorpusEntryStatus::byte_mismatch: return "byte_mismatch";
    case CorpusEntryStatus::reopen_failed: return "reopen_failed";
    }
    return "unknown";
}

bool CorpusEntryReceipt::passed() const noexcept {
    return status == CorpusEntryStatus::passed && parse_ok && writer_ok &&
           byte_identical && reopen_ok && modified_byte_count == 0U &&
           source_sha256.size() == 64U && source_sha256 == output_sha256;
}

bool CorpusReceipt::ok() const noexcept {
    return scan_completed && mod_file_count != 0U &&
           passed_file_count == mod_file_count && failed_file_count == 0U &&
           entries.size() == static_cast<std::size_t>(mod_file_count);
}

std::string CorpusReceipt::to_json() const {
    std::ostringstream output;
    output << "{\n"
           << "  \"schema\": \"dmc-rengine.mod-writer-corpus-receipt.v1\",\n"
           << "  \"root\": \"" << escape_json(root) << "\",\n"
           << "  \"scan_completed\": "
           << (scan_completed ? "true" : "false") << ",\n"
           << "  \"mod_file_count\": " << mod_file_count << ",\n"
           << "  \"passed_file_count\": " << passed_file_count << ",\n"
           << "  \"failed_file_count\": " << failed_file_count << ",\n"
           << "  \"total_bytes\": " << total_bytes << ",\n"
           << "  \"all_passed\": " << (ok() ? "true" : "false") << ",\n"
           << "  \"diagnostics\": ";
    write_string_array(output, diagnostics, "  ");
    output << ",\n  \"entries\": [";

    for (std::size_t index = 0U; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (index != 0U) output << ',';
        output << "\n    {\n"
               << "      \"relative_path\": \""
               << escape_json(entry.relative_path) << "\",\n"
               << "      \"status\": \"" << to_string(entry.status)
               << "\",\n"
               << "      \"source_sha256\": \""
               << escape_json(entry.source_sha256) << "\",\n"
               << "      \"output_sha256\": \""
               << escape_json(entry.output_sha256) << "\",\n"
               << "      \"byte_count\": " << entry.byte_count << ",\n"
               << "      \"modified_byte_count\": "
               << entry.modified_byte_count << ",\n"
               << "      \"parse_ok\": "
               << (entry.parse_ok ? "true" : "false") << ",\n"
               << "      \"writer_ok\": "
               << (entry.writer_ok ? "true" : "false") << ",\n"
               << "      \"byte_identical\": "
               << (entry.byte_identical ? "true" : "false") << ",\n"
               << "      \"reopen_ok\": "
               << (entry.reopen_ok ? "true" : "false") << ",\n"
               << "      \"diagnostic_codes\": ";
        write_string_array(output, entry.diagnostic_codes, "      ");
        output << "\n    }";
    }

    if (!entries.empty()) output << '\n';
    output << "  ]\n}\n";
    return output.str();
}

CorpusReceipt WriterCorpusRunner::run(const std::filesystem::path& root) {
    CorpusReceipt receipt;

    std::error_code error;
    auto normalized_root = std::filesystem::absolute(root, error);
    if (error) {
        error.clear();
        normalized_root = root;
    }
    normalized_root = normalized_root.lexically_normal();
    receipt.root = normalized_root.generic_string();

    if (!std::filesystem::is_directory(normalized_root, error) || error) {
        receipt.diagnostics.push_back(
            "mod.writer.corpus.root-not-directory");
        return receipt;
    }

    std::vector<std::filesystem::path> files;
    std::filesystem::recursive_directory_iterator iterator(
        normalized_root,
        std::filesystem::directory_options::skip_permission_denied,
        error);
    const std::filesystem::recursive_directory_iterator end;
    if (error) {
        receipt.diagnostics.push_back(
            "mod.writer.corpus.scan-open-failed:" + error.message());
        return receipt;
    }

    receipt.scan_completed = true;
    while (iterator != end) {
        std::error_code entry_error;
        const auto regular = iterator->is_regular_file(entry_error);
        if (entry_error) {
            receipt.scan_completed = false;
            receipt.diagnostics.push_back(
                "mod.writer.corpus.entry-status-failed:" +
                entry_error.message());
        } else if (regular && is_mod_path(iterator->path())) {
            files.push_back(iterator->path());
        }

        iterator.increment(error);
        if (error) {
            receipt.scan_completed = false;
            receipt.diagnostics.push_back(
                "mod.writer.corpus.scan-step-failed:" + error.message());
            break;
        }
    }

    std::sort(files.begin(), files.end(),
              [&normalized_root](const auto& lhs, const auto& rhs) {
                  return relative_name(lhs, normalized_root) <
                         relative_name(rhs, normalized_root);
              });

    receipt.mod_file_count = static_cast<std::uint64_t>(files.size());
    receipt.entries.reserve(files.size());

    for (const auto& path : files) {
        CorpusEntryReceipt entry;
        entry.relative_path = relative_name(path, normalized_root);

        std::vector<std::byte> bytes;
        std::string read_error;
        if (!read_binary_file(path, bytes, read_error)) {
            entry.status = CorpusEntryStatus::io_error;
            entry.diagnostic_codes.push_back(
                "mod.writer.corpus.io-error:" + read_error);
            ++receipt.failed_file_count;
            receipt.entries.push_back(std::move(entry));
            continue;
        }

        entry.byte_count = static_cast<std::uint64_t>(bytes.size());
        receipt.total_bytes += entry.byte_count;
        const auto source_span = std::span<const std::byte>{
            bytes.data(), bytes.size()};
        entry.source_sha256 = core::Sha256::compute(source_span).hex();

        const auto parsed = Parser::parse(source_span);
        entry.parse_ok = parsed.ok();
        append_diagnostic_codes(entry.diagnostic_codes, parsed.diagnostics);
        if (!entry.parse_ok) {
            entry.status = CorpusEntryStatus::parse_failed;
            ++receipt.failed_file_count;
            receipt.entries.push_back(std::move(entry));
            continue;
        }

        const auto written = Writer::write(source_span, parsed.document);
        entry.writer_ok = written.ok();
        entry.output_sha256 = written.receipt.output_sha256;
        entry.modified_byte_count = written.receipt.modified_byte_count;
        append_diagnostic_codes(entry.diagnostic_codes, written.diagnostics);
        if (!entry.writer_ok) {
            entry.status = CorpusEntryStatus::write_failed;
            ++receipt.failed_file_count;
            receipt.entries.push_back(std::move(entry));
            continue;
        }

        entry.byte_identical = written.bytes == bytes &&
            written.receipt.no_edit_byte_identical &&
            written.receipt.modified_byte_count == 0U &&
            written.receipt.source_sha256 == written.receipt.output_sha256 &&
            written.receipt.source_sha256 == entry.source_sha256;
        entry.reopen_ok = written.receipt.output_reparse_ok &&
            written.reparsed.ok();

        if (!entry.byte_identical) {
            entry.status = CorpusEntryStatus::byte_mismatch;
            ++receipt.failed_file_count;
        } else if (!entry.reopen_ok) {
            entry.status = CorpusEntryStatus::reopen_failed;
            ++receipt.failed_file_count;
        } else {
            entry.status = CorpusEntryStatus::passed;
            ++receipt.passed_file_count;
        }
        receipt.entries.push_back(std::move(entry));
    }

    return receipt;
}

} // namespace dmc::rengine::formats::mod

#include "mod_writer_corpus_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_writer.hpp"
#include "dmc_rengine/formats/mod_writer_corpus.hpp"

#include <bit>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

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
        error_message = "file is too large for the in-memory writer gate";
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

[[nodiscard]] bool write_binary_file_exclusive(
    const std::filesystem::path& path,
    const std::span<const std::byte> bytes,
    std::string& error_message) {
    std::error_code error;
    if (std::filesystem::exists(path, error)) {
        error_message = error ?
            "output existence check failed: " + error.message() :
            "output already exists";
        return false;
    }
    if (error) {
        error_message = "output existence check failed: " + error.message();
        return false;
    }

    auto temporary = path;
    temporary += ".dmc-rengine.tmp";
    if (std::filesystem::exists(temporary, error)) {
        error_message = error ?
            "temporary-path check failed: " + error.message() :
            "temporary output already exists";
        return false;
    }
    if (error) {
        error_message = "temporary-path check failed: " + error.message();
        return false;
    }

    {
        std::ofstream stream(
            temporary, std::ios::binary | std::ios::trunc);
        if (!stream) {
            error_message = "unable to create temporary output";
            return false;
        }
        if (!bytes.empty()) {
            stream.write(reinterpret_cast<const char*>(bytes.data()),
                         static_cast<std::streamsize>(bytes.size()));
        }
        stream.flush();
        if (!stream) {
            error_message = "failed while writing temporary output";
            stream.close();
            std::filesystem::remove(temporary, error);
            return false;
        }
    }

    std::filesystem::rename(temporary, path, error);
    if (error) {
        error_message = "unable to publish output: " + error.message();
        std::error_code cleanup_error;
        std::filesystem::remove(temporary, cleanup_error);
        return false;
    }
    return true;
}

[[nodiscard]] bool parse_index(
    const std::string_view text,
    std::size_t& value) noexcept {
    std::uint64_t parsed{};
    const auto first = text.data();
    const auto last = text.data() + text.size();
    const auto result = std::from_chars(first, last, parsed, 10);
    if (result.ec != std::errc{} || result.ptr != last ||
        parsed > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        return false;
    }
    value = static_cast<std::size_t>(parsed);
    return true;
}

[[nodiscard]] bool parse_positive_float(
    const std::string& text,
    float& value) noexcept {
    if (text.empty()) {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    const auto parsed = std::strtof(text.c_str(), &end);
    if (errno == ERANGE || end == text.c_str() ||
        end != text.c_str() + text.size() ||
        !std::isfinite(parsed) || parsed <= 0.0F) {
        return false;
    }
    value = parsed;
    return true;
}

[[nodiscard]] std::string changed_offsets_json(
    const std::vector<std::uint64_t>& offsets) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0U; index < offsets.size(); ++index) {
        if (index != 0U) output << ',';
        output << offsets[index];
    }
    output << ']';
    return output.str();
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

[[nodiscard]] int run_mod_writer_set_bounding_radius(
    const std::filesystem::path& input_path,
    const std::size_t object_index,
    const float new_radius,
    const std::filesystem::path& output_path,
    const std::optional<std::filesystem::path>& receipt_path) {
    std::error_code error;
    const auto absolute_input =
        std::filesystem::absolute(input_path, error).lexically_normal();
    if (error) {
        std::cerr << "mod-writer-set-bounding-radius: input path error: "
                  << error.message() << '\n';
        return 2;
    }
    const auto absolute_output =
        std::filesystem::absolute(output_path, error).lexically_normal();
    if (error) {
        std::cerr << "mod-writer-set-bounding-radius: output path error: "
                  << error.message() << '\n';
        return 2;
    }
    if (absolute_input == absolute_output) {
        std::cerr << "mod-writer-set-bounding-radius: source overwrite is forbidden\n";
        return 2;
    }

    std::vector<std::byte> source;
    std::string io_error;
    if (!read_binary_file(absolute_input, source, io_error)) {
        std::cerr << "mod-writer-set-bounding-radius: " << io_error << '\n';
        return 2;
    }
    const auto source_span = std::span<const std::byte>{
        source.data(), source.size()};
    const auto parsed = formats::mod::Parser::parse(source_span);
    if (!parsed.ok()) {
        std::cerr << "mod-writer-set-bounding-radius: canonical parse failed\n";
        for (const auto& diagnostic : parsed.diagnostics) {
            std::cerr << "  " << diagnostic.code << '\n';
        }
        return 3;
    }
    if (object_index >= parsed.document.outer_models.size()) {
        std::cerr << "mod-writer-set-bounding-radius: object index out of range\n";
        return 3;
    }

    auto edited = parsed.document;
    auto& object = edited.outer_models[object_index];
    const auto old_radius = object.bounding_radius;
    if (std::bit_cast<std::uint32_t>(old_radius) ==
        std::bit_cast<std::uint32_t>(new_radius)) {
        std::cerr << "mod-writer-set-bounding-radius: requested radius is byte-identical to source\n";
        return 3;
    }
    object.bounding_radius = new_radius;

    const auto written = formats::mod::Writer::write(source_span, edited);
    if (!written.ok()) {
        std::cerr << "mod-writer-set-bounding-radius: preserve-layout writer failed\n";
        for (const auto& diagnostic : written.diagnostics) {
            std::cerr << "  " << diagnostic.code << '\n';
        }
        return 4;
    }

    const auto expected_offset = object.record_offset +
        formats::model_family::ObjectCoreAbi::bounding_radius_field;
    constexpr std::uint64_t expected_size = sizeof(float);
    std::vector<std::uint64_t> changed_offsets;
    changed_offsets.reserve(expected_size);
    bool changed_bytes_within_expected_span = true;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] == written.bytes[index]) {
            continue;
        }
        const auto offset = static_cast<std::uint64_t>(index);
        changed_offsets.push_back(offset);
        if (offset < expected_offset ||
            offset >= expected_offset + expected_size) {
            changed_bytes_within_expected_span = false;
        }
    }

    if (changed_offsets.empty() ||
        !changed_bytes_within_expected_span ||
        changed_offsets.size() !=
            static_cast<std::size_t>(written.receipt.modified_byte_count)) {
        std::cerr << "mod-writer-set-bounding-radius: exact changed-span gate failed\n";
        return 5;
    }

    if (!write_binary_file_exclusive(
            absolute_output,
            std::span<const std::byte>{written.bytes.data(),
                                       written.bytes.size()},
            io_error)) {
        std::cerr << "mod-writer-set-bounding-radius: " << io_error << '\n';
        return 6;
    }

    std::vector<std::byte> disk_output;
    if (!read_binary_file(absolute_output, disk_output, io_error)) {
        std::cerr << "mod-writer-set-bounding-radius: output reread failed: "
                  << io_error << '\n';
        return 7;
    }
    const auto disk_span = std::span<const std::byte>{
        disk_output.data(), disk_output.size()};
    const auto disk_sha = core::Sha256::compute(disk_span).hex();
    const auto disk_reparse = formats::mod::Parser::parse(disk_span);
    const bool disk_bytes_match_writer = disk_output == written.bytes;
    const bool disk_hash_matches_writer =
        disk_sha == written.receipt.output_sha256;
    const bool disk_reopen_ok = disk_reparse.ok() &&
        object_index < disk_reparse.document.outer_models.size() &&
        std::bit_cast<std::uint32_t>(
            disk_reparse.document.outer_models[object_index].bounding_radius) ==
        std::bit_cast<std::uint32_t>(new_radius);

    const bool success = disk_bytes_match_writer &&
        disk_hash_matches_writer && disk_reopen_ok &&
        written.receipt.unauthorized_bytes_unchanged &&
        written.receipt.output_reparse_ok;

    std::ostringstream json;
    json << std::setprecision(std::numeric_limits<float>::max_digits10)
         << "{\n"
         << "  \"schema\": \"dmc-rengine.mod-writer-controlled-edit-receipt.v1\",\n"
         << "  \"field\": \"object.bounding_radius\",\n"
         << "  \"object_index\": " << object_index << ",\n"
         << "  \"old_value\": " << old_radius << ",\n"
         << "  \"new_value\": " << new_radius << ",\n"
         << "  \"serialized_offset\": " << expected_offset << ",\n"
         << "  \"serialized_size\": " << expected_size << ",\n"
         << "  \"source_sha256\": \""
         << written.receipt.source_sha256 << "\",\n"
         << "  \"output_sha256\": \""
         << written.receipt.output_sha256 << "\",\n"
         << "  \"byte_count\": " << written.receipt.byte_count << ",\n"
         << "  \"modified_byte_count\": "
         << written.receipt.modified_byte_count << ",\n"
         << "  \"changed_byte_offsets\": "
         << changed_offsets_json(changed_offsets) << ",\n"
         << "  \"changed_bytes_within_expected_span\": "
         << (changed_bytes_within_expected_span ? "true" : "false") << ",\n"
         << "  \"writer_unauthorized_bytes_unchanged\": "
         << (written.receipt.unauthorized_bytes_unchanged ? "true" : "false")
         << ",\n"
         << "  \"writer_output_reparse_ok\": "
         << (written.receipt.output_reparse_ok ? "true" : "false") << ",\n"
         << "  \"disk_bytes_match_writer\": "
         << (disk_bytes_match_writer ? "true" : "false") << ",\n"
         << "  \"disk_hash_matches_writer\": "
         << (disk_hash_matches_writer ? "true" : "false") << ",\n"
         << "  \"disk_output_reopen_ok\": "
         << (disk_reopen_ok ? "true" : "false") << ",\n"
         << "  \"success\": " << (success ? "true" : "false") << "\n"
         << "}\n";

    if (receipt_path.has_value()) {
        if (!write_receipt(*receipt_path, json.str())) {
            std::cerr << "mod-writer-set-bounding-radius: unable to write receipt\n";
            return 8;
        }
    }

    std::cout << std::setprecision(std::numeric_limits<float>::max_digits10)
              << "MOD controlled retail fixed-layout edit gate\n"
              << "  field             : object.bounding_radius\n"
              << "  object            : " << object_index << '\n'
              << "  old radius        : " << old_radius << '\n'
              << "  new radius        : " << new_radius << '\n'
              << "  serialized span   : [" << expected_offset << ", "
              << expected_offset + expected_size << ")\n"
              << "  modified bytes    : "
              << written.receipt.modified_byte_count << '\n'
              << "  output SHA-256    : " << disk_sha << '\n'
              << "  disk reopen       : "
              << (disk_reopen_ok ? "yes" : "no") << '\n'
              << "  result            : " << (success ? "PASS" : "FAIL") << '\n';
    if (receipt_path.has_value()) {
        std::cout << "  receipt           : " << receipt_path->string() << '\n';
    }
    return success ? 0 : 9;
}

} // namespace

void print_mod_writer_corpus_help() {
    std::cout
        << "  mod-writer-corpus <directory> [receipt.json]\n"
        << "                            Verify no-op MOD writer byte parity and reopen across a recursive corpus\n"
        << "  mod-writer-set-bounding-radius <input.mod> <object-index> <radius> <output.mod> [receipt.json]\n"
        << "                            Evidence-oriented fixed-layout radius edit; source overwrite is forbidden\n";
}

int try_run_mod_writer_corpus_command(int argc, char** argv) {
    if (argc < 2) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command == "mod-writer-corpus") {
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

    if (command == "mod-writer-set-bounding-radius") {
        if (argc < 6 || argc > 7) {
            std::cerr
                << "Usage: dmc-rengine mod-writer-set-bounding-radius <input.mod> <object-index> <radius> <output.mod> [receipt.json]\n";
            return 1;
        }
        std::size_t object_index{};
        if (!parse_index(argv[3], object_index)) {
            std::cerr << "mod-writer-set-bounding-radius: invalid object index\n";
            return 1;
        }
        float radius{};
        if (!parse_positive_float(argv[4], radius)) {
            std::cerr << "mod-writer-set-bounding-radius: radius must be a finite positive f32 value\n";
            return 1;
        }
        std::optional<std::filesystem::path> receipt_path;
        if (argc == 7) {
            receipt_path = std::filesystem::path{argv[6]};
        }
        return run_mod_writer_set_bounding_radius(
            std::filesystem::path{argv[2]}, object_index, radius,
            std::filesystem::path{argv[5]}, receipt_path);
    }

    return -1;
}

} // namespace dmc::rengine::cli

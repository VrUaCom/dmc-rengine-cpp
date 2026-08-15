#include "exe_acquisition_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/byte_window.hpp"
#include "dmc_rengine/exe/byte_window_receipt.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<gdspaces::ResourcePayload> load_local_file(
    const std::filesystem::path& path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error ||
        raw_size > static_cast<std::uintmax_t>(
            std::numeric_limits<std::uint64_t>::max())) {
        return std::nullopt;
    }

    constexpr std::string_view source_id = "cli-exe-acquisition";
    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        return std::nullopt;
    }

    const gdspaces::ResourceId resource{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };

    auto payload = registry.read(resource);
    if (!payload || !payload->readable()) {
        return std::nullopt;
    }
    return payload;
}

[[nodiscard]] std::optional<std::uint64_t> parse_u64(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    int base = 10;
    if (text.size() > 2U && text[0] == '0' &&
        (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        text.remove_prefix(2U);
        if (text.empty()) {
            return std::nullopt;
        }
    }

    std::uint64_t value = 0U;
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value, base);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] std::optional<std::string> normalize_sha256(std::string_view text) {
    if (text.size() != 64U) {
        return std::nullopt;
    }

    std::string normalized;
    normalized.reserve(text.size());
    for (const auto ch : text) {
        const auto uch = static_cast<unsigned char>(ch);
        if (std::isxdigit(uch) == 0) {
            return std::nullopt;
        }
        normalized.push_back(static_cast<char>(std::tolower(uch)));
    }
    return normalized;
}

[[nodiscard]] std::string bytes_to_hex(std::span<const std::byte> bytes) {
    static constexpr char k_hex[] = "0123456789abcdef";
    std::string result;
    result.resize(bytes.size() * 2U);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        const auto value = std::to_integer<std::uint8_t>(bytes[index]);
        result[index * 2U] = k_hex[(value >> 4U) & 0x0FU];
        result[index * 2U + 1U] = k_hex[value & 0x0FU];
    }
    return result;
}

int run_extract_exe_window(
    const std::filesystem::path& path,
    std::string_view expected_sha_text,
    std::string_view va_text,
    std::string_view size_text,
    bool include_hex) {
    const auto expected_sha = normalize_sha256(expected_sha_text);
    if (!expected_sha) {
        std::cerr << "Expected SHA-256 must be exactly 64 hexadecimal characters.\n";
        return 2;
    }

    const auto va = parse_u64(va_text);
    const auto size64 = parse_u64(size_text);
    if (!va || !size64 ||
        *size64 > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        std::cerr << "VA and size must be decimal or 0x-prefixed unsigned integers.\n";
        return 2;
    }

    const auto payload = load_local_file(path);
    if (!payload) {
        std::cerr << "Could not read executable through GDSpaces: " << path << '\n';
        return 2;
    }

    const auto file_bytes = std::span<const std::byte>{payload->bytes};
    const auto actual_sha = core::Sha256::compute(file_bytes).hex();
    if (actual_sha != *expected_sha) {
        std::cerr << "Executable SHA-256 mismatch.\n"
                  << "  expected: " << *expected_sha << '\n'
                  << "  actual:   " << actual_sha << '\n';
        return 3;
    }

    const auto pe = exe::PeReader::read(file_bytes);
    for (const auto& warning : pe.warnings) {
        std::cerr << "[warning] " << warning << '\n';
    }
    if (!pe.ok()) {
        std::cerr << "PE inspection failed:\n";
        for (const auto& parse_error : pe.errors) {
            std::cerr << "  - " << parse_error << '\n';
        }
        return 4;
    }

    const auto window = exe::ExeByteWindowExtractor::extract(
        file_bytes,
        *pe.image,
        *va,
        static_cast<std::size_t>(*size64));
    if (!window.ok()) {
        std::cerr << "Executable window extraction failed ["
                  << exe::to_string(window.error) << "]: "
                  << window.message << '\n';
        return 5;
    }

    const auto window_bytes = std::span<const std::byte>{window.window->bytes};
    const auto window_sha = core::Sha256::compute(window_bytes).hex();

    std::string bytes_hex;
    if (include_hex) {
        std::cerr
            << "Warning: --hex emits raw executable bytes for local reverse evidence. "
               "Do not commit proprietary byte output to the public repository.\n";
        bytes_hex = bytes_to_hex(window_bytes);
    }

    const exe::ExeByteWindowReceipt receipt{
        .artifact_sha256 = actual_sha,
        .artifact_size = static_cast<std::uint64_t>(payload->bytes.size()),
        .image_base = pe.image->image_base,
        .va = window.window->va,
        .rva = window.window->rva,
        .file_offset = window.window->file_offset,
        .size = static_cast<std::uint64_t>(window.window->bytes.size()),
        .section_name = window.window->section_name,
        .window_sha256 = window_sha,
    };

    const auto json = exe::byte_window_receipt_to_json(receipt, bytes_hex);
    if (json.empty()) {
        std::cerr << "Failed to construct a valid executable byte-window receipt.\n";
        return 6;
    }

    std::cout << json;
    return 0;
}

} // namespace

int try_run_exe_acquisition_command(int argc, char** argv) {
    if (argc < 2 || std::string_view(argv[1]) != "extract-exe-window") {
        return -1;
    }

    if (argc != 6 && argc != 7) {
        std::cerr
            << "Usage: dmc-rengine extract-exe-window <exe> <expected-sha256> "
               "<va> <size> [--hex]\n";
        return 2;
    }

    bool include_hex = false;
    if (argc == 7) {
        if (std::string_view(argv[6]) != "--hex") {
            std::cerr << "Only optional flag supported is --hex.\n";
            return 2;
        }
        include_hex = true;
    }

    return run_extract_exe_window(
        argv[2],
        argv[3],
        argv[4],
        argv[5],
        include_hex);
}

} // namespace dmc::rengine::cli

#include "exe_acquisition_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/byte_window.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace {

struct LoadedLocalFile final {
    gdspaces::ResourceRef ref;
    gdspaces::ResourcePayload payload;
};

[[nodiscard]] std::optional<LoadedLocalFile> load_local_file(
    const std::filesystem::path& path) {
    const auto absolute = std::filesystem::absolute(path);
    const auto root = absolute.has_parent_path()
        ? absolute.parent_path()
        : std::filesystem::current_path();

    gdspaces::LocalDirectorySource source("cli-exe-acquisition", root, false);
    for (const auto& ref : source.enumerate()) {
        if (ref.id.logical_path != absolute.filename().generic_string()) {
            continue;
        }
        const auto payload = source.read(ref.id);
        if (!payload || !payload->readable()) {
            return std::nullopt;
        }
        return LoadedLocalFile{.ref = ref, .payload = *payload};
    }
    return std::nullopt;
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

[[nodiscard]] std::string hex_u64(std::uint64_t value) {
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << value;
    return stream.str();
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

[[nodiscard]] std::string json_escape(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const auto ch : value) {
        switch (ch) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20U) {
                result += "?";
            } else {
                result.push_back(ch);
            }
            break;
        }
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
    if (!va || !size64 || *size64 > static_cast<std::uint64_t>(SIZE_MAX)) {
        std::cerr << "VA and size must be decimal or 0x-prefixed unsigned integers.\n";
        return 2;
    }

    const auto loaded = load_local_file(path);
    if (!loaded) {
        std::cerr << "Could not read executable through GDSpaces: " << path << '\n';
        return 2;
    }

    const auto actual_sha = core::Sha256::compute(loaded->payload.bytes).hex();
    if (actual_sha != *expected_sha) {
        std::cerr << "Executable SHA-256 mismatch.\n"
                  << "  expected: " << *expected_sha << '\n'
                  << "  actual:   " << actual_sha << '\n';
        return 3;
    }

    const auto pe = exe::PeReader::read(loaded->payload.bytes);
    if (!pe.ok()) {
        std::cerr << "PE inspection failed:\n";
        for (const auto& diagnostic : pe.diagnostics) {
            std::cerr << "  - " << diagnostic.message << '\n';
        }
        return 4;
    }

    const auto window = exe::ExeByteWindowExtractor::extract(
        loaded->payload.bytes,
        *pe.image,
        *va,
        static_cast<std::size_t>(*size64));
    if (!window.ok()) {
        std::cerr << "Executable window extraction failed ["
                  << exe::to_string(window.error) << "]: "
                  << window.message << '\n';
        return 5;
    }

    const auto window_sha = core::Sha256::compute(window.window->bytes).hex();

    if (include_hex) {
        std::cerr
            << "Warning: --hex emits raw executable bytes for local reverse evidence. "
               "Do not commit proprietary byte output to the public repository.\n";
    }

    std::cout << "{\n"
              << "  \"schema\": \"dmc-rengine.exe-byte-window.v1\",\n"
              << "  \"artifact_sha256\": \"" << actual_sha << "\",\n"
              << "  \"artifact_size\": " << loaded->payload.bytes.size() << ",\n"
              << "  \"va\": \"" << hex_u64(window.window->va) << "\",\n"
              << "  \"rva\": \"" << hex_u64(window.window->rva) << "\",\n"
              << "  \"file_offset\": \"" << hex_u64(window.window->file_offset) << "\",\n"
              << "  \"size\": " << window.window->bytes.size() << ",\n"
              << "  \"section\": \"" << json_escape(window.window->section_name) << "\",\n"
              << "  \"window_sha256\": \"" << window_sha << "\"";
    if (include_hex) {
        std::cout << ",\n  \"bytes_hex\": \""
                  << bytes_to_hex(window.window->bytes) << "\"";
    }
    std::cout << "\n}\n";
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

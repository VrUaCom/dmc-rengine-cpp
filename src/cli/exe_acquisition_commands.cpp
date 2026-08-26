#include "exe_acquisition_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/byte_window.hpp"
#include "dmc_rengine/exe/byte_window_receipt.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/exe/process_memory_window.hpp"
#include "dmc_rengine/exe/process_memory_window_receipt.hpp"
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

[[nodiscard]] std::optional<std::string> section_name_for_window(
    const exe::PeImage& image,
    std::uint64_t rva,
    std::uint64_t size) {
    if (size == 0U || rva > std::numeric_limits<std::uint64_t>::max() - size) {
        return std::nullopt;
    }

    if (rva < image.size_of_headers &&
        size <= static_cast<std::uint64_t>(image.size_of_headers) - rva) {
        return std::string{"<headers>"};
    }

    if (rva > std::numeric_limits<std::uint32_t>::max() ||
        size - 1U > std::numeric_limits<std::uint32_t>::max() - rva) {
        return std::nullopt;
    }

    const auto start = static_cast<std::uint32_t>(rva);
    const auto end = static_cast<std::uint32_t>(rva + size - 1U);
    for (const auto& section : image.sections) {
        if (section.contains_rva(start) && section.contains_rva(end)) {
            return section.name;
        }
    }
    return std::nullopt;
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

int run_capture_exe_process_window(
    int argc,
    char** argv) {
    if (argc < 7) {
        std::cerr
            << "Usage: dmc-rengine capture-exe-process-window <pid> "
               "<expected-image-sha256> <expected-image-size> <rva> <size> "
               "[--expect-window <canonical-artifact-sha256> "
               "<canonical-window-sha256>] [--process-instance-v2] [--hex]\n";
        return 2;
    }

    const auto pid64 = parse_u64(argv[2]);
    const auto expected_image_sha = normalize_sha256(argv[3]);
    const auto expected_image_size = parse_u64(argv[4]);
    const auto rva = parse_u64(argv[5]);
    const auto size64 = parse_u64(argv[6]);
    if (!pid64 || *pid64 == 0U ||
        *pid64 > std::numeric_limits<std::uint32_t>::max() ||
        !expected_image_sha || !expected_image_size || *expected_image_size == 0U ||
        !rva || !size64 || *size64 == 0U ||
        *size64 > exe::k_max_process_memory_window_size ||
        *size64 > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        std::cerr
            << "PID/image-size/RVA/size must be valid unsigned values; SHA-256 "
               "must be 64 hex digits and process window size must be within 0x1000.\n";
        return 2;
    }

    bool include_hex = false;
    bool process_instance_v2 = false;
    std::optional<std::string> expected_window_artifact_sha;
    std::optional<std::string> expected_window_sha;
    for (int index = 7; index < argc;) {
        const std::string_view flag{argv[index]};
        if (flag == "--hex") {
            if (include_hex) {
                std::cerr << "--hex may be specified only once.\n";
                return 2;
            }
            include_hex = true;
            ++index;
            continue;
        }
        if (flag == "--process-instance-v2") {
            if (process_instance_v2) {
                std::cerr << "--process-instance-v2 may be specified only once.\n";
                return 2;
            }
            process_instance_v2 = true;
            ++index;
            continue;
        }
        if (flag == "--expect-window") {
            if (expected_window_sha || index + 2 >= argc) {
                std::cerr
                    << "--expect-window requires canonical artifact SHA-256 and "
                       "canonical window SHA-256 exactly once.\n";
                return 2;
            }
            expected_window_artifact_sha = normalize_sha256(argv[index + 1]);
            expected_window_sha = normalize_sha256(argv[index + 2]);
            if (!expected_window_artifact_sha || !expected_window_sha) {
                std::cerr << "--expect-window values must be 64 hex digits.\n";
                return 2;
            }
            index += 3;
            continue;
        }

        std::cerr << "Unknown capture-exe-process-window flag: " << flag << '\n';
        return 2;
    }

    const auto capture = exe::capture_main_module_window(
        static_cast<std::uint32_t>(*pid64),
        *rva,
        static_cast<std::size_t>(*size64));
    if (!capture.ok()) {
        std::cerr << "Process window capture failed ["
                  << exe::to_string(capture.error) << "]: "
                  << capture.message << '\n';
        return capture.error == exe::ProcessMemoryWindowError::platform_not_supported
            ? 7
            : 5;
    }

    const auto& process_window = *capture.window;
    const auto payload = load_local_file(process_window.image_path);
    if (!payload) {
        std::cerr << "Could not read the running process image through GDSpaces: "
                  << process_window.image_path << '\n';
        return 3;
    }

    if (payload->bytes.size() != *expected_image_size) {
        std::cerr << "Running process image size mismatch.\n"
                  << "  expected: " << *expected_image_size << '\n'
                  << "  actual:   " << payload->bytes.size() << '\n';
        return 3;
    }

    const auto file_bytes = std::span<const std::byte>{payload->bytes};
    const auto actual_image_sha = core::Sha256::compute(file_bytes).hex();
    if (actual_image_sha != *expected_image_sha) {
        std::cerr << "Running process image SHA-256 mismatch.\n"
                  << "  expected: " << *expected_image_sha << '\n'
                  << "  actual:   " << actual_image_sha << '\n';
        return 3;
    }

    const auto pe = exe::PeReader::read(file_bytes);
    if (!pe.ok()) {
        std::cerr << "Running process image PE inspection failed.\n";
        return 4;
    }
    if (*rva >= pe.image->size_of_image ||
        *size64 > static_cast<std::uint64_t>(pe.image->size_of_image) - *rva) {
        std::cerr << "Requested runtime RVA window is outside PE SizeOfImage.\n";
        return 4;
    }

    const auto section_name = section_name_for_window(*pe.image, *rva, *size64);
    if (!section_name) {
        std::cerr
            << "Requested runtime RVA window is not contained by one PE section/header range.\n";
        return 4;
    }

    const auto runtime_bytes = std::span<const std::byte>{process_window.bytes};
    const auto runtime_window_sha = core::Sha256::compute(runtime_bytes).hex();

    std::string bytes_hex;
    if (include_hex) {
        std::cerr
            << "Warning: --hex emits raw process bytes for local reverse evidence. "
               "Do not commit proprietary byte output to the public repository.\n";
        bytes_hex = bytes_to_hex(runtime_bytes);
    }

    std::string json;
    bool has_mapping_expectation = false;
    bool matches_expected_window = false;
    if (process_instance_v2) {
        const exe::ProcessMemoryWindowReceiptV2 receipt{
            .artifact_sha256 = actual_image_sha,
            .artifact_size = static_cast<std::uint64_t>(payload->bytes.size()),
            .image_path = process_window.image_path.generic_string(),
            .preferred_image_base = pe.image->image_base,
            .pid = process_window.pid,
            .process_creation_filetime = process_window.process_creation_filetime,
            .module_base = process_window.module_base,
            .rva = process_window.rva,
            .runtime_va = process_window.runtime_va,
            .size = static_cast<std::uint64_t>(process_window.bytes.size()),
            .section_name = *section_name,
            .window_sha256 = runtime_window_sha,
            .expected_window_artifact_sha256 = expected_window_artifact_sha,
            .expected_window_sha256 = expected_window_sha,
        };
        json = exe::process_memory_window_receipt_v2_to_json(receipt, bytes_hex);
        has_mapping_expectation = receipt.has_mapping_expectation();
        matches_expected_window = receipt.matches_expected_window();
    } else {
        const exe::ProcessMemoryWindowReceipt receipt{
            .artifact_sha256 = actual_image_sha,
            .artifact_size = static_cast<std::uint64_t>(payload->bytes.size()),
            .image_path = process_window.image_path.generic_string(),
            .preferred_image_base = pe.image->image_base,
            .pid = process_window.pid,
            .module_base = process_window.module_base,
            .rva = process_window.rva,
            .runtime_va = process_window.runtime_va,
            .size = static_cast<std::uint64_t>(process_window.bytes.size()),
            .section_name = *section_name,
            .window_sha256 = runtime_window_sha,
            .expected_window_artifact_sha256 = expected_window_artifact_sha,
            .expected_window_sha256 = expected_window_sha,
        };
        json = exe::process_memory_window_receipt_to_json(receipt, bytes_hex);
        has_mapping_expectation = receipt.has_mapping_expectation();
        matches_expected_window = receipt.matches_expected_window();
    }

    if (json.empty()) {
        std::cerr << "Failed to construct a valid process-window receipt.\n";
        return 6;
    }

    std::cout << json;
    if (has_mapping_expectation && !matches_expected_window) {
        std::cerr
            << "Runtime window SHA-256 does not match the canonical expected window. "
               "No address-mapping claim is promoted.\n";
        return 8;
    }
    return 0;
}

} // namespace

int try_run_exe_acquisition_command(int argc, char** argv) {
    if (argc < 2) {
        return -1;
    }

    const std::string_view command{argv[1]};
    if (command == "capture-exe-process-window") {
        return run_capture_exe_process_window(argc, argv);
    }

    if (command != "extract-exe-window") {
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

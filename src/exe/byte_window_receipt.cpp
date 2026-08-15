#include "dmc_rengine/exe/byte_window_receipt.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <sstream>
#include <vector>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] bool is_lower_hex_digit(char value) noexcept {
    return (value >= '0' && value <= '9') ||
        (value >= 'a' && value <= 'f');
}

[[nodiscard]] bool is_canonical_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    for (const auto character : value) {
        if (!is_lower_hex_digit(character)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool is_canonical_hex_bytes(
    std::string_view value,
    std::uint64_t size) noexcept {
    if (size > k_max_exe_byte_window_size ||
        value.size() != static_cast<std::size_t>(size) * 2U) {
        return false;
    }
    for (const auto character : value) {
        if (!is_lower_hex_digit(character)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::uint8_t hex_nibble(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return static_cast<std::uint8_t>(value - '0');
    }
    return static_cast<std::uint8_t>(10 + value - 'a');
}

[[nodiscard]] bool bytes_hex_matches_sha256(
    std::string_view bytes_hex,
    std::string_view expected_sha256) {
    std::vector<std::byte> bytes(bytes_hex.size() / 2U);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        const auto high = hex_nibble(bytes_hex[index * 2U]);
        const auto low = hex_nibble(bytes_hex[index * 2U + 1U]);
        bytes[index] = static_cast<std::byte>(
            static_cast<std::uint8_t>((high << 4U) | low));
    }

    return core::Sha256::compute(std::span<const std::byte>{bytes}).hex() ==
        expected_sha256;
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] std::string hex_u64(std::uint64_t value) {
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << value;
    return output.str();
}

} // namespace

bool ExeByteWindowReceipt::valid() const noexcept {
    if (!is_canonical_sha256(artifact_sha256) ||
        !is_canonical_sha256(window_sha256) || artifact_size == 0U ||
        size == 0U || size > k_max_exe_byte_window_size ||
        section_name.empty() || va < image_base) {
        return false;
    }

    const auto rva64 = va - image_base;
    if (rva64 != static_cast<std::uint64_t>(rva)) {
        return false;
    }

    const auto offset64 = static_cast<std::uint64_t>(file_offset);
    return offset64 <= artifact_size && size <= artifact_size - offset64;
}

std::string byte_window_receipt_to_json(
    const ExeByteWindowReceipt& receipt,
    std::string_view bytes_hex) {
    if (!receipt.valid()) {
        return {};
    }
    if (!bytes_hex.empty() &&
        (!is_canonical_hex_bytes(bytes_hex, receipt.size) ||
         !bytes_hex_matches_sha256(bytes_hex, receipt.window_sha256))) {
        return {};
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema\": \"dmc-rengine.exe-byte-window.v1\",\n"
           << "  \"artifact_sha256\": \"" << receipt.artifact_sha256 << "\",\n"
           << "  \"artifact_size\": " << receipt.artifact_size << ",\n"
           << "  \"image_base\": \"" << hex_u64(receipt.image_base) << "\",\n"
           << "  \"va\": \"" << hex_u64(receipt.va) << "\",\n"
           << "  \"rva\": \"" << hex_u64(receipt.rva) << "\",\n"
           << "  \"file_offset\": \"" << hex_u64(receipt.file_offset) << "\",\n"
           << "  \"size\": " << receipt.size << ",\n"
           << "  \"section\": \"" << escape_json(receipt.section_name) << "\",\n"
           << "  \"window_sha256\": \"" << receipt.window_sha256 << "\"";
    if (!bytes_hex.empty()) {
        output << ",\n  \"bytes_hex\": \"" << bytes_hex << "\"";
    }
    output << "\n}\n";
    return output.str();
}

} // namespace dmc::rengine::exe

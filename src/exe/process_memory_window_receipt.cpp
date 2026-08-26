#include "dmc_rengine/exe/process_memory_window_receipt.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/process_memory_window.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
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

[[nodiscard]] std::uint8_t hex_nibble(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return static_cast<std::uint8_t>(value - '0');
    }
    return static_cast<std::uint8_t>(10 + value - 'a');
}

[[nodiscard]] bool is_canonical_hex_bytes(
    std::string_view value,
    std::uint64_t size) noexcept {
    if (size == 0U || size > k_max_process_memory_window_size ||
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

[[nodiscard]] bool common_receipt_fields_valid(
    std::string_view artifact_sha256,
    std::uint64_t artifact_size,
    std::string_view image_path,
    std::uint64_t preferred_image_base,
    std::uint32_t pid,
    std::uint64_t module_base,
    std::uint64_t rva,
    std::uint64_t runtime_va,
    std::uint64_t size,
    std::string_view section_name,
    std::string_view window_sha256,
    const std::optional<std::string>& expected_window_artifact_sha256,
    const std::optional<std::string>& expected_window_sha256) noexcept {
    if (!is_canonical_sha256(artifact_sha256) || artifact_size == 0U ||
        image_path.empty() || preferred_image_base == 0U || pid == 0U ||
        module_base == 0U || size == 0U ||
        size > k_max_process_memory_window_size || section_name.empty() ||
        !is_canonical_sha256(window_sha256)) {
        return false;
    }
    if (rva > std::numeric_limits<std::uint64_t>::max() - module_base ||
        module_base + rva != runtime_va) {
        return false;
    }
    const auto has_expected_artifact = expected_window_artifact_sha256.has_value();
    const auto has_expected_window = expected_window_sha256.has_value();
    if (has_expected_artifact != has_expected_window) {
        return false;
    }
    if (has_expected_artifact &&
        (!is_canonical_sha256(*expected_window_artifact_sha256) ||
         !is_canonical_sha256(*expected_window_sha256))) {
        return false;
    }
    return true;
}

void emit_common_receipt_fields(
    std::ostringstream& output,
    std::string_view schema,
    std::string_view artifact_sha256,
    std::uint64_t artifact_size,
    std::string_view image_path,
    std::uint64_t preferred_image_base,
    std::uint32_t pid,
    std::optional<std::uint64_t> process_creation_filetime,
    std::uint64_t module_base,
    std::uint64_t rva,
    std::uint64_t runtime_va,
    std::uint64_t size,
    std::string_view section_name,
    std::string_view window_sha256) {
    output << "{\n"
           << "  \"schema\": \"" << schema << "\",\n"
           << "  \"artifact_sha256\": \"" << artifact_sha256 << "\",\n"
           << "  \"artifact_size\": " << artifact_size << ",\n"
           << "  \"image_path\": \"" << escape_json(image_path) << "\",\n"
           << "  \"preferred_image_base\": \""
           << hex_u64(preferred_image_base) << "\",\n"
           << "  \"pid\": " << pid << ",\n";
    if (process_creation_filetime.has_value()) {
        output << "  \"process_creation_filetime\": "
               << *process_creation_filetime << ",\n";
    }
    output << "  \"module_base\": \"" << hex_u64(module_base) << "\",\n"
           << "  \"rva\": \"" << hex_u64(rva) << "\",\n"
           << "  \"runtime_va\": \"" << hex_u64(runtime_va) << "\",\n"
           << "  \"size\": " << size << ",\n"
           << "  \"section\": \"" << escape_json(section_name) << "\",\n"
           << "  \"window_sha256\": \"" << window_sha256 << "\"";
}

void emit_optional_expectation(
    std::ostringstream& output,
    const std::optional<std::string>& expected_window_artifact_sha256,
    const std::optional<std::string>& expected_window_sha256,
    bool matches_expected_window) {
    if (expected_window_artifact_sha256 && expected_window_sha256) {
        output << ",\n  \"expected_window_artifact_sha256\": \""
               << *expected_window_artifact_sha256 << "\",\n"
               << "  \"expected_window_sha256\": \""
               << *expected_window_sha256 << "\",\n"
               << "  \"matches_expected_window\": "
               << (matches_expected_window ? "true" : "false");
    }
}

} // namespace

bool ProcessMemoryWindowReceipt::has_mapping_expectation() const noexcept {
    return expected_window_artifact_sha256.has_value() &&
        expected_window_sha256.has_value();
}

bool ProcessMemoryWindowReceipt::matches_expected_window() const noexcept {
    return has_mapping_expectation() &&
        window_sha256 == *expected_window_sha256;
}

bool ProcessMemoryWindowReceipt::valid() const noexcept {
    return common_receipt_fields_valid(
        artifact_sha256,
        artifact_size,
        image_path,
        preferred_image_base,
        pid,
        module_base,
        rva,
        runtime_va,
        size,
        section_name,
        window_sha256,
        expected_window_artifact_sha256,
        expected_window_sha256);
}

bool ProcessMemoryWindowReceiptV2::has_mapping_expectation() const noexcept {
    return expected_window_artifact_sha256.has_value() &&
        expected_window_sha256.has_value();
}

bool ProcessMemoryWindowReceiptV2::matches_expected_window() const noexcept {
    return has_mapping_expectation() &&
        window_sha256 == *expected_window_sha256;
}

bool ProcessMemoryWindowReceiptV2::valid() const noexcept {
    return process_creation_filetime != 0U && common_receipt_fields_valid(
        artifact_sha256,
        artifact_size,
        image_path,
        preferred_image_base,
        pid,
        module_base,
        rva,
        runtime_va,
        size,
        section_name,
        window_sha256,
        expected_window_artifact_sha256,
        expected_window_sha256);
}

std::string process_memory_window_receipt_to_json(
    const ProcessMemoryWindowReceipt& receipt,
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
    emit_common_receipt_fields(
        output,
        "dmc-rengine.exe-process-window.v1",
        receipt.artifact_sha256,
        receipt.artifact_size,
        receipt.image_path,
        receipt.preferred_image_base,
        receipt.pid,
        std::nullopt,
        receipt.module_base,
        receipt.rva,
        receipt.runtime_va,
        receipt.size,
        receipt.section_name,
        receipt.window_sha256);
    emit_optional_expectation(
        output,
        receipt.expected_window_artifact_sha256,
        receipt.expected_window_sha256,
        receipt.matches_expected_window());
    if (!bytes_hex.empty()) {
        output << ",\n  \"bytes_hex\": \"" << bytes_hex << "\"";
    }
    output << "\n}\n";
    return output.str();
}

std::string process_memory_window_receipt_v2_to_json(
    const ProcessMemoryWindowReceiptV2& receipt,
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
    emit_common_receipt_fields(
        output,
        "dmc-rengine.exe-process-window.v2",
        receipt.artifact_sha256,
        receipt.artifact_size,
        receipt.image_path,
        receipt.preferred_image_base,
        receipt.pid,
        receipt.process_creation_filetime,
        receipt.module_base,
        receipt.rva,
        receipt.runtime_va,
        receipt.size,
        receipt.section_name,
        receipt.window_sha256);
    emit_optional_expectation(
        output,
        receipt.expected_window_artifact_sha256,
        receipt.expected_window_sha256,
        receipt.matches_expected_window());
    if (!bytes_hex.empty()) {
        output << ",\n  \"bytes_hex\": \"" << bytes_hex << "\"";
    }
    output << "\n}\n";
    return output.str();
}

} // namespace dmc::rengine::exe

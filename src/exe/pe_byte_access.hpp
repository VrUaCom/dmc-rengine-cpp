#pragma once

/// Bounds-checked little-endian primitives shared by the PE readers.
///
/// Internal to `src/exe`. Every accessor returns an empty optional rather than
/// reading past the buffer, so a malformed image degrades into missing fields
/// instead of undefined behavior.

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::exe::detail {

[[nodiscard]] inline bool has_range(std::span<const std::byte> bytes, std::size_t offset,
                                    std::size_t size) noexcept {
    return offset <= bytes.size() && size <= bytes.size() - offset;
}

[[nodiscard]] inline std::optional<std::uint8_t> read_u8(std::span<const std::byte> bytes,
                                                         std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 1U)) {
        return std::nullopt;
    }
    return std::to_integer<std::uint8_t>(bytes[offset]);
}

[[nodiscard]] inline std::optional<std::uint16_t> read_u16(std::span<const std::byte> bytes,
                                                           std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 2U)) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U));
}

[[nodiscard]] inline std::optional<std::uint32_t> read_u32(std::span<const std::byte> bytes,
                                                           std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 4U)) {
        return std::nullopt;
    }

    std::uint32_t value = 0;
    for (std::size_t index = 0; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

[[nodiscard]] inline std::optional<std::uint64_t> read_u64(std::span<const std::byte> bytes,
                                                           std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 8U)) {
        return std::nullopt;
    }

    std::uint64_t value = 0;
    for (std::size_t index = 0; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(bytes[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

/// Reads a NUL-terminated ASCII string, refusing unterminated and
/// unreasonably long runs rather than walking to the end of the file.
[[nodiscard]] inline std::optional<std::string> read_cstring(std::span<const std::byte> bytes,
                                                             std::size_t offset,
                                                             std::size_t limit = 4096U) {
    if (offset >= bytes.size()) {
        return std::nullopt;
    }

    std::string text;
    for (std::size_t index = 0; index < limit && offset + index < bytes.size(); ++index) {
        const auto character = std::to_integer<unsigned char>(bytes[offset + index]);
        if (character == 0U) {
            return text;
        }
        text.push_back(static_cast<char>(character));
    }
    return std::nullopt;
}

} // namespace dmc::rengine::exe::detail

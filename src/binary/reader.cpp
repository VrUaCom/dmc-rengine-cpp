#include "dmc_rengine/binary/reader.hpp"

#include <bit>
#include <cstring>

namespace dmc::rengine::binary {

Reader::Reader(std::span<const std::byte> bytes) noexcept
    : bytes_(bytes) {}

std::size_t Reader::size() const noexcept {
    return bytes_.size();
}

bool Reader::contains(std::size_t offset, std::size_t length) const noexcept {
    return offset <= bytes_.size() && length <= bytes_.size() - offset;
}

std::optional<std::span<const std::byte>> Reader::slice(
    std::size_t offset,
    std::size_t length) const noexcept {
    if (!contains(offset, length)) {
        return std::nullopt;
    }
    return bytes_.subspan(offset, length);
}

std::optional<std::uint8_t> Reader::u8(std::size_t offset) const noexcept {
    if (!contains(offset, 1U)) {
        return std::nullopt;
    }
    return std::to_integer<std::uint8_t>(bytes_[offset]);
}

std::optional<std::uint16_t> Reader::u16_le(std::size_t offset) const noexcept {
    if (!contains(offset, 2U)) {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes_[offset]) |
        (static_cast<std::uint16_t>(
             std::to_integer<std::uint8_t>(bytes_[offset + 1U]))
         << 8U));
}

std::optional<std::uint32_t> Reader::u32_le(std::size_t offset) const noexcept {
    if (!contains(offset, 4U)) {
        return std::nullopt;
    }

    std::uint32_t value = 0U;
    for (std::size_t index = 0; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(
                     std::to_integer<std::uint8_t>(bytes_[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

std::optional<std::uint64_t> Reader::u64_le(std::size_t offset) const noexcept {
    if (!contains(offset, 8U)) {
        return std::nullopt;
    }

    std::uint64_t value = 0U;
    for (std::size_t index = 0; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
                     std::to_integer<std::uint8_t>(bytes_[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

std::optional<float> Reader::f32_le(std::size_t offset) const noexcept {
    const auto bits = u32_le(offset);
    if (!bits.has_value()) {
        return std::nullopt;
    }
    return std::bit_cast<float>(*bits);
}

std::optional<std::string> Reader::fixed_string(
    std::size_t offset,
    std::size_t length,
    bool stop_at_null) const {
    const auto bytes = slice(offset, length);
    if (!bytes.has_value()) {
        return std::nullopt;
    }

    std::string value;
    value.reserve(length);
    for (const auto byte : *bytes) {
        const auto character = std::to_integer<unsigned char>(byte);
        if (stop_at_null && character == 0U) {
            break;
        }
        value.push_back(static_cast<char>(character));
    }
    return value;
}

bool Reader::matches(
    std::size_t offset,
    std::string_view ascii) const noexcept {
    if (!contains(offset, ascii.size())) {
        return false;
    }

    for (std::size_t index = 0; index < ascii.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes_[offset + index]) !=
            static_cast<unsigned char>(ascii[index])) {
            return false;
        }
    }
    return true;
}

} // namespace dmc::rengine::binary

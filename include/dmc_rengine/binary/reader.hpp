#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::binary {

class Reader final {
public:
    explicit Reader(std::span<const std::byte> bytes) noexcept;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool contains(std::size_t offset, std::size_t length) const noexcept;
    [[nodiscard]] std::optional<std::span<const std::byte>> slice(
        std::size_t offset,
        std::size_t length) const noexcept;

    [[nodiscard]] std::optional<std::uint8_t> u8(std::size_t offset) const noexcept;
    [[nodiscard]] std::optional<std::uint16_t> u16_le(std::size_t offset) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> u32_le(std::size_t offset) const noexcept;
    [[nodiscard]] std::optional<std::uint64_t> u64_le(std::size_t offset) const noexcept;
    [[nodiscard]] std::optional<float> f32_le(std::size_t offset) const noexcept;

    [[nodiscard]] std::optional<std::string> fixed_string(
        std::size_t offset,
        std::size_t length,
        bool stop_at_null = true) const;
    [[nodiscard]] bool matches(
        std::size_t offset,
        std::string_view ascii) const noexcept;

private:
    std::span<const std::byte> bytes_;
};

} // namespace dmc::rengine::binary

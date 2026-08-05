#pragma once

#include <compare>
#include <cstdint>
#include <optional>

namespace dmc::rengine::exe {

enum class AddressSpace : std::uint8_t {
    file_offset,
    rva,
    va,
};

struct Address {
    AddressSpace space{AddressSpace::file_offset};
    std::uint64_t value{};

    auto operator<=>(const Address&) const = default;
};

struct AddressRange {
    Address begin{};
    std::uint64_t size{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return size != 0U;
    }

    [[nodiscard]] constexpr std::optional<std::uint64_t> end_value() const noexcept {
        if (begin.value > UINT64_MAX - size) {
            return std::nullopt;
        }
        return begin.value + size;
    }

    [[nodiscard]] constexpr bool contains(const Address address) const noexcept {
        if (address.space != begin.space) {
            return false;
        }
        const auto end = end_value();
        return end.has_value() && address.value >= begin.value && address.value < *end;
    }
};

} // namespace dmc::rengine::exe

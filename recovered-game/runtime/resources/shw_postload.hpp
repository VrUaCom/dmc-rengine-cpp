#pragma once

#include "runtime/resources/resource_manager.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

namespace dmc::recovered::dmc3::runtime::resources {

inline constexpr std::uint64_t shw_postload_helper_va = 0x1403204C0ULL;
inline constexpr std::size_t shw_record_count_offset = 0x10U;
inline constexpr std::size_t shw_first_record_offset = 0x30U;
inline constexpr std::size_t shw_record_stride = 0x40U;
inline constexpr std::size_t shw_pointer_count_per_record = 4U;

namespace shw_detail {

[[nodiscard]] inline std::uint64_t read_u64_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint64_t value{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

inline void write_u64_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint64_t value) noexcept {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

} // namespace shw_detail

// Direct reconstruction of helper 0x1403204C0. For each record the original
// code converts four file-relative qword offsets to process pointers by adding
// the loaded resource base address in place.
//
// This wrapper adds only a host-side bounds check for the record table itself so
// malformed research fixtures cannot make the reconstruction write past the
// supplied span. It does not add semantic validation of the relative offsets,
// because the original helper does not perform such a check in this loop.
[[nodiscard]] inline bool apply_shw_postload(
    std::span<std::byte> bytes) noexcept {
    if (bytes.size() <= shw_record_count_offset) {
        return false;
    }

    const auto record_count = std::to_integer<std::uint8_t>(
        bytes[shw_record_count_offset]);
    if (record_count == 0U) {
        return true;
    }

    const auto last_record = static_cast<std::size_t>(record_count - 1U);
    const auto required_size = shw_first_record_offset +
        last_record * shw_record_stride +
        (shw_pointer_count_per_record - 1U) * 8U + 8U;
    if (required_size > bytes.size()) {
        return false;
    }

    const auto base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(bytes.data()));
    for (std::size_t record = 0U; record < record_count; ++record) {
        const auto record_offset =
            shw_first_record_offset + record * shw_record_stride;
        for (std::size_t pointer_index = 0U;
             pointer_index < shw_pointer_count_per_record;
             ++pointer_index) {
            const auto field_offset = record_offset + pointer_index * 8U;
            const auto relative = shw_detail::read_u64_le(bytes, field_offset);
            if (relative > std::numeric_limits<std::uint64_t>::max() - base) {
                return false;
            }
            shw_detail::write_u64_le(
                bytes, field_offset, base + relative);
        }
    }
    return true;
}

class ShwPostLoadBackend final : public IConfirmedPostLoadBackend {
public:
    [[nodiscard]] bool apply(
        PostLoadFormat format,
        std::span<std::byte> bytes) override {
        if (format != PostLoadFormat::shw) {
            return false;
        }
        return apply_shw_postload(bytes);
    }
};

} // namespace dmc::recovered::dmc3::runtime::resources

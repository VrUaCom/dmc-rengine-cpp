#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::tests::hits_fixture {

inline void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

inline void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

[[nodiscard]] inline std::vector<std::byte> make_minimal_hits(
    std::uint32_t flags = 0x00000001U) {
    constexpr std::size_t spatial_offset = 0x44U;
    constexpr std::size_t pointer_table_offset = 0x4CU;
    constexpr std::size_t list_offset = 0x50U;
    constexpr std::size_t triangle_offset = 0x58U;
    constexpr std::size_t end_offset = 0x90U;

    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};

    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));

    write_f32(bytes, 0x08U, 0.0F);
    write_f32(bytes, 0x0CU, 0.0F);
    write_f32(bytes, 0x10U, 0.0F);
    write_f32(bytes, 0x14U, 10.0F);
    write_f32(bytes, 0x18U, 10.0F);
    write_f32(bytes, 0x1CU, 10.0F);

    write_f32(bytes, 0x20U, 10.0F);
    write_f32(bytes, 0x24U, 10.0F);
    write_f32(bytes, 0x28U, 10.0F);

    write_u32(bytes, 0x2CU, 1U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 1U);
    write_u32(bytes, 0x3CU, static_cast<std::uint32_t>(spatial_offset - 8U));
    write_u32(bytes, 0x40U, static_cast<std::uint32_t>(triangle_offset - 8U));

    write_u32(bytes, pointer_table_offset,
        static_cast<std::uint32_t>(list_offset - 8U));
    write_u32(bytes, list_offset, 0U);
    write_u32(bytes, list_offset + 4U, 0xFFFFFFFFU);

    write_u32(bytes, triangle_offset + 0x00U, flags);
    write_f32(bytes, triangle_offset + 0x04U, 0.0F);
    write_f32(bytes, triangle_offset + 0x08U, 0.0F);
    write_f32(bytes, triangle_offset + 0x0CU, 0.0F);
    write_f32(bytes, triangle_offset + 0x10U, 1.0F);
    write_f32(bytes, triangle_offset + 0x14U, 0.0F);
    write_f32(bytes, triangle_offset + 0x18U, 0.0F);
    write_f32(bytes, triangle_offset + 0x1CU, 0.0F);
    write_f32(bytes, triangle_offset + 0x20U, 0.0F);
    write_f32(bytes, triangle_offset + 0x24U, 1.0F);
    write_f32(bytes, triangle_offset + 0x28U, 0.0F);
    write_f32(bytes, triangle_offset + 0x2CU, -1.0F);
    write_f32(bytes, triangle_offset + 0x30U, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);

    return bytes;
}

} // namespace dmc::rengine::tests::hits_fixture

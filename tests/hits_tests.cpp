#include "dmc_rengine/formats/hits.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(std::vector<std::byte>& bytes, std::size_t offset, std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_vec3(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float x,
    float y,
    float z) {
    write_f32(bytes, offset, x);
    write_f32(bytes, offset + 4U, y);
    write_f32(bytes, offset + 8U, z);
}

void write_triangle(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t flags,
    float origin_x) {
    write_u32(bytes, offset, flags);
    write_vec3(bytes, offset + 0x04U, origin_x, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x10U, origin_x + 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x1CU, origin_x, 0.0F, 1.0F);
    write_vec3(bytes, offset + 0x28U, 0.0F, 1.0F, 0.0F);
    write_f32(bytes, offset + 0x34U, 0.0F);
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    constexpr std::size_t triangle_offset = 0x80U;
    constexpr std::size_t end_offset = triangle_offset + 2U * 0x38U;
    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -10.0F, -2.0F, -10.0F);
    write_vec3(bytes, 0x14U, 10.0F, 2.0F, 10.0F);
    write_vec3(bytes, 0x20U, 10.0F, 4.0F, 20.0F);
    write_u32(bytes, 0x2CU, 2U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 2U);
    write_u32(bytes, 0x3CU, 0x3CU); // +8 -> 0x44 spatial base
    write_u32(bytes, 0x40U, 0x78U); // +8 -> 0x80 triangle base

    write_i32(bytes, 0x4CU, 0x4CU); // +8 -> 0x54 list 0
    write_i32(bytes, 0x50U, 0x54U); // +8 -> 0x5C list 1
    write_i32(bytes, 0x54U, 0);
    write_i32(bytes, 0x58U, -1);
    write_i32(bytes, 0x5CU, 0x38);
    write_i32(bytes, 0x60U, -1);

    write_triangle(bytes, triangle_offset, 0x18060001U, -2.0F);
    write_triangle(bytes, triangle_offset + 0x38U, 0x00000001U, 3.0F);
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::flatten_cell_index;
    using dmc::rengine::formats::hits::triangle_size;

    static_assert(triangle_size == 0x38U);
    static_assert(flatten_cell_index(1U, 2U, 3U, 4U, 5U) == 33U);

    const auto bytes = make_fixture();
    const auto result = RecordScanner::scan(std::span<const std::byte>{bytes});
    assert(result.recognized);
    assert(result.ok());
    assert(result.header.grid_count_x == 2U);
    assert(result.header.cell_count() == 2U);
    assert(result.header.spatial_offset() == 0x44U);
    assert(result.header.triangle_offset() == 0x80U);
    assert(result.cells.size() == 2U);
    assert(result.cells[0].triangle_byte_offsets == std::vector<std::uint32_t>{0U});
    assert(result.cells[1].triangle_byte_offsets == std::vector<std::uint32_t>{0x38U});
    assert(result.triangles.size() == 2U);
    assert(result.triangles[0].flags == 0x18060001U);
    assert(result.triangles[0].point_b.x == -1.0F);
    assert(result.triangles[0].normal.y == 1.0F);
    assert(result.triangles[0].plane_d == 0.0F);
    assert(dmc::rengine::formats::hits::evaluate_plane(
        result.triangles[0], result.triangles[0].point_c) == 0.0F);

    const std::vector<std::byte> wrong_magic{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'}, std::byte{'E'}};
    const auto unrecognized = RecordScanner::scan(wrong_magic);
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    std::vector<std::byte> truncated{
        std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}};
    const auto truncated_result = RecordScanner::scan(truncated);
    assert(truncated_result.recognized);
    assert(!truncated_result.ok());
    assert(truncated_result.diagnostics[0].severity == ParseSeverity::error);
    assert(truncated_result.diagnostics[0].code == "hits.truncated_header");

    auto invalid_reference = bytes;
    write_i32(invalid_reference, 0x54U, 1);
    const auto invalid = RecordScanner::scan(invalid_reference);
    assert(invalid.recognized);
    assert(!invalid.ok());
    assert(invalid.diagnostics.back().code == "hits.invalid_triangle_reference");

    return 0;
}

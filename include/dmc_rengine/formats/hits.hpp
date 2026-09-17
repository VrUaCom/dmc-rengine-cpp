#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::hits {

inline constexpr std::size_t magic_size = 4U;
inline constexpr std::size_t header_size = 0x44U;
inline constexpr std::size_t triangle_size = 0x38U;
inline constexpr std::int32_t cell_list_terminator = -1;
inline constexpr std::uint64_t relative_offset_base = 8U;

struct Vec3 final {
    float x{};
    float y{};
    float z{};

    friend bool operator==(const Vec3&, const Vec3&) = default;
};

// Serialized HITS +0x20/+0x24/+0x28 are DWORD integer cell extents.
// The canonical DMC3 executable divides integer-truncated world deltas by
// these fields and converts them to float only when building cell AABBs.
struct CellExtent final {
    std::uint32_t x{};
    std::uint32_t y{};
    std::uint32_t z{};

    friend bool operator==(const CellExtent&, const CellExtent&) = default;
};

struct Header final {
    std::uint32_t end_offset{};
    Vec3 bounds_min;
    Vec3 bounds_max;
    CellExtent cell_size;
    std::uint32_t grid_count_x{};
    std::uint32_t grid_count_y{};
    std::uint32_t grid_count_z{};
    std::uint32_t triangle_count{};
    std::uint32_t spatial_table_relative_offset{};
    std::uint32_t triangle_array_relative_offset{};

    [[nodiscard]] std::uint64_t cell_count() const noexcept;
    [[nodiscard]] std::uint64_t spatial_offset() const noexcept;
    [[nodiscard]] std::uint64_t triangle_offset() const noexcept;
};

struct Triangle final {
    std::uint64_t offset{};
    std::uint32_t flags{};
    Vec3 point_a;
    Vec3 point_b;
    Vec3 point_c;
    Vec3 normal;
    float plane_d{};
};

struct Cell final {
    std::uint32_t index{};
    std::uint64_t pointer_offset{};
    std::uint64_t list_offset{};
    std::vector<std::uint32_t> triangle_byte_offsets;
};

struct ScanResult final {
    bool recognized{false};
    Header header;
    std::vector<Cell> cells;
    std::vector<Triangle> triangles;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class RecordScanner final {
public:
    [[nodiscard]] static ScanResult scan(std::span<const std::byte> bytes);
};

[[nodiscard]] constexpr std::uint32_t flatten_cell_index(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z,
    std::uint32_t grid_count_y,
    std::uint32_t grid_count_z) noexcept {
    return ((x * grid_count_y) + y) * grid_count_z + z;
}

[[nodiscard]] constexpr std::uint16_t upper_flag_mask(
    std::uint32_t flags) noexcept {
    return static_cast<std::uint16_t>(flags >> 16U);
}

[[nodiscard]] constexpr std::uint16_t lower_flag_value(
    std::uint32_t flags) noexcept {
    return static_cast<std::uint16_t>(flags & 0xFFFFU);
}

[[nodiscard]] float evaluate_plane(
    const Triangle& triangle,
    const Vec3& point) noexcept;

} // namespace dmc::rengine::formats::hits

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

// Read-only port of the character collision tables (ICollisionHandle, vtable
// 0x1404C65A0). Reverse authority: dmc-rengine-cpp
// docs/research/dmc3-collision-tables-2026-09-24.md.
//
// The handle is set up by 0x14005C260(handle, index table, shape table, owner
// kind, ...): pl000.pac slots 6/7 (0x1401EEFD1), em028 slots 11/12
// (0x140130FA8), the em000 family slots 39/40 (0x14009823F). An attack
// 0x14005C740(handle, id, param, bone matrices) reads index entry `id`
// (4 bytes: target mask, bone, u16 shape) and spawns the 80-byte shape
// record, placed on that bone's world matrix.
namespace dmc::rengine::profiles::dmc3::collision {

// Index entry (0x14005C7C6).
struct AttackEntry final {
    std::uint8_t mask{};    // bits 1/2/4 -> target layers (0x1402CCD60)
    std::uint8_t bone{};    // model bone matrix index
    std::uint16_t shape{};  // shape record index
};

enum class ShapeType : std::uint8_t {
    Sphere = 2,   // centre +0x10, radius +0x20
    Box = 3,      // centre +0x10, Euler degrees +0x1C/+0x20/+0x24, half size +0x28 (0x1402CC115,
                  // corners +-1 at 0x1405CEC60)
    Capsule = 4,  // a +0x10, b +0x20, radius +0x30 (0x1402CC300)
};

struct Shape final {
    std::uint8_t type{};  // ShapeType, other values kept as read (0..6 dispatch)
    std::array<float, 3> a{};
    std::array<float, 3> b{};  // capsule end; box Euler degrees
    std::array<float, 3> size{};  // box half extents
    float radius{};
    std::array<float, 16> raw{};  // +0x10..+0x4C as read
};

inline constexpr std::size_t kShapeRecordSize = 0x50U;

// Content identity of a shape table: whole 80-byte records, type byte 0..6
// with zero padding up to +0x10, finite values, at least one sphere/box/capsule.
[[nodiscard]] bool looks_like_shape_table(std::span<const std::uint8_t> bytes) noexcept;
[[nodiscard]] std::vector<Shape> parse_shapes(std::span<const std::uint8_t> bytes);

// Index table: whole 4-byte entries; with `shape_count` every non-empty
// entry must name an existing shape.
[[nodiscard]] bool looks_like_attack_index(std::span<const std::uint8_t> bytes,
                                           std::optional<std::size_t> shape_count) noexcept;
[[nodiscard]] std::vector<AttackEntry> parse_attack_index(std::span<const std::uint8_t> bytes);

}  // namespace dmc::rengine::profiles::dmc3::collision

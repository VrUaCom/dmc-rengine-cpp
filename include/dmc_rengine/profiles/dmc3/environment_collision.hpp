#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

namespace dmc::rengine::profiles::dmc3::environment_collision {

// HITS is the stage/environment collision family.  It is deliberately kept
// separate from collision_shapes.h: the latter is the character attack
// collision table bound to body bones, while HITS stores world-space
// triangle-plane records and a spatial cell index.
struct Triangle final {
    std::uint32_t flags{};
    Vec3 point_a{};
    Vec3 point_b{};
    Vec3 point_c{};
    Vec3 normal{};
    float plane_d{};
};

struct Source final {
    std::string resource_name;
    std::uint32_t resource_slot{};

    Vec3 bounds_min{};
    Vec3 bounds_max{};
    // The current vendored parser exposes +0x20 as a Vec3f, but the supplied
    // retail surfaces carry exact integer-looking little-endian words there
    // (500/500/500 and 300/300/300). Preserve the raw lane until its EXE
    // scalar encoding is independently closed; do not use it for transforms
    // or gameplay queries.
    std::array<std::uint32_t, 3> cell_size_raw{};
    std::uint32_t grid_count_x{};
    std::uint32_t grid_count_y{};
    std::uint32_t grid_count_z{};
    std::size_t cell_reference_count{};

    std::vector<Triangle> triangles;

    // The source ordering and physical slot are preserved.  For DMC3 stage
    // PACs the first HITS source is the detailed source-0 path and a later
    // source may be the optional coarse source-1 path; this is provenance,
    // not a universal semantic guess for other game profiles.
    const char* evidence{"EXE_AND_CORPUS_CONFIRMED"};
};

[[nodiscard]] bool looks_like(std::span<const std::uint8_t> bytes) noexcept;

// Read-only, fail-closed projection of the canonical Rengine HITS parser.
// Unknown header/flag semantics remain outside this product projection.
[[nodiscard]] std::optional<Source> parse(std::string_view resource_name,
                                           std::uint32_t resource_slot,
                                           std::span<const std::uint8_t> bytes) noexcept;

// One kind of record: a distinct `flags` value (+0x00 of the triangle-plane
// record). The low byte holds type bits, the high word (flags >> 16) the
// category bits that 0x14005E880 tests against the caller's skip mask.
// Numbering follows ascending flag value, so it is stable for a source.
struct Kind final {
    std::uint32_t flags{};
    std::size_t count{};
    std::size_t floors{};    // normal.y >= kWallNormalY
    std::size_t walls{};
    std::size_t ceilings{};  // normal.y <= -kWallNormalY
};
[[nodiscard]] std::vector<Kind> kinds(const Source& source);
// Kind index (into kinds(source)) of every triangle record.
[[nodiscard]] std::vector<std::uint8_t> triangle_kinds(const Source& source,
                                                        const std::vector<Kind>& kinds);
// Kind index of every line pair of debug_lines (3 per triangle).
[[nodiscard]] std::vector<std::uint8_t> debug_line_kinds(const Source& source,
                                                          const std::vector<Kind>& kinds);

// Room-local line pairs for the optional collision inspection overlay.  The
// renderer applies the same room pivot/yaw/offset as the visible stage mesh.
[[nodiscard]] std::vector<Vec3> debug_lines(const Source& source);

// Segment query of the stage collision manager, 0x14005E880 (reached through
// 0x14005E7A0 on [global+0x28]+0x710): the triangle-plane records of the
// cells the segment crosses are tested once each; a triangle whose
// (flags >> 16) shares a bit with `skip_mask` is ignored (the caller's
// category mask, e.g. 0x10 for the CEm034 line-of-sight test 0x140168ED0,
// 0x02/0x10/0x20/0x40 by object type in 0x1402C64F0, 0 without an object).
// Every accepted hit shortens the segment, so the nearest hit wins; the hit
// triangle record is copied out.
struct SegmentHit final {
    Vec3 point{};
    Vec3 normal{};
    float fraction{};  // 0 at `from`, 1 at `to`
    std::size_t triangle{};
    std::uint32_t flags{};
};
[[nodiscard]] std::optional<SegmentHit> segment_hit(const Source& source,
                                                    const Vec3& from,
                                                    const Vec3& to,
                                                    std::uint16_t skip_mask = 0U) noexcept;

// Closest point of a triangle record to `point` (for sphere push-out).
[[nodiscard]] Vec3 closest_point(const Triangle& triangle, const Vec3& point) noexcept;

// Records steeper than this |normal.y| are walls; flatter-up ones floors.
inline constexpr float kWallNormalY = 0.7F;

// Reader character proxy (the retail character-vs-HITS response is not
// decoded yet): a sphere of `radius` moved from `from` to `to` in sub-steps
// of at most radius/2, pushed horizontally out of every wall record it
// overlaps, so it slides along walls and cannot tunnel through them.
[[nodiscard]] Vec3 slide_sphere(const Source& source, const Vec3& from, const Vec3& to,
                                float radius) noexcept;

// Height of the highest floor record (normal.y >= kWallNormalY) under the
// vertical line through `point`, between point.y and point.y - depth.
[[nodiscard]] std::optional<float> floor_below(const Source& source, const Vec3& point,
                                               float depth) noexcept;

}  // namespace dmc::rengine::profiles::dmc3::environment_collision

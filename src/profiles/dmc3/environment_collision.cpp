#include "dmc_rengine/profiles/dmc3/environment_collision.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

#include "dmc_rengine/formats/hits.hpp"

namespace dmc::rengine::profiles::dmc3::environment_collision {
namespace {

[[nodiscard]] Vec3 convert(const dmc::rengine::formats::hits::Vec3& value) noexcept {
    return {value.x, value.y, value.z};
}

[[nodiscard]] std::uint32_t read_u32_le(std::span<const std::uint8_t> bytes,
                                        std::size_t offset) noexcept {
    if (offset > bytes.size() || bytes.size() - offset < 4U) return 0U;
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

}  // namespace

bool looks_like(std::span<const std::uint8_t> bytes) noexcept {
    return bytes.size() >= 4U && bytes[0] == static_cast<std::uint8_t>('H') &&
           bytes[1] == static_cast<std::uint8_t>('I') &&
           bytes[2] == static_cast<std::uint8_t>('T') &&
           bytes[3] == static_cast<std::uint8_t>('S');
}

std::optional<Source> parse(std::string_view resource_name,
                            std::uint32_t resource_slot,
                            std::span<const std::uint8_t> bytes) noexcept {
    try {
        const auto raw = std::span<const std::byte>{
            reinterpret_cast<const std::byte*>(bytes.data()), bytes.size()};
        const auto scan = dmc::rengine::formats::hits::RecordScanner::scan(raw);
        if (!scan.ok()) return std::nullopt;

        Source out;
        out.resource_name = std::string{resource_name};
        out.resource_slot = resource_slot;
        out.bounds_min = convert(scan.header.bounds_min);
        out.bounds_max = convert(scan.header.bounds_max);
        out.cell_size_raw = {
            read_u32_le(bytes, 0x20U),
            read_u32_le(bytes, 0x24U),
            read_u32_le(bytes, 0x28U),
        };
        out.grid_count_x = scan.header.grid_count_x;
        out.grid_count_y = scan.header.grid_count_y;
        out.grid_count_z = scan.header.grid_count_z;
        for (const auto& cell : scan.cells) {
            out.cell_reference_count += cell.triangle_byte_offsets.size();
        }
        out.triangles.reserve(scan.triangles.size());
        for (const auto& triangle : scan.triangles) {
            out.triangles.push_back({
                .flags = triangle.flags,
                .point_a = convert(triangle.point_a),
                .point_b = convert(triangle.point_b),
                .point_c = convert(triangle.point_c),
                .normal = convert(triangle.normal),
                .plane_d = triangle.plane_d,
            });
        }
        return out;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<Vec3> debug_lines(const Source& source) {
    std::vector<Vec3> out;
    out.reserve(source.triangles.size() * 6U);
    for (const auto& triangle : source.triangles) {
        out.push_back(triangle.point_a);
        out.push_back(triangle.point_b);
        out.push_back(triangle.point_b);
        out.push_back(triangle.point_c);
        out.push_back(triangle.point_c);
        out.push_back(triangle.point_a);
    }
    return out;
}

std::vector<Kind> kinds(const Source& source) {
    std::vector<Kind> out;
    for (const auto& t : source.triangles) {
        auto it = std::find_if(out.begin(), out.end(), [&](const Kind& k) { return k.flags == t.flags; });
        if (it == out.end()) {
            out.push_back({t.flags, 0U, 0U, 0U, 0U});
            it = out.end() - 1;
        }
        ++it->count;
        if (t.normal.y >= kWallNormalY) ++it->floors;
        else if (t.normal.y <= -kWallNormalY) ++it->ceilings;
        else ++it->walls;
    }
    std::sort(out.begin(), out.end(), [](const Kind& a, const Kind& b) { return a.flags < b.flags; });
    return out;
}

std::vector<std::uint8_t> triangle_kinds(const Source& source, const std::vector<Kind>& list) {
    std::vector<std::uint8_t> out;
    out.reserve(source.triangles.size());
    for (const auto& t : source.triangles) {
        const auto it = std::find_if(list.begin(), list.end(), [&](const Kind& k) { return k.flags == t.flags; });
        out.push_back(static_cast<std::uint8_t>(it == list.end() ? 0U : std::min<std::size_t>(it - list.begin(), 255U)));
    }
    return out;
}

std::vector<std::uint8_t> debug_line_kinds(const Source& source, const std::vector<Kind>& list) {
    const auto per_triangle = triangle_kinds(source, list);
    std::vector<std::uint8_t> out;
    out.reserve(per_triangle.size() * 3U);
    for (const auto kind : per_triangle) out.insert(out.end(), 3U, kind);
    return out;
}

namespace {

[[nodiscard]] Vec3 sub(const Vec3& a, const Vec3& b) noexcept { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
[[nodiscard]] float dot(const Vec3& a, const Vec3& b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }
[[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) noexcept {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
[[nodiscard]] Vec3 lerp(const Vec3& a, const Vec3& b, float t) noexcept {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
}

}  // namespace

std::optional<SegmentHit> segment_hit(const Source& source, const Vec3& from, const Vec3& to,
                                      std::uint16_t skip_mask) noexcept {
    const Vec3 lo{std::min(from.x, to.x), std::min(from.y, to.y), std::min(from.z, to.z)};
    const Vec3 hi{std::max(from.x, to.x), std::max(from.y, to.y), std::max(from.z, to.z)};
    constexpr float kPad = 1.0F;
    if (hi.x < source.bounds_min.x - kPad || lo.x > source.bounds_max.x + kPad ||
        hi.y < source.bounds_min.y - kPad || lo.y > source.bounds_max.y + kPad ||
        hi.z < source.bounds_min.z - kPad || lo.z > source.bounds_max.z + kPad) {
        return std::nullopt;
    }
    std::optional<SegmentHit> best;
    float limit = 1.0F;
    for (std::size_t index = 0U; index < source.triangles.size(); ++index) {
        const auto& t = source.triangles[index];
        if (skip_mask != 0U && ((t.flags >> 16U) & skip_mask) != 0U) continue;
        // The record's plane: n.p + d = 0. The segment must cross it.
        const float da = dot(t.normal, from) + t.plane_d;
        const float db = dot(t.normal, to) + t.plane_d;
        if ((da > 0.0F && db > 0.0F) || (da < 0.0F && db < 0.0F) || da == db) continue;
        const float f = da / (da - db);
        if (!(f >= 0.0F) || f > limit) continue;
        const Vec3 p = lerp(from, to, f);
        // Inside test against the three edges, in the plane of the record.
        const Vec3 e0 = cross(sub(t.point_b, t.point_a), sub(p, t.point_a));
        const Vec3 e1 = cross(sub(t.point_c, t.point_b), sub(p, t.point_b));
        const Vec3 e2 = cross(sub(t.point_a, t.point_c), sub(p, t.point_c));
        const float s0 = dot(e0, t.normal), s1 = dot(e1, t.normal), s2 = dot(e2, t.normal);
        constexpr float kEdge = -1.0e-3F;
        const bool inside = (s0 >= kEdge && s1 >= kEdge && s2 >= kEdge) ||
                            (s0 <= -kEdge && s1 <= -kEdge && s2 <= -kEdge);
        if (!inside) continue;
        limit = f;
        best = SegmentHit{p, t.normal, f, index, t.flags};
    }
    return best;
}

Vec3 closest_point(const Triangle& t, const Vec3& p) noexcept {
    // Ericson, Real-Time Collision Detection 5.1.5.
    const Vec3 ab = sub(t.point_b, t.point_a), ac = sub(t.point_c, t.point_a), ap = sub(p, t.point_a);
    const float d1 = dot(ab, ap), d2 = dot(ac, ap);
    if (d1 <= 0.0F && d2 <= 0.0F) return t.point_a;
    const Vec3 bp = sub(p, t.point_b);
    const float d3 = dot(ab, bp), d4 = dot(ac, bp);
    if (d3 >= 0.0F && d4 <= d3) return t.point_b;
    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0F && d1 >= 0.0F && d3 <= 0.0F) return lerp(t.point_a, t.point_b, d1 / (d1 - d3));
    const Vec3 cp = sub(p, t.point_c);
    const float d5 = dot(ab, cp), d6 = dot(ac, cp);
    if (d6 >= 0.0F && d5 <= d6) return t.point_c;
    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0F && d2 >= 0.0F && d6 <= 0.0F) return lerp(t.point_a, t.point_c, d2 / (d2 - d6));
    const float va = d3 * d6 - d5 * d4;
    if (va <= 0.0F && (d4 - d3) >= 0.0F && (d5 - d6) >= 0.0F) {
        return lerp(t.point_b, t.point_c, (d4 - d3) / ((d4 - d3) + (d5 - d6)));
    }
    const float denom = 1.0F / (va + vb + vc);
    const float v = vb * denom, w = vc * denom;
    return {t.point_a.x + ab.x * v + ac.x * w, t.point_a.y + ab.y * v + ac.y * w,
            t.point_a.z + ab.z * v + ac.z * w};
}

Vec3 slide_sphere(const Source& source, const Vec3& from, const Vec3& to, float radius) noexcept {
    if (!(radius > 0.0F)) return to;
    const Vec3 delta = sub(to, from);
    const float length = std::sqrt(dot(delta, delta));
    if (!std::isfinite(length)) return from;
    const int steps = std::clamp(static_cast<int>(std::ceil(length / (radius * 0.5F))), 1, 256);
    const Vec3 step{delta.x / static_cast<float>(steps), delta.y / static_cast<float>(steps),
                    delta.z / static_cast<float>(steps)};
    // Only records near the path can touch the sphere.
    const float reach = radius + 1.0F;
    const Vec3 lo{std::min(from.x, to.x) - reach, std::min(from.y, to.y) - reach, std::min(from.z, to.z) - reach};
    const Vec3 hi{std::max(from.x, to.x) + reach, std::max(from.y, to.y) + reach, std::max(from.z, to.z) + reach};
    std::vector<const Triangle*> walls;
    for (const auto& t : source.triangles) {
        if (std::fabs(t.normal.y) >= kWallNormalY) continue;
        const float tx0 = std::min({t.point_a.x, t.point_b.x, t.point_c.x});
        const float tx1 = std::max({t.point_a.x, t.point_b.x, t.point_c.x});
        const float ty0 = std::min({t.point_a.y, t.point_b.y, t.point_c.y});
        const float ty1 = std::max({t.point_a.y, t.point_b.y, t.point_c.y});
        const float tz0 = std::min({t.point_a.z, t.point_b.z, t.point_c.z});
        const float tz1 = std::max({t.point_a.z, t.point_b.z, t.point_c.z});
        if (tx1 < lo.x || tx0 > hi.x || ty1 < lo.y || ty0 > hi.y || tz1 < lo.z || tz0 > hi.z) continue;
        walls.push_back(&t);
    }
    Vec3 at = from;
    for (int i = 0; i < steps; ++i) {
        at = {at.x + step.x, at.y + step.y, at.z + step.z};
        for (int pass = 0; pass < 4; ++pass) {
            bool moved = false;
            for (const auto* t : walls) {
                const Vec3 q = closest_point(*t, at);
                const Vec3 d = sub(at, q);
                const float dist = std::sqrt(dot(d, d));
                if (dist >= radius) continue;
                // Push along the wall's horizontal normal, to the side the
                // sphere centre is on.
                float nx = t->normal.x, nz = t->normal.z;
                const float nl = std::sqrt(nx * nx + nz * nz);
                if (!(nl > 1.0e-6F)) continue;
                nx /= nl;
                nz /= nl;
                const float side = dot(t->normal, at) + t->plane_d;
                if (side < 0.0F) {
                    nx = -nx;
                    nz = -nz;
                }
                const float along = d.x * nx + d.z * nz;  // current horizontal clearance
                const float push = radius - std::max(along, 0.0F);
                if (!(push > 0.0F)) continue;
                at.x += nx * push;
                at.z += nz * push;
                moved = true;
            }
            if (!moved) break;
        }
    }
    return at;
}

std::optional<float> floor_below(const Source& source, const Vec3& p, float depth) noexcept {
    std::optional<float> best;
    for (const auto& t : source.triangles) {
        if (t.normal.y < kWallNormalY) continue;
        // Point-in-triangle in the xz projection.
        const auto edge = [&](const Vec3& a, const Vec3& b) {
            return (b.x - a.x) * (p.z - a.z) - (b.z - a.z) * (p.x - a.x);
        };
        const float e0 = edge(t.point_a, t.point_b), e1 = edge(t.point_b, t.point_c), e2 = edge(t.point_c, t.point_a);
        const bool inside = (e0 >= 0.0F && e1 >= 0.0F && e2 >= 0.0F) || (e0 <= 0.0F && e1 <= 0.0F && e2 <= 0.0F);
        if (!inside) continue;
        const float y = -(t.normal.x * p.x + t.normal.z * p.z + t.plane_d) / t.normal.y;
        if (!std::isfinite(y) || y > p.y + 1.0F || y < p.y - depth) continue;
        if (!best || y > *best) best = y;
    }
    return best;
}

}  // namespace dmc::rengine::profiles::dmc3::environment_collision

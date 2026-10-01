#include "dmc_rengine/profiles/dmc3/cloth_chain.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <string>

namespace dmc::rengine::profiles::dmc3::motion {
namespace {

using Vec = std::array<float, 3>;
using Mat = std::array<float, 16>;

[[nodiscard]] Vec row(const Mat& m, std::size_t r) noexcept {
    return {m[r * 4U + 0U], m[r * 4U + 1U], m[r * 4U + 2U]};
}

void set_row(Mat& m, std::size_t r, const Vec& v) noexcept {
    m[r * 4U + 0U] = v[0];
    m[r * 4U + 1U] = v[1];
    m[r * 4U + 2U] = v[2];
    m[r * 4U + 3U] = 0.0F;
}

[[nodiscard]] Vec add(const Vec& a, const Vec& b) noexcept { return {a[0] + b[0], a[1] + b[1], a[2] + b[2]}; }
[[nodiscard]] Vec sub(const Vec& a, const Vec& b) noexcept { return {a[0] - b[0], a[1] - b[1], a[2] - b[2]}; }
[[nodiscard]] Vec scale(const Vec& a, float s) noexcept { return {a[0] * s, a[1] * s, a[2] * s}; }
[[nodiscard]] float dot(const Vec& a, const Vec& b) noexcept { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
[[nodiscard]] float length(const Vec& a) noexcept { return std::sqrt(dot(a, a)); }
[[nodiscard]] Vec cross(const Vec& a, const Vec& b) noexcept {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}
[[nodiscard]] Vec normalized(const Vec& a) noexcept {
    const float l = length(a);
    return l > 0.0F ? scale(a, 1.0F / l) : a;
}

// 0x14032EEE0 (X), 0x14032F4C0 (Y), 0x14032FD90 (Z): rebuild the rotation
// rows so axis `axis` equals d, using r (a rest row) as the reference.
void align_axis(Mat& m, std::uint8_t axis, Vec d, Vec r) noexcept {
    if (length(d) + 1.0e-6F == 0.0F) d = {1.0F, 0.0F, 0.0F};
    if (length(r) + 1.0e-6F == 0.0F) r = {0.0F, 1.0F, 0.0F};
    if (axis == 0U) {
        const Vec c = cross(d, r);
        set_row(m, 0U, normalized(d));
        set_row(m, 1U, normalized(cross(c, d)));
        set_row(m, 2U, normalized(c));
    } else if (axis == 1U) {
        const Vec c = cross(r, d);
        set_row(m, 0U, normalized(cross(d, c)));
        set_row(m, 1U, normalized(d));
        set_row(m, 2U, normalized(c));
    } else {
        const Vec c = cross(r, d);
        set_row(m, 0U, normalized(c));
        set_row(m, 1U, normalized(cross(d, c)));
        set_row(m, 2U, normalized(d));
    }
    m[15] = 1.0F;
}

[[nodiscard]] std::vector<std::string_view> tokens(std::string_view line) {
    std::vector<std::string_view> out;
    std::size_t i = 0U;
    while (i < line.size()) {
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
        std::size_t j = i;
        while (j < line.size() && !std::isspace(static_cast<unsigned char>(line[j]))) ++j;
        if (j > i) out.push_back(line.substr(i, j - i));
        i = j;
    }
    return out;
}

[[nodiscard]] float to_float(std::string_view s) {
    try {
        return std::stof(std::string{s});
    } catch (...) {
        return 0.0F;
    }
}

[[nodiscard]] std::int32_t to_int(std::string_view s) {
    std::int32_t value = 0;
    std::from_chars(s.data(), s.data() + s.size(), value);
    return value;
}

}  // namespace

bool looks_like_clt(std::string_view text) {
    if (text.empty() || text.front() != ';') return false;
    const auto head = text.substr(0U, std::min<std::size_t>(text.size(), 1024U));
    return head.find("ClothNo") != std::string_view::npos;
}

std::vector<ClothParams> parse_clt(std::string_view text) {
    std::vector<ClothParams> out;
    if (text.empty() || text.front() != ';') return out;
    std::optional<ClothParams> current;
    std::size_t start = 0U;
    while (start < text.size()) {
        auto end = text.find_first_of("\r\n", start);
        if (end == std::string_view::npos) end = text.size();
        const auto words = tokens(text.substr(start, end - start));
        start = end + 1U;
        if (words.empty()) continue;
        const auto key = words.front();
        if (key == "ClothNo") {
            if (current) out.push_back(std::move(*current));
            current.emplace();
        } else if (!current) {
            continue;
        } else if (key == "Gravity" && words.size() >= 4U) {
            current->gravity = {to_float(words[1]), to_float(words[2]), to_float(words[3])};
        } else if (key == "Wind" && words.size() >= 4U) {
            current->wind = {to_float(words[1]), to_float(words[2]), to_float(words[3])};
        } else if (key == "SpringForce" && words.size() >= 2U) {
            current->spring_force = to_float(words[1]);
        } else if (key == "MaxSpeed" && words.size() >= 2U) {
            current->max_speed = to_float(words[1]);
        } else if (key == "Stiffness" && words.size() >= 2U) {
            current->stiffness = to_float(words[1]);
        } else if (key == "FloorLevel" && words.size() >= 2U) {
            current->floor_level = to_float(words[1]);
        } else if (key == "WindLocal" && words.size() >= 2U) {
            current->wind_local = to_int(words[1]) == 1;
        } else if (key == "WindParent" && words.size() >= 2U) {
            current->wind_parent = to_int(words[1]);
        } else if (key == "WindType" && words.size() >= 2U) {
            current->wind_type = to_int(words[1]);
        } else if (key == "LimitLength" && words.size() >= 2U) {
            current->limit_length = to_int(words[1]) == 1;
        } else if (key == "Bone" && words.size() >= 3U) {
            const auto axis = words[2];
            std::uint8_t id = 1U;
            if (axis == "X") id = 0U;
            else if (axis == "Y") id = 1U;
            else if (axis == "Z") id = 2U;
            else if (axis == "NX") id = 3U;
            else if (axis == "NY") id = 4U;
            else if (axis == "NZ") id = 5U;
            else continue;
            current->bones.push_back({static_cast<std::uint32_t>(to_int(words[1])), id});
        } else if (key == "End") {
            out.push_back(std::move(*current));
            current.reset();
        }
    }
    if (current) out.push_back(std::move(*current));
    return out;
}

std::array<float, 16> step_cloth_node(ClothState& state,
                                      std::uint32_t node,
                                      const std::array<float, 16>& target,
                                      const std::array<float, 16>& parent,
                                      const std::array<float, 16>& wind_parent_world,
                                      float rest_length,
                                      float dt,
                                      std::span<const WorldCapsule> capsules) {
    const auto& p = state.params_for(node);
    if (node >= state.sim.size() || node >= state.axis_by_node.size() ||
        state.axis_by_node[node] < 0) {
        return target;
    }
    auto& velocity = state.velocity[node];
    Mat sim = state.sim[node];
    const auto axis = static_cast<std::uint8_t>(state.axis_by_node[node]);

    // S.t += dt * v
    const Vec moved = add(row(sim, 3U), scale(velocity, dt));
    sim[12] = moved[0];
    sim[13] = moved[1];
    sim[14] = moved[2];

    // Axis toward the parent (types 3-5 away from it); reference = rest row.
    Vec d = sub(row(parent, 3U), moved);
    const Vec to_parent = d;
    if (axis >= 3U) d = scale(d, -1.0F);
    const Vec reference = (axis % 3U == 1U) ? row(target, 0U) : row(target, 1U);
    align_axis(sim, static_cast<std::uint8_t>(axis % 3U), d, reference);

    // W = Stiffness * target + (1 - Stiffness) * S (0x14032DA20).
    const float a = p.stiffness;
    const float b = 1.0F - p.stiffness;
    Mat world{};
    const Vec z = add(scale(row(target, 2U), a), scale(row(sim, 2U), b));
    const Vec y = add(scale(row(target, 1U), a), scale(row(sim, 1U), b));
    align_axis(world, 2U, z, y);
    const Vec t = add(scale(row(target, 3U), a), scale(row(sim, 3U), b));

    // Collision (0x1402C9714..0x1402C98F4): push the node out of every
    // capsule it is inside (0x1402D0630: closest point on a-b, then onto the
    // surface at the radius); any hit zeroes the x and z velocity.
    // Only the first chain of a part collides: 0x140214D50 parses every
    // ClothNo into consecutive chains (+0xA210, +0xA300, ...) but calls the
    // capsule setter 0x1402CA2F0 on +0xA210 alone (0x1402151E7).
    if (node < state.block_by_node.size() && state.block_by_node[node] != 0U) capsules = {};
    Vec t_hit = t;
    bool hit = false;
    for (const auto& capsule : capsules) {
        const Vec ab = sub(capsule.b, capsule.a);
        const float len2 = dot(ab, ab);
        float k = len2 > 0.0F ? dot(sub(t_hit, capsule.a), ab) / len2 : 0.0F;
        k = std::clamp(k, 0.0F, 1.0F);
        const Vec closest = add(capsule.a, scale(ab, k));
        const Vec out = sub(t_hit, closest);
        const float dist = length(out);
        if (!(capsule.radius > dist)) continue;
        const Vec dir = dist > 0.0F ? scale(out, 1.0F / dist) : Vec{0.0F, 0.0F, 1.0F};
        t_hit = add(closest, scale(dir, capsule.radius));
        hit = true;
    }
    if (hit) {
        velocity[0] = 0.0F;
        velocity[2] = 0.0F;
    }

    // Wind: parent-joint space when WindLocal, scaled by 10 * (1 - |cos|).
    Vec wind = p.wind;
    if (p.wind_local) {
        wind = add(add(scale(row(wind_parent_world, 0U), p.wind[0]),
                       scale(row(wind_parent_world, 1U), p.wind[1])),
                   scale(row(wind_parent_world, 2U), p.wind[2]));
    }
    const float dl = length(to_parent);
    const float wl = length(wind);
    if (dl > 0.0F && wl > 0.0F) {
        const float c = std::fabs(dot(scale(to_parent, 1.0F / dl), scale(wind, 1.0F / wl)));
        wind = scale(wind, (1.0F - c) * 10.0F);
    }
    Vec v = add(add(velocity, scale(wind, dt)), scale(p.gravity, dt));

    // Rest bone length and spring (0x1402C9A8C...0x1402C9C63).
    Vec e = sub(t_hit, row(parent, 3U));
    float le = dot(e, e);
    float lr = rest_length * rest_length;
    if (p.limit_length) {
        le = std::sqrt(le);
        lr = rest_length;
        if (le > 0.0F) e = scale(e, lr / le);
    }
    const Vec position = add(row(parent, 3U), e);
    if (lr > 0.0F) {
        v = sub(v, scale(e, (le - lr) / lr * p.spring_force * dt));
    }
    const float speed = length(v);
    if (speed > p.max_speed && speed > 0.0F) v = scale(v, p.max_speed / speed);
    v = scale(v, p.damping);

    world[12] = position[0];
    world[13] = std::max(position[1], p.floor_level);
    world[14] = position[2];
    world[15] = 1.0F;
    for (std::size_t k = 0U; k < 3U; ++k) world[k * 4U + 3U] = 0.0F;
    velocity = v;
    state.sim[node] = world;
    return world;
}

}  // namespace dmc::rengine::profiles::dmc3::motion

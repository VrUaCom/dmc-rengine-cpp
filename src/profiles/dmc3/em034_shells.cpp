#include "dmc_rengine/profiles/dmc3/em034_shells.hpp"

#include <algorithm>
#include <cmath>

namespace dmc::rengine::profiles::dmc3::em034 {

Vec3 cross(const Vec3& a, const Vec3& b) noexcept {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

float length_sq(const Vec3& v) noexcept { return v.x * v.x + v.y * v.y + v.z * v.z; }

Vec3 normalize(Vec3 v, Vec3 fallback) noexcept {
    float len2 = length_sq(v);
    if (!(len2 > 1.0e-15F) || !std::isfinite(len2)) {
        v = fallback;
        len2 = length_sq(v);
    }
    if (!(len2 > 1.0e-15F) || !std::isfinite(len2)) return {1.0F, 0.0F, 0.0F};
    const float inv = 1.0F / std::sqrt(len2);
    return {v.x * inv, v.y * inv, v.z * inv};
}

Vec3 robust_cross(Vec3 a, Vec3 b, bool a_cross_b) noexcept {
    const auto product = [&] { return a_cross_b ? cross(a, b) : cross(b, a); };
    Vec3 out = product();
    if (length_sq(out) > 1.0e-15F) return normalize(out, {1.0F, 0.0F, 0.0F});
    b.x += 0.1F;
    out = product();
    if (length_sq(out) <= 1.0e-15F) {
        b.y += 0.1F;
        out = product();
    }
    if (length_sq(out) <= 1.0e-15F) {
        b.z += 0.1F;
        out = product();
    }
    return normalize(out, {1.0F, 0.0F, 0.0F});
}

Vec3 rotate_point(const Vec3& v, const Matrix4& m) noexcept {
    const auto& a = m.values;
    return {v.x * a[0] + v.y * a[4] + v.z * a[8], v.x * a[1] + v.y * a[5] + v.z * a[9],
            v.x * a[2] + v.y * a[6] + v.z * a[10]};
}

namespace {

Matrix4 rows(const Vec3& r0, const Vec3& r1, const Vec3& r2, const Vec3& p) noexcept {
    Matrix4 out;
    out.values = {r0.x, r0.y, r0.z, 0.0F, r1.x, r1.y, r1.z, 0.0F,
                  r2.x, r2.y, r2.z, 0.0F, p.x,  p.y,  p.z,  1.0F};
    return out;
}

Vec3 translation(const Matrix4& m) noexcept { return {m.values[12], m.values[13], m.values[14]}; }

Vec3 add(const Vec3& a, const Vec3& b) noexcept { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 scale(const Vec3& v, float s) noexcept { return {v.x * s, v.y * s, v.z * s}; }

float dot(const Vec3& a, const Vec3& b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }

}  // namespace

Matrix4 align_z_matrix(Vec3 direction, Vec3 position) noexcept {
    direction = normalize(direction, {1.0F, 0.0F, 0.0F});
    const Vec3 row0 = robust_cross({0.0F, 1.0F, 0.0F}, direction, true);
    const Vec3 row1 = normalize(cross(direction, row0), {0.0F, 1.0F, 0.0F});
    return rows(row0, row1, direction, position);
}

Matrix4 align_x_matrix(Vec3 direction, Vec3 position) noexcept {
    direction = normalize(direction, {1.0F, 0.0F, 0.0F});
    const Vec3 row1 = robust_cross({0.0F, 1.0F, 0.0F}, direction, true);
    const Vec3 row2 = normalize(cross(direction, row1), {0.0F, 0.0F, 1.0F});
    return rows(direction, row1, row2, position);
}

Shl02Spawn shl02_spawn(const Matrix4& slot20_node0) noexcept {
    Shl02Spawn out;
    out.direction = normalize(rotate_point({1.0F, 0.0F, 0.0F}, slot20_node0), {1.0F, 0.0F, 0.0F});
    out.origin = add(translation(slot20_node0), kShl02InitOffset);
    return out;
}

Matrix4 shl02_world_at(const Shl02Spawn& spawn, float age, float hit_age) noexcept {
    const float end = hit_age >= 0.0F ? std::min(hit_age, kShl02Lifetime) : kShl02Lifetime;
    const float flight = std::clamp(age, 0.0F, end);
    return align_z_matrix(spawn.direction,
                          add(spawn.origin, scale(spawn.direction, kShl02Speed * flight)));
}

float shl02_explode_age(float hit_age) noexcept {
    return hit_age >= 0.0F ? std::ceil(hit_age) + 1.0F : kShl02Lifetime + 1.0F;
}

Shl03Spawn shl03_spawn(const Matrix4& component_node0, const Matrix4& component_node1) noexcept {
    const Vec3 offset = rotate_point({87.80000305F, 0.0F, 4.28000021F}, component_node0);
    const Vec3 spawn = add(offset, translation(component_node1));
    Shl03Spawn out;
    out.velocity = scale(rotate_point({1.0F, 0.0F, 0.0F}, component_node0), 50.0F);
    out.world = align_x_matrix(out.velocity, spawn);
    return out;
}

Vec3 gun_axis(const Matrix4& component_node0, const Vec3& axis) noexcept {
    return normalize(rotate_point(axis, component_node0), {0.0F, 0.0F, 1.0F});
}

Vec3 straight_shell_position(const Shell& shell, float age) noexcept {
    const float end = shell.hit_age >= 0.0F ? std::min(shell.hit_age, kStraightShellLifetime)
                                            : kStraightShellLifetime;
    return add(shell.origin, scale(shell.velocity, std::clamp(age, 0.0F, end)));
}

GrenadePose grenade_pose(const Shell& shell, float age, const SegmentRaycast& raycast) {
    GrenadePose pose{shell.origin, false};
    Vec3 velocity = shell.velocity;
    const float flight_end = std::floor(shell.lifetime) + 1.0F;  // state 2
    const float limit = std::clamp(age, 0.0F, flight_end);
    const auto whole = static_cast<int>(std::floor(limit));
    const auto bounce = [&](const Vec3& from) {
        if (raycast) {
            const auto hit = raycast(from, pose.position);
            if (!hit) return;
            const Vec3 n = normalize(hit->normal, {0.0F, 1.0F, 0.0F});
            const float over = dot({pose.position.x - hit->point.x, pose.position.y - hit->point.y,
                                    pose.position.z - hit->point.z},
                                   n);
            pose.position = add(pose.position, scale(n, -2.0F * over));
            const float vn = dot(velocity, n);
            velocity = scale(add(velocity, scale(n, -2.0F * vn)), kGrenadeRestitution);
        } else {
            if (!(pose.position.y < 0.0F && from.y >= 0.0F)) return;
            pose.position.y = -pose.position.y;
            velocity = {kGrenadeRestitution * velocity.x, -kGrenadeRestitution * velocity.y,
                        kGrenadeRestitution * velocity.z};
        }
        if (std::sqrt(length_sq(velocity)) < kGrenadeRestSpeed) pose.resting = true;
    };
    for (int tick = 1; tick <= whole; ++tick) {
        if (pose.resting) continue;
        const Vec3 previous = pose.position;
        pose.position = add(pose.position, velocity);
        velocity.y = std::min(velocity.y - kGrenadeGravity, kGrenadeMaxRise);
        bounce(previous);
    }
    const float fraction = limit - static_cast<float>(whole);
    if (fraction > 0.0F && !pose.resting && static_cast<float>(whole) < flight_end) {
        const Vec3 from = pose.position;
        const Vec3 to = add(from, scale(velocity, fraction));
        if (raycast) {
            const auto hit = raycast(from, to);
            pose.position = hit ? hit->point : to;
        } else {
            pose.position = {to.x, std::max(to.y, 0.0F), to.z};
        }
    }
    return pose;
}

float straight_shell_hit_tick(const Shell& shell) noexcept { return std::ceil(shell.hit_age) + 1.0F; }

float shell_retire_age(const Shell& shell) noexcept {
    if (shell.grenade) return std::floor(shell.lifetime) + 6.0F;
    if (shell.hit_age >= 0.0F) return straight_shell_hit_tick(shell) + 1.0F;
    return kStraightShellLifetime + 2.0F;
}

Matrix4 shell_world(const Shell& shell, float age, const SegmentRaycast& raycast) {
    if (shell.grenade) {
        Matrix4 out;
        const auto pose = grenade_pose(shell, age, raycast);
        out.values[12] = pose.position.x;
        out.values[13] = pose.position.y;
        out.values[14] = pose.position.z;
        return out;
    }
    return align_z_matrix(shell.velocity, straight_shell_position(shell, age));
}

}  // namespace dmc::rengine::profiles::dmc3::em034

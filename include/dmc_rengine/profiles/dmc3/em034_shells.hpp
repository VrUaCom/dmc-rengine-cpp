#pragma once

#include <functional>
#include <optional>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// CEm034 (Lady) model-less shells: spawn poses and flight paths read from
// dmc3.exe. See docs/research/dmc3-native-reader-code-facts-2026-10-01.md and
// the Reader's docs/research/dmc3-shell-effect-runtime-exe-v73.md. Everything
// here is a pure function of joint matrices and the shell's age in ticks;
// steering toward the player (Shl02 after its first retarget) needs gameplay
// state and is not modeled.
namespace dmc::rengine::profiles::dmc3::em034 {

// --- shared math --------------------------------------------------------

[[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) noexcept;
[[nodiscard]] float length_sq(const Vec3& v) noexcept;
[[nodiscard]] Vec3 normalize(Vec3 v, Vec3 fallback) noexcept;
// Normalized a x b (or b x a); when the vectors are nearly parallel the EXE
// perturbs the reference axis in 0.1 steps (x, then y, then z).
[[nodiscard]] Vec3 robust_cross(Vec3 a, Vec3 b, bool a_cross_b) noexcept;
// (x,y,z,1) * m with row 3 forced to (0,0,0,1) (0x14016F610 / 0x14016CBC8).
[[nodiscard]] Vec3 rotate_point(const Vec3& v, const Matrix4& m) noexcept;
// 0x14032FD90(out, dir, ref (0,1,0)): row0 = normalize(up x dir),
// row1 = normalize(dir x row0), row2 = dir, row3 = position.
[[nodiscard]] Matrix4 align_z_matrix(Vec3 direction, Vec3 position) noexcept;
// 0x14032F1D0: row0 = dir, row1 = normalize(up x dir), row2 = normalize(dir x row1).
[[nodiscard]] Matrix4 align_x_matrix(Vec3 direction, Vec3 position) noexcept;

// --- Shl02 (homing missile) ----------------------------------------------

// Flight table 0x14057B4E0 (loaded by 0x140244940) and init offset 0x14057BB20.
inline constexpr float kShl02Speed = 30.0F;               // shell+0x160 per tick
inline constexpr float kShl02Lifetime = 120.0F;           // shell+0x17C
inline constexpr float kShl02RetargetInterval = 10.0F;    // shell+0x180/+0x184
inline constexpr Vec3 kShl02InitOffset{18.6F, 0.0F, 12.0F};
// State 2 (0x140173800): +0xD68 = 3.0 counts down; state 3 once negative.
inline constexpr float kShl02ExplodeTicks = 4.0F;

struct Shl02Spawn final {
    Vec3 origin{};
    Vec3 direction{};  // unit, shell+0x140 after 0x140330390
};
// CEm034 0x140169937: direction = X axis of the slot-20 joint; origin =
// joint translation + (18.6,0,12) in world axes (init 0x1401738F0).
[[nodiscard]] Shl02Spawn shl02_spawn(const Matrix4& slot20_node0) noexcept;
// State 1 (0x140173C60 -> CShell::move 0x140244870): straight flight at 30
// per tick until the lifetime ends or the stage hit age (negative = none).
[[nodiscard]] Matrix4 shl02_world_at(const Shl02Spawn& spawn, float age,
                                     float hit_age = -1.0F) noexcept;
// Age of the state-2 entry: lifetime + 1, or the update after a stage hit.
[[nodiscard]] float shl02_explode_age(float hit_age = -1.0F) noexcept;

// --- Shl03 (rocket from the launcher) --------------------------------------

struct Shl03Spawn final {
    Matrix4 world{};
    Vec3 velocity{};
};
// 0x14016CBB0..0x14016CC41: spawn = (87.8,0,4.28) rotated by node0 +
// node1 translation; velocity = 50 * node0 X axis; basis 0x14032F1D0.
[[nodiscard]] Shl03Spawn shl03_spawn(const Matrix4& component_node0,
                                     const Matrix4& component_node1) noexcept;

// --- Shl00 / Shl05 straight shells, Shl04 grenade ------------------------

// Shl00/Shl05 (0x140172590 / 0x140175ED0): pos += vel * dt, +0x52C = 120.
inline constexpr float kStraightShellLifetime = 120.0F;
// CEm034 init 0x14016FEC2: em+0x59F0 = 35 (pistol and Shl05 speed).
inline constexpr float kLadyGunSpeed = 35.0F;
// 0x140171C70: SMG shots use a fixed 45 (.rdata 0x14057B428).
inline constexpr float kLadySmgSpeed = 45.0F;
// Shl04 (0x1401756E0) and the bounce in 0x1402C64F0.
inline constexpr float kGrenadeGravity = 1.0F;
inline constexpr float kGrenadeMaxRise = 30.0F;
inline constexpr float kGrenadeRestitution = 0.5F;
inline constexpr float kGrenadeRestSpeed = 10.0F;
inline constexpr float kGrenadeWarnFuse = 60.0F;

struct Shell final {
    Vec3 origin{};
    Vec3 velocity{};
    float lifetime{};       // Shl04 fuse
    float hit_age{-1.0F};   // straight shells: age of the stage hit, < 0 = none
    bool grenade{};         // Shl04
};

// 0x14016F780: (axis, 1) * gun component node world without translation,
// normalized: the shot direction.
[[nodiscard]] Vec3 gun_axis(const Matrix4& component_node0, const Vec3& axis) noexcept;

[[nodiscard]] Vec3 straight_shell_position(const Shell& shell, float age) noexcept;

struct RayHit final {
    Vec3 point{};
    Vec3 normal{};
};
// Stage raycast from -> to (0x14005E7A0, category mask 0 for a shell).
using SegmentRaycast = std::function<std::optional<RayHit>(const Vec3& from, const Vec3& to)>;

struct GrenadePose final {
    Vec3 position{};
    bool resting{};
};
// One EXE update per whole tick; the stage hit mirrors the rest of the move
// about the hit plane and reflects + halves the velocity. Without a raycast
// the plane y = 0 stands in for the floor.
[[nodiscard]] GrenadePose grenade_pose(const Shell& shell, float age,
                                       const SegmentRaycast& raycast = {});

// Tick the update after a straight shell's stage hit sees it (state 2).
[[nodiscard]] float straight_shell_hit_tick(const Shell& shell) noexcept;
// Age at which the shell is retired: grenade fuse + 6 (state 2, explode,
// 3.0 countdown, state 3), straight hit tick + 1, else 122.
[[nodiscard]] float shell_retire_age(const Shell& shell) noexcept;
// Shell world: grenade = translation only (its visual is a billboard);
// straight shells = align_z_matrix(velocity, position).
[[nodiscard]] Matrix4 shell_world(const Shell& shell, float age,
                                  const SegmentRaycast& raycast = {});

}  // namespace dmc::rengine::profiles::dmc3::em034

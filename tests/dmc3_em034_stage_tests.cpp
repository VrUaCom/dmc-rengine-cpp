#include <cassert>
#include <cmath>
#include <cstdio>
#include <optional>

#include "dmc_rengine/profiles/dmc3/em034_shells.hpp"
#include "dmc_rengine/profiles/dmc3/stage_layout.hpp"

namespace em034 = dmc::rengine::profiles::dmc3::em034;
namespace layout = dmc::rengine::profiles::dmc3::stage_layout;
using dmc::rengine::profiles::dmc3::Matrix4;
using dmc::rengine::profiles::dmc3::Vec3;

namespace {

bool near(float a, float b, float eps = 1.0e-4F) { return std::fabs(a - b) <= eps; }

bool near3(const Vec3& a, const Vec3& b, float eps = 1.0e-4F) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}

Vec3 row(const Matrix4& m, int r) {
    const auto i = static_cast<std::size_t>(r * 4);
    return {m.values[i], m.values[i + 1U], m.values[i + 2U]};
}

// Rotation of 90 degrees about Y: X axis -> (0,0,-1).
Matrix4 yaw90(Vec3 t) {
    Matrix4 m;
    m.values = {0.0F, 0.0F, -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F,
                1.0F, 0.0F, 0.0F,  0.0F, t.x,  t.y,  t.z,  1.0F};
    return m;
}

}  // namespace

int main() {
    // align_z: row2 = dir, rows orthonormal; the parallel case still works.
    {
        const auto m = em034::align_z_matrix({0.0F, 0.0F, 2.0F}, {1.0F, 2.0F, 3.0F});
        assert(near3(row(m, 2), {0.0F, 0.0F, 1.0F}));
        assert(near3(row(m, 0), {1.0F, 0.0F, 0.0F}));  // up x z
        assert(near3(row(m, 1), {0.0F, 1.0F, 0.0F}));
        assert(near3(row(m, 3), {1.0F, 2.0F, 3.0F}));
        const auto up = em034::align_z_matrix({0.0F, 1.0F, 0.0F}, {});
        assert(near(em034::length_sq(row(up, 0)), 1.0F));
        assert(near(row(up, 0).x * 0.0F + row(up, 0).y, 0.0F));
    }

    // Shl02: origin = translation + (18.6,0,12) unrotated, direction = X axis.
    {
        const auto spawn = em034::shl02_spawn(yaw90({100.0F, 50.0F, -20.0F}));
        assert(near3(spawn.direction, {0.0F, 0.0F, -1.0F}));
        assert(near3(spawn.origin, {118.6F, 50.0F, -8.0F}));
        const auto at10 = em034::shl02_world_at(spawn, 10.0F);
        assert(near3(row(at10, 3), {118.6F, 50.0F, -308.0F}, 1.0e-3F));
        const auto late = em034::shl02_world_at(spawn, 500.0F);
        assert(near(late.values[14], -8.0F - 30.0F * 120.0F, 1.0e-2F));
        const auto hit = em034::shl02_world_at(spawn, 50.0F, 4.5F);
        assert(near(hit.values[14], -8.0F - 135.0F, 1.0e-3F));
        assert(em034::shl02_explode_age() == 121.0F);
        assert(em034::shl02_explode_age(4.5F) == 6.0F);
    }

    // Shl03: offset rotated by node0, node1 translation, velocity 50 * X.
    {
        const auto spawn = em034::shl03_spawn(yaw90({}), yaw90({10.0F, 20.0F, 30.0F}));
        assert(near3(spawn.velocity, {0.0F, 0.0F, -50.0F}));
        // (87.8, 0, 4.28) * yaw90 = (4.28, 0, -87.8).
        assert(near3(row(spawn.world, 3), {14.28F, 20.0F, -57.8F}, 1.0e-3F));
        assert(near3(row(spawn.world, 0), {0.0F, 0.0F, -1.0F}));
    }

    // Straight shells and retire ages.
    {
        em034::Shell shot{{0.0F, 0.0F, 0.0F}, {em034::kLadyGunSpeed, 0.0F, 0.0F}};
        assert(near3(em034::straight_shell_position(shot, 2.0F), {70.0F, 0.0F, 0.0F}));
        assert(near(em034::straight_shell_position(shot, 1000.0F).x, 35.0F * 120.0F));
        assert(em034::shell_retire_age(shot) == 122.0F);
        shot.hit_age = 3.2F;
        assert(near(em034::straight_shell_position(shot, 10.0F).x, 35.0F * 3.2F, 1.0e-3F));
        assert(em034::straight_shell_hit_tick(shot) == 5.0F);
        assert(em034::shell_retire_age(shot) == 6.0F);
        assert(near3(em034::gun_axis(yaw90({5.0F, 5.0F, 5.0F}), {2.0F, 0.0F, 0.0F}), {0.0F, 0.0F, -1.0F}));
    }

    // Grenade: gravity 1/tick^2, floor bounce halves the velocity, rest < 10.
    {
        em034::Shell grenade{{0.0F, 10.0F, 0.0F}, {4.0F, 5.0F, 0.0F}, 200.0F, -1.0F, true};
        const auto t1 = em034::grenade_pose(grenade, 1.0F);
        assert(near3(t1.position, {4.0F, 15.0F, 0.0F}));
        const auto t2 = em034::grenade_pose(grenade, 2.0F);
        assert(near3(t2.position, {8.0F, 19.0F, 0.0F}));  // vy 5 -> 4
        const auto rest = em034::grenade_pose(grenade, 199.0F);
        assert(rest.resting && rest.position.y >= 0.0F);
        // The raycast path mirrors about the hit plane like the floor fallback.
        const em034::SegmentRaycast floor = [](const Vec3& from, const Vec3& to) -> std::optional<em034::RayHit> {
            if (!(from.y >= 0.0F && to.y < 0.0F)) return std::nullopt;
            const float f = from.y / (from.y - to.y);
            return em034::RayHit{{from.x + (to.x - from.x) * f, 0.0F, from.z + (to.z - from.z) * f},
                                 {0.0F, 1.0F, 0.0F}};
        };
        for (float age = 0.0F; age <= 30.0F; age += 1.0F) {
            const auto a = em034::grenade_pose(grenade, age);
            const auto b = em034::grenade_pose(grenade, age, floor);
            assert(near3(a.position, b.position, 1.0e-3F) && a.resting == b.resting);
        }
        assert(em034::shell_retire_age(grenade) == 206.0F);
        const auto world = em034::shell_world(grenade, 1.0F);
        assert(near(world.values[13], 15.0F) && world.values[0] == 1.0F);
    }

    // Stage layout text.
    {
        const char* text =
            "# GAME\n"
            "# SET 0 CONFIG\n"
            "  cam_init 1.5, 2, -3 ; camera\n"
            "# SET 1 MODEL\n"
            "  model 4\n"
            "  pos 100, 0, -50\n"
            "  rot 0, 90, 0\n"
            "  scale 2, 2, 2\n"
            "  uv 0, 1, 0.5, -0.25\n"
            "  eff V 98\n"
            "  epos 1, 2, 3\n"
            "# SET 3 BREAK ; drum\n"
            "  model 3\n"
            "  bmodel 4\n"
            "  eff V 98\n"
            "  epos 0.0, 130.0, 0.0\n"
            "  beff V 104\n"
            "  remain on ; stays\n"
            "  pos 2100, 15, 3050\n"
            "# SET 4 BREAK\n"
            "  model 1\n"
            "  beff V 122\n"
            "  epos 1, 2, 3\n"
            "# GAME_END\n"
            "# SET 2 IGNORED\n";
        const auto game = layout::parse_game(text);
        assert(game.has_camera && near3(game.camera, {1.5F, 2.0F, -3.0F}));
        assert(game.sets.size() == 4U);
        const auto& set = game.sets[1];
        assert(set.kind == "MODEL" && set.model == 4);
        assert(near3(set.pos, {100.0F, 0.0F, -50.0F}) && near3(set.rot, {0.0F, 90.0F, 0.0F}));
        assert(set.uv.size() == 1U && set.uv[0][3] == -0.25F);
        assert(set.effect_kind == 'V' && set.effect_id == 98 && near3(set.effect_pos, {1.0F, 2.0F, 3.0F}));
        // Y by 90 degrees: (1,0,0) -> (0,0,-1), scaled 2, moved.
        assert(near3(layout::place_point(set, {1.0F, 0.0F, 0.0F}), {100.0F, 0.0F, -52.0F}, 1.0e-3F));
        assert(near3(layout::place_normal(set, {0.0F, 0.0F, 1.0F}), {1.0F, 0.0F, 0.0F}, 1.0e-5F));
        assert(layout::numbers("1, 2 x ; 3").size() == 2U);
        const auto& drum = game.sets[2];
        assert(drum.kind == "BREAK" && drum.model == 3 && drum.broken_model == 4 && drum.remain == 1U);
        assert(drum.effect_id == 98 && near3(drum.effect_pos, {0.0F, 130.0F, 0.0F}));
        assert(drum.broken_effect_kind == 'V' && drum.broken_effect_id == 104 && near3(drum.broken_effect_pos, {}));
        const auto intact = layout::object_state(drum, false), broken = layout::object_state(drum, true);
        assert(intact.model == 3 && intact.effect_id == 98 && !intact.once);
        assert(broken.model == 4 && broken.effect_id == 104 && broken.once);
        const auto& crate = game.sets[3];
        assert(crate.remain == 0U && crate.broken_model == -1 && near3(crate.broken_effect_pos, {1.0F, 2.0F, 3.0F}));
        assert(near3(crate.effect_pos, {}) && layout::object_state(crate, true).model == -1);
        assert(layout::object_state(set, true).model == 4);  // not a BREAK block
    }

    std::printf("dmc3 em034/stage ok\n");
    return 0;
}

// Effect parent modes and the enemy event tables recovered from dmc3.exe
// (docs/research/dmc3-effect-triggers-2026-10-01.md).
#include "dmc_rengine/profiles/dmc3/fx/enemy_events.hpp"
#include "dmc_rengine/profiles/dmc3/fx/effect_runtime.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    namespace fx = dmc::rengine::profiles::dmc3::fx;
    using fx::runtime::MatrixMode;

    fx::Matrix4 m;
    m.values = {2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4, 0, 10, 20, 30, 1};
    const auto pos = fx::runtime::apply_mode(m, MatrixMode::PositionOnly);
    assert(pos.values[0] == 1.0F && pos.values[5] == 1.0F && pos.values[13] == 20.0F);
    const auto rot = fx::runtime::apply_mode(m, MatrixMode::RotationOnly);
    assert(rot.values[0] == 2.0F && rot.values[12] == 0.0F);
    const auto norm = fx::runtime::apply_mode(m, MatrixMode::Normalized);
    assert(norm.values[0] == 1.0F && norm.values[10] == 1.0F && norm.values[14] == 30.0F);
    const auto lifted = fx::runtime::ca0_spawn_matrix(nullptr);
    assert(lifted.values[13] == 2.0F && lifted.values[0] == 1.0F);
    assert(fx::runtime::v_entry_spawn_age(-3) == 1.0F && fx::runtime::v_entry_spawn_age(4) == 5.0F);
    assert(fx::runtime::e_alive(23.0F, 23, false) && !fx::runtime::e_alive(23.5F, 23, false));

    const auto* c3 = fx::enemy::find_event(3U);
    assert(c3 != nullptr && c3->spawns.size() == 2U);
    assert(c3->spawns[0].id == 42U && c3->spawns[0].object == 1U && c3->spawns[0].scale == 2.0F);
    assert(c3->spawns[0].parent_mode == MatrixMode::PositionOnly);
    const auto* death = fx::enemy::find_event(0xC8U, 0x1AU);
    assert(death != nullptr && death->spawns.size() == 2U && death->spawns[0].id == 302U);
    const auto* plain = fx::enemy::find_event(0xC8U, 0U);
    assert(plain != nullptr && plain->spawns[0].id == 315U);
    assert(fx::enemy::find_event(0x3E4U) == nullptr);
    for (const auto& c : fx::enemy::event_cases()) assert(c.code < fx::enemy::kFirstControlCode && !c.spawns.empty());

    const auto death_steps = fx::enemy::em000_death_schedule();
    assert(death_steps.size() == 6U && death_steps[5].tick == 25.0F && death_steps[5].codes[0] == 0xCEU);
    assert(fx::enemy::command_tables().size() == 8U);
    for (const auto& a : fx::enemy::em000_attack_events()) assert(a.code == 3U && a.bank == 0U);
    std::printf("dmc3 fx enemy events ok\n");
    return 0;
}

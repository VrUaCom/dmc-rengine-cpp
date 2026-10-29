// The effect spawn-site catalog and the em000 event tables
// (profiles/dmc3/effect_spawn_sites.hpp, em000_events.hpp): known rows from
// the reverse notes are where they should be.
#include "dmc_rengine/profiles/dmc3/effect_spawn_sites.hpp"
#include "dmc_rengine/profiles/dmc3/em000_events.hpp"

#include <algorithm>
#include <cassert>
#include <string_view>

namespace sites = dmc::rengine::profiles::dmc3::effect_spawn_sites;
namespace em000 = dmc::rengine::profiles::dmc3::em000_events;

int main() {
    const auto all = sites::spawn_sites();
    assert(all.size() > 1000U);
    for (const auto& site : all) {
        assert(site.kind == 'P' || site.kind == 'E' || site.kind == 'G' || site.kind == 'V' || site.kind == '?');
        assert(site.function <= site.address && sites::call_target(site) != 0U);
    }
    // CDrawUV starts V 34 (its first row), and CGenerator E 63.
    const auto v34 = sites::owners('V', 34);
    assert(std::find(v34.begin(), v34.end(), std::string_view{"CDrawUV"}) != v34.end());
    const auto e63 = sites::owners('E', 63);
    assert(std::find(e63.begin(), e63.end(), std::string_view{"CGenerator"}) != e63.end());

    // Code 3 (the scythe): E 42 and V 42, scale 2, following body joint 1 by position.
    int code3 = 0;
    for (const auto& spawn : em000::handler_spawns()) {
        if (spawn.code != 3U || spawn.type_1a) continue;
        assert((spawn.kind == 'E' || spawn.kind == 'V') && spawn.id == 42U && spawn.scale == 2.0F);
        assert(spawn.place == em000::Place::joint && spawn.joint == 1U && spawn.mode == 1U);
        ++code3;
    }
    assert(code3 == 2);
    // Command 0x08 of table 1 sends code 3 at frame 50.
    const auto events = em000::command_events();
    assert(std::any_of(events.begin(), events.end(), [](const em000::CommandEvent& e) {
        return e.table == 1U && e.command == 0x08U && e.frame == 50.0F && e.code == 0x03U;
    }));
    const auto tables = em000::class_tables();
    assert(tables.size() >= 5U && tables[0].name == "CEm000");
    return 0;
}

#include "dmc_rengine/profiles/dmc3/player_attachment_contract.hpp"

#include <cassert>

int main() {
    namespace pa = dmc::rengine::profiles::dmc3::player_attachment;
    static_assert(pa::coat_slot == 12U && pa::texture_slot == 0U && pa::body_slot == 1U);
    static_assert(pa::coat_host_joint == 3U);
    static_assert(pa::form_joint_base[1] == 24 && pa::form_joint_base[2] == 48);

    constexpr auto rebellion = pa::weapon_record_for_stem("plwp_sword");
    static_assert(rebellion.has_value());
    static_assert(rebellion->joint == 3U);
    static_assert(rebellion->translation[1] == 32.0F);
    static_assert(rebellion->class_name == "CPlWpSword");

    constexpr auto yamato = pa::weapon_record_for_stem("plwp_vergilsword");
    static_assert(yamato.has_value() && yamato->joint == 13U);
    static_assert(!pa::weapon_record_for_stem("plwp_gun").has_value());

    constexpr auto pair = pa::second_part_for_class("CPlWp2Sword");
    static_assert(pair.has_value() && pair->first_node == 2U && pair->second_node == 1U);
    static_assert(pair->joint == 3U && pair->translation[0] == -13.0F);
    static_assert(!pa::second_part_for_class("CPlWpSword").has_value());

    for (const auto& record : pa::weapon_state0_records) {
        assert(record.pac_stem.starts_with("plwp_"));
        assert(record.attach_table_va >= 0x140553000ULL);   // .data
        assert(record.state0_record_va >= 0x140553000ULL);
    }
    return 0;
}

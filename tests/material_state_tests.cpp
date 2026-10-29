#include "dmc_rengine/profiles/dmc3/material_state.hpp"

#include <cassert>
#include <cstdint>

namespace gm = dmc::rengine::profiles::dmc3::material_state;

namespace {

void test_nearest_filter() {
    assert(gm::nearest_filter(0x4000));
    assert(!gm::nearest_filter(0x3FFF));
}

void test_legacy_clamp_register() {
    assert(!gm::legacy_clamp_register(0, 5, 6, 7).has_value());

    const auto reg = gm::legacy_clamp_register(1, 2, 3, 4);
    assert(reg.has_value());
    assert(*reg == (1ull << 4 | 0xFull | 2ull << 14 | 3ull << 24 | 4ull << 34));

    // Words above 0x3FF are not masked.
    const auto wide = gm::legacy_clamp_register(0x400, 2, 3, 4);
    assert(wide.has_value());
    assert(*wide == (0x400ull << 4 | 0xFull | 2ull << 14 | 3ull << 24 | 4ull << 34));
}

void test_addressing() {
    assert(gm::address_for_mode(0) == gm::Address::wrap);
    assert(gm::address_for_mode(1) == gm::Address::clamp);
    assert(gm::address_for_mode(2) == gm::Address::clamp);
    assert(gm::address_for_mode(3) == gm::Address::wrap);

    const auto none = gm::addressing(0, 5, 6, 7);
    assert(none.u == gm::Address::wrap && none.v == gm::Address::wrap);

    for (std::uint16_t min_u : {std::uint16_t{1}, std::uint16_t{0x3FF},
                                std::uint16_t{0x400}, std::uint16_t{0xFFFF}}) {
        const auto a = gm::addressing(min_u, 0xFFFF, 0xFFFF, 0xFFFF);
        assert(a.u == gm::Address::wrap && a.v == gm::Address::wrap);
    }
}

void test_part_alpha() {
    assert(gm::effective_alpha_control(0x2, 0xC1) == 0x80);
    assert(gm::effective_alpha_control(0x4000, 0xC1) == 0xC1);

    assert(gm::part_alpha(0x80) == 128.0f / 255.0f);
    assert(gm::part_alpha(0xC5) == 1.0f);
    assert(gm::part_alpha(0x40) == 64.0f / 255.0f);

    assert(!gm::alpha_override(0x80));
    assert(gm::alpha_override(0x81));
}

void test_material_byte() {
    // Blend 2 stays 2.
    const auto additive = gm::material_byte(0x2, 0);
    assert(gm::material_blend(additive) == 2);
    assert(!gm::material_nearest(additive));
    assert(gm::material_part_alpha(additive) == 128.0f / 255.0f);

    // 0x4000 sets the nearest bit.
    const auto nearest = gm::material_byte(0x4000, 0);
    assert(gm::material_nearest(nearest));
    assert(gm::material_blend(nearest) == 0);

    // Control 0xC3 with blend 0 sets the override.
    const auto over = gm::material_byte(0x0, 0xC3);
    assert(gm::material_part_alpha(over) == 1.0f);
    assert(gm::material_blend(over) == 0);

    // The same with blend 2 does not: the control is forced to 0x80.
    const auto forced = gm::material_byte(0x2, 0xC3);
    assert(gm::material_part_alpha(forced) == 128.0f / 255.0f);
    assert(gm::material_blend(forced) == 2);

    // A low control is not carried: reads back as 128/255.
    assert(gm::material_part_alpha(gm::material_byte(0x0, 0x40)) == 128.0f / 255.0f);

    // Bits 6..7 stay zero.
    assert((gm::material_byte(0xFFFFFFFFu, 0xFF) & 0xC0) == 0);
}

} // namespace

int main() {
    test_nearest_filter();
    test_legacy_clamp_register();
    test_addressing();
    test_part_alpha();
    test_material_byte();
    return 0;
}

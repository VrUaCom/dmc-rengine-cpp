#pragma once

#include <array>
#include <cstdint>
#include <string_view>

// em000.pac is shared by five enemy classes. Evidence:
// docs/research/dmc3-em000-family-assembly-2026-09-23.md
// (dmc3.exe SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082).
namespace dmc::rengine::profiles::dmc3::em000_family {

struct ClothPart final {
    std::uint8_t model_slot;
    std::uint8_t clt_slot;    // ";emNNN_0K.clt" text
    std::uint8_t body_joint;  // [this+0x3254] / [this+0x3258]
};

struct EnemyClass final {
    std::string_view class_name;
    std::uint64_t vtable_va;
    std::uint64_t init_va;     // secondary vtable (+0x60) entry +0xE8
    std::uint8_t body_slot;    // with PTX slot 0
    std::array<ClothPart, 2> cloth;
    std::uint8_t cloth_count;
    std::uint8_t weapon_slot_variant01;  // [this+0x670] in 0..1
    std::uint8_t weapon_slot_variant23;  // [this+0x670] in 2..3
    std::uint8_t weapon_texture_slot;
    std::array<float, 3> weapon_translation;    // this+0x28B0
    std::array<float, 3> weapon_rotation_zyx;   // this+0x28C0, 0x1403304A0 order
};

inline constexpr std::uint8_t weapon_body_joint = 9U;  // [this+0x720]
inline constexpr std::uint8_t motion_bank_slot = 35U;
inline constexpr std::uint8_t effect_bank_slot = 41U;

inline constexpr std::array<float, 3> hell_weapon_t{-15.0F, -61.39939880371094F,
                                                    -18.93269920349121F};
inline constexpr std::array<float, 3> hell_weapon_r{0.20725786685943604F, 0.0F, 0.0F};

inline constexpr std::array<EnemyClass, 5> classes{{
    {"CEm000", 0x1404C9AF8ULL, 0x140097B40ULL, 1U, {{{3U, 2U, 14U}, {0U, 0U, 0U}}}, 1U,
     26U, 29U, 25U, hell_weapon_t, hell_weapon_r},
    {"CEm001", 0x1404CA1A8ULL, 0x14009CF70ULL, 5U, {{{7U, 6U, 14U}, {0U, 0U, 0U}}}, 1U,
     28U, 31U, 25U, hell_weapon_t, hell_weapon_r},
    {"CEm002", 0x1404CA4A0ULL, 0x1400A1D80ULL, 8U, {{{10U, 9U, 8U}, {12U, 11U, 12U}}}, 2U,
     26U, 29U, 25U, hell_weapon_t, hell_weapon_r},
    {"CEm003", 0x1404CA790ULL, 0x1400A6BD0ULL, 13U, {{{15U, 14U, 14U}, {17U, 16U, 14U}}}, 2U,
     27U, 30U, 25U, hell_weapon_t, hell_weapon_r},
    {"CEm004", 0x1404CAA90ULL, 0x1400A85E0ULL, 18U, {{{0U, 0U, 0U}, {0U, 0U, 0U}}}, 0U,
     34U, 34U, 32U, {2.0F, 20.0F, -72.0F},
     {-0.03490658476948738F, 0.10471975803375244F, 1.6580626964569092F}},
}};

}  // namespace dmc::rengine::profiles::dmc3::em000_family

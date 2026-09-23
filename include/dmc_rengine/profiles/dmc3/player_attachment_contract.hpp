#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

// IPlayer model attachment contract recovered from dmc3.exe
// (SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082).
// Evidence: docs/research/dmc3-player-coat-attachment-2026-09-23.md and
// docs/research/dmc3-player-weapon-attachment-2026-09-23.md.
namespace dmc::rengine::profiles::dmc3::player_attachment {

// Player PAC slots read through 0x1401B82C0 by CPlVergil/CPlDante/CPlNewVergil.
inline constexpr std::uint32_t texture_slot = 0U;
inline constexpr std::uint32_t body_slot = 1U;
inline constexpr std::uint32_t coat_slot = 12U;
inline constexpr std::uint32_t coat_cloth_slot = 13U;

// Coat root = identity root local x body joint[3] world (vtbl+0x190).
inline constexpr std::uint32_t coat_host_joint = 3U;

// Joint-table base per player form (0x1401FAA90 reads .rdata 0x1404E0328).
inline constexpr std::array<std::int32_t, 3> form_joint_base{0, 24, 48};

// One CPlayerWeapon attach record (0x1401FD8F0): byte +3 = joint index,
// +0x10 translation, +0x20 XYZ Euler radians. The weapon root is
// local(translation, rotation) x player.joint(formBase + joint)->world, the
// local built like the MOD rest local (0x140330450 then 0x140031200).
struct WeaponAttachRecord final {
    std::string_view class_name;
    std::string_view pac_stem;          // obj\<stem>.pac
    std::uint64_t attach_table_va;      // weapon +0x128
    std::uint64_t state0_record_va;
    std::uint8_t joint;
    std::array<float, 3> translation;
    std::array<float, 3> rotation_xyz_radians;
};

// State-0 (idle / sheathed) record of each class whose vtable[1] installs an
// attach table. Class -> PAC stem pairing follows the executable's own names
// (CPlWpSword / plwp_sword.pac ...); it is a naming correspondence, not a
// traced load path.
inline constexpr std::array<WeaponAttachRecord, 8> weapon_state0_records{{
    {"CPlWpSword", "plwp_sword", 0x14058C010ULL, 0x14058BF50ULL, 3U,
     {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWp2Sword", "plwp_2sword", 0x14058C800ULL, 0x14058C1A0ULL, 3U,
     {16.0F, -43.0F, -15.0F}, {-1.6057028770446777F, 0.0F, 0.2617993950843811F}},
    {"CPlWpGuitar", "plwp_guitar", 0x14058DAE0ULL, 0x14058D210ULL, 3U,
     {-30.0F, -80.0F, -23.0F},
     {-1.5009831190109253F, -0.11344639956951141F, -0.5235987901687622F}},
    {"CPlWpLaser", "plwp_laser", 0x14058E380ULL, 0x14058E260ULL, 8U,
     {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    {"CPlWpFoeceEdge", "plwp_forceedge", 0x14058ED30ULL, 0x14058ECA0ULL, 3U,
     {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWpNeroSword", "plwp_nerosword", 0x14058FAF0ULL, 0x14058FA90ULL, 3U,
     {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWpVergilSword", "plwp_vergilsword", 0x14058F280ULL, 0x14058EEC0ULL, 13U,
     {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
    {"CPlWpNewVergilSword", "plwp_newvergilsword", 0x14058F770ULL, 0x14058F410ULL, 13U,
     {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
}};

[[nodiscard]] constexpr std::optional<WeaponAttachRecord> weapon_record_for_stem(
    std::string_view stem) noexcept {
    for (const auto& record : weapon_state0_records) {
        if (record.pac_stem == stem) return record;
    }
    return std::nullopt;
}

// Records are 0x60 bytes: part 0 (+0x03 joint, +0x10 T, +0x20 R) and part 1
// (+0x31 joint, +0x40 T, +0x50 R); 0x1401FDA80 builds both locals, while
// 0x1401FD8F0 builds one of them. CPlWp2Sword (Agni & Rudra) is one MOD:
// pose 0x140227CF0 sets node 2 = local(part 0) x joint(+0x114), node 1 =
// local(part 1) x joint(+0x115) and node 0 = player world (player +0x180).
struct WeaponSecondPart final {
    std::string_view class_name;
    std::uint64_t pose_function_va;
    std::uint8_t first_node;
    std::uint8_t second_node;
    std::uint8_t joint;
    std::array<float, 3> translation;
    std::array<float, 3> rotation_xyz_radians;
};

inline constexpr std::array<WeaponSecondPart, 1> weapon_state0_second_parts{{
    {"CPlWp2Sword", 0x140227CF0ULL, 2U, 1U, 3U, {-13.0F, 32.0F, -14.0F},
     {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
}};

[[nodiscard]] constexpr std::optional<WeaponSecondPart> second_part_for_class(
    std::string_view class_name) noexcept {
    for (const auto& part : weapon_state0_second_parts) {
        if (part.class_name == class_name) return part;
    }
    return std::nullopt;
}

// Weapon ids. Factory 0x1401DED20 switches on the melee slots (player
// +0x6498, jump table 0x1401DEF64) and gun slots (+0x649A, table 0x1401DEFA4);
// LadyGun is created with dl = id. Dante's loader 0x1401DF6BE loads
// motion\\pl000\\pl000_00_N.pac with N = byte 0x14058ABC8[id * 4] through
// 0x1401B90B0 (per-character path lists at 0x1405B0F30).
struct WeaponId final {
    std::uint8_t id;
    std::string_view class_name;
    std::uint8_t pl000_motion_file;
};

inline constexpr std::array<WeaponId, 15> weapon_ids{{
    {0U, "CPlWpSword", 3U},        {1U, "CPlWpNunchaku", 4U},
    {2U, "CPlWp2Sword", 5U},       {3U, "CPlWpGuitar", 6U},
    {4U, "CPlWpFight", 7U},        {5U, "CPlWpGun", 8U},
    {6U, "CPlWpShotGun", 9U},      {7U, "CPlWpLaser", 10U},
    {8U, "CPlWpRifle", 11U},       {9U, "CPlWpLadyGun", 12U},
    {10U, "CPlWpLadyGun", 27U},    {11U, "CPlWpNewVergilSword", 28U},
    {12U, "CPlWpFight", 29U},      {13U, "CPlWpFoeceEdge", 30U},
    {14U, "CPlWpVergilSword", 31U},
}};

} // namespace dmc::rengine::profiles::dmc3::player_attachment

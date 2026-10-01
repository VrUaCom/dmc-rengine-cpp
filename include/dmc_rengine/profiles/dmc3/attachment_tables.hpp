#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/profiles/dmc3/cloth_chain.hpp"
#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"


namespace dmc::rengine::profiles::dmc3::motion {

// One node of an attached part driven by a host joint (mode 1 of 0x1402CBBE0:
// node world = offset x host joint world).
struct CompositeNodeConstraint final {
    std::uint32_t child_node{};
    std::uint32_t host_node{};
    Matrix4 offset{};
};


// C++23 port of the IPlayer attachment contract. Reverse authority:
// dmc-rengine-cpp include/dmc_rengine/profiles/dmc3/player_attachment_contract.hpp
// with docs/research/dmc3-player-coat-attachment-2026-09-23.md and
// dmc3-player-weapon-attachment-2026-09-23.md.
//  * coat: PAC slot 12, texture slot 0, root = identity x bodyJoint[3]
//    (CPlVergil 0x140225A16, CPlDante 0x1402120A7, CPlNewVergil 0x1402204E9);
//  * weapons: root = local(T, R) x player.joint(j) (0x1401FD8F0, 0x140231505).
inline constexpr std::uint32_t kPlayerCoatHostJoint = 3U;
// Coat host joint for a coat MOD. The retail EXE always uses joint 3; with
// Native Reader tools/mod_fix/coatjoint_patch.py, CPlDante reads the coat
// manager's copy of header +0x13 (player +0x76BA) and uses joint 3 + that
// byte. Retail coats carry 0 (pl000, pl001), so they stay on joint 3. Out of
// range values fall back to joint 3.
[[nodiscard]] std::uint32_t player_coat_host_joint(std::span<const std::uint8_t> coat_mod,
                                                   std::size_t body_joint_count) noexcept;
inline constexpr std::uint32_t kPlayerBodySlot = 1U;
inline constexpr std::uint32_t kPlayerCoatSlot = 12U;
inline constexpr std::uint32_t kPlayerTextureSlot = 0U;
// IPlayer coat chain parameters (";pl000_02.clt" in the sample pl000.pac).
inline constexpr std::uint32_t kPlayerCoatClothSlot = 13U;

// Chain (.clt) slot driving a model slot of an enemy archive: CEm028 init
// 0x140130480 caches slots 7/8 for hair/dress; CEm000..CEm003 inits pair each
// cloth model with the slot before it (0x140097B40 .. 0x1400A6BD0).
struct EnemyClothSource final {
    std::string_view pac_stem;
    std::uint32_t model_slot;
    std::uint32_t clt_slot;
};

inline constexpr std::array<EnemyClothSource, 10> kEnemyClothSources{{
    {"em028", 4U, 7U},
    {"em028", 5U, 8U},
    {"em000", 3U, 2U},
    {"em000", 7U, 6U},
    {"em000", 10U, 9U},
    {"em000", 12U, 11U},
    {"em000", 15U, 14U},
    {"em000", 17U, 16U},
    // em034_018.clt identifies itself as pl002_01.clt and addresses bones
    // 2/4/6/8, exactly matching the four chains of both 9-node companion MODs.
    {"em034", 17U, 18U},
    {"em034", 34U, 18U},
}};

// Texture scroll (.tsc) slot and the model slots its CDrawUV objects drive:
// CEm028 init caches slot 13 (0x14013065F) and hands it to the CDrawUV at
// this+0x2D00 for the dress (slot 5, 0x1401307FD) and at this+0x2D38 for the
// sleeves (slot 6, 0x14013089E).
struct TscSource final {
    std::string_view pac_stem;
    std::uint32_t tsc_slot;
    std::array<std::uint32_t, 2> model_slots;
    std::uint32_t model_count;
};

inline constexpr std::array<TscSource, 2> kTscSources{{
    {"em028", 13U, {5U, 6U}, 2U},
    {"em000", 24U, {23U, 0U}, 1U},  // CEm005Shl01 (0x1400AD757)
}};

// The .tsc slot driving `model_slot` of archive `archive_name`, if any.
[[nodiscard]] std::optional<std::uint32_t> tsc_slot_for(std::string_view archive_name,
                                                        std::uint32_t model_slot) noexcept;

// Match "<stem>.pac" (any directory, any case) and a model slot to its .clt slot.
[[nodiscard]] std::optional<std::uint32_t> enemy_cloth_slot(std::string_view archive_name,
                                                            std::uint32_t model_slot) noexcept;

struct WeaponAttachRecord final {
    std::string_view class_name;
    std::string_view pac_stem;
    std::uint32_t joint;
    std::array<float, 3> translation;
    std::array<float, 3> rotation_xyz_radians;
};

inline constexpr std::array<WeaponAttachRecord, 8> kWeaponState0Records{{
    {"CPlWpSword", "plwp_sword", 3U, {-14.5F, 32.0F, -14.0F},
     {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWp2Sword", "plwp_2sword", 3U, {16.0F, -43.0F, -15.0F},
     {-1.6057028770446777F, 0.0F, 0.2617993950843811F}},
    {"CPlWpGuitar", "plwp_guitar", 3U, {-30.0F, -80.0F, -23.0F},
     {-1.5009831190109253F, -0.11344639956951141F, -0.5235987901687622F}},
    {"CPlWpLaser", "plwp_laser", 8U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    {"CPlWpFoeceEdge", "plwp_forceedge", 3U, {-14.5F, 32.0F, -14.0F},
     {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWpNeroSword", "plwp_nerosword", 3U, {-14.5F, 32.0F, -14.0F},
     {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
    {"CPlWpVergilSword", "plwp_vergilsword", 13U, {19.0F, -0.5F, 11.0F},
     {0.0F, 3.839724063873291F, 0.0F}},
    {"CPlWpNewVergilSword", "plwp_newvergilsword", 13U, {19.0F, -0.5F, 11.0F},
     {0.0F, 3.839724063873291F, 0.0F}},
}};

// Enemy node constraints. Reverse authority: dmc-rengine-cpp
// docs/research/dmc3-em028-nevan-assembly-2026-09-23.md and
// profiles/dmc3/enemy_node_constraint_contract.hpp. CEm028 (Nevan) loads
// body slot 1 and parts 4, 5, 6 with texture slot 0 (0x140130480); its init
// links the listed part nodes to body joints (mode 1, identity offset), the
// remaining part nodes are cloth/bat chains (0x1402C9DC0) not simulated here.
struct EnemyPartConstraints final {
    std::string_view pac_stem;
    std::uint32_t body_slot;
    std::uint32_t part_slot;
    std::span<const CompositeNodeConstraint> constraints;
};

inline constexpr std::array<CompositeNodeConstraint, 3> kEm028Slot4{{{0U, 3U}, {1U, 4U}, {2U, 5U}}};
inline constexpr std::array<CompositeNodeConstraint, 4> kEm028Slot5{
    {{0U, 1U}, {1U, 14U}, {2U, 2U}, {3U, 3U}}};
inline constexpr std::array<CompositeNodeConstraint, 5> kEm028Slot6{
    {{0U, 14U}, {1U, 7U}, {6U, 11U}, {2U, 8U}, {7U, 12U}}};

inline constexpr std::array<EnemyPartConstraints, 3> kEnemyPartConstraints{{
    {"em028", 1U, 4U, kEm028Slot4},
    {"em028", 1U, 5U, kEm028Slot5},
    {"em028", 1U, 6U, kEm028Slot6},
}};

// Match "em028.pac" (any directory, any case) and a top-level part slot.
[[nodiscard]] std::optional<EnemyPartConstraints> enemy_constraints_for(
    std::string_view archive_name, std::uint32_t part_slot) noexcept;

// Hang `child_part` from `host_part` through per-node constraints.
// Coat node constraints: optional player PAC slot 15 ('CCNS', version 1),
// which the retail game never reads. With Native Reader
// tools/mod_fix/coat_patch.py, CPlDante's coat load (hook at 0x140215373)
// gives each listed coat node a mode-1 constraint (0x1402CBBE0: world =
// offset x body joint world), as CEm028 init 0x140130480 does for Nevan.
// Layout: +0 'CCNS', +4 version 1, +8 count, +0x10 count x 0x50 records
// {u32 coat node, u32 body joint, u64 0, f32[16] offset}. The patch takes at
// most 16 records, coat nodes < 39 (0x1401DE820 allocates 39 coat joints)
// and body joints < 96; this parser applies the same limits.
inline constexpr std::uint32_t kPlayerCoatConstraintSlot = 15U;
inline constexpr std::uint32_t kPlayerCoatJointCapacity = 39U;
[[nodiscard]] std::vector<CompositeNodeConstraint> parse_coat_constraints(
    std::span<const std::uint8_t> bytes);

// The six coat collision capsules for a player PAC: kPlayerCoatCapsules with
// the shape replacements of slot 15 applied. After the node records, slot 15
// holds `+0x0C` count (<= 6) records of 0x40 bytes: u32 shape index,
// 12 reserved bytes, f32[4] A, f32[4] B, f32[4] radius. The patch writes
// them over player +0xB630 + index*0x50 + 0x10 before the capsule setter.
[[nodiscard]] std::array<ClothCapsule, 6> player_coat_capsules(std::span<const std::uint8_t> slot15);

// Replaces a part's node constraints and re-poses it.


// Two-part weapons. CPlWp2Sword (Agni & Rudra) is one MOD whose node 2
// carries Agni and node 1 Rudra; 0x1401FDA80 builds part 0 from record
// +0x03/+0x10/+0x20 and part 1 from +0x31/+0x40/+0x50, and pose 0x140227CF0
// sets node2 = local(part0) x joint(+0x114), node1 = local(part1) x
// joint(+0x115), node0 = player world. State-0 record 0x14058C1A0.
struct WeaponSecondPart final {
    std::string_view class_name;
    std::uint32_t first_node;   // node driven by the record's first part
    std::uint32_t second_node;  // node driven by the second part
    std::uint32_t joint;
    std::array<float, 3> translation;
    std::array<float, 3> rotation_xyz_radians;
};

inline constexpr std::array<WeaponSecondPart, 1> kWeaponSecondParts{{
    {"CPlWp2Sword", 2U, 1U, 3U, {-13.0F, 32.0F, -14.0F},
     {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
}};

[[nodiscard]] std::optional<WeaponSecondPart> weapon_second_part(
    std::string_view class_name) noexcept;

// local(T, R) built like the MOD rest local (0x140330450 + 0x140031200).
[[nodiscard]] Matrix4 attach_local_matrix(const std::array<float, 3>& translation,
                                          const std::array<float, 3>& rotation_xyz_radians) noexcept;

// Enemy families sharing one archive. em000.pac feeds five classes
// (CEm000-CEm004); each init (0x140097B40, 0x14009CF70, 0x1400A1D80,
// 0x1400A6BD0, 0x1400A85E0) reads its own body slot, cloth slots (with their
// .clt text slot) and weapon slot; the shared update (0x1401C6FB0 family)
// roots cloth models at body joints [this+0x3254]/[this+0x3258] and the
// weapon at offset(this+0x28E0) x body joint 9. The weapon slot shown is the
// one loaded for variant 0-1 ([this+0x670]); 2-3 load the other weapon slot.
// Offsets are built by 0x1403304A0: Rz x Ry x Rx, then translation.
struct EnemyClothPart final {
    std::uint32_t slot;
    std::uint32_t host_joint;
};

inline constexpr std::uint32_t kNoEnemySlot = 0xFFFFFFFFU;

struct EnemyVariant final {
    std::string_view pac_stem;
    std::string_view class_name;
    std::uint32_t body_slot;
    std::array<EnemyClothPart, 2> cloth;
    std::uint32_t cloth_count;
    std::uint32_t weapon_slot;      // [this+0x670] in 0..1
    std::uint32_t weapon_slot_alt;  // [this+0x670] in 2..3 (same slot: no choice)
    bool cloth_only_first_variant;  // CEm000: cloth drawn for 0..1 only (0x140097980)
    std::uint32_t weapon_joint;
    std::array<float, 3> weapon_translation;
    std::array<float, 3> weapon_rotation_zyx;
    // PTX slot of the body when it is not the nearest preceding one.
    std::uint32_t texture_slot{kNoEnemySlot};
    // Motion PACs the class init reads (slot 35 body motions, 36 / 37 extra;
    // 0 = unused). Empty: every MOT of the archive is offered.
    std::array<std::uint32_t, 3> motion_slots{};
};

inline constexpr std::array<float, 3> kEm000WeaponT{-15.0F, -61.39939880371094F,
                                                   -18.93269920349121F};
inline constexpr std::array<float, 3> kEm000WeaponR{0.20725786685943604F, 0.0F, 0.0F};

// CEm005Shl01 (init 0x1400AD620, vtable 0x1404CB3D8): the EFM in slot 23 is
// loaded as a model with PTX slot 32, motions from slot 37, .clt slot 22
// (em005_02) and .tsc slot 24.
//
// Class inits (vtable slot 53) and the PAC slots they read, in order:
//   CEm000 0x140097B40  41 fx | body 1 tex 0 mot 35 | cloth 3 tex 2 | 29 26 25 | 39 40 col | 38 script
//   CEm001 0x14009CF70  body 5  | cloth 7 (6)            | 31 28 25
//   CEm002 0x1400A1D80  body 8  | cloth 10 (9), 12 (11)  | 29 26 25
//   CEm003 0x1400A6BD0  body 13 | cloth 15 (14), 17 (16) | 30 27 25
//   CEm004 0x1400A85E0  body 18 mot 35 | part 34 tex 32 mot 36
//   CEm005 0x1400AABD0  body 19 tex 0 mot 35 + 37 | cloth 3 (2) | 33 32
//   CEm005Shl00 0x1400AC6B0  model 33 tex 32 mot 37 | 39 40 col | 38 script
inline constexpr std::array<EnemyVariant, 8> kEm000Variants{{
    {"em000", "CEm000", 1U, {{{3U, 14U}, {0U, 0U}}}, 1U, 26U, 29U, true, 9U, kEm000WeaponT,
     kEm000WeaponR, kNoEnemySlot, {35U, 0U, 0U}},
    {"em000", "CEm001", 5U, {{{7U, 14U}, {0U, 0U}}}, 1U, 28U, 31U, false, 9U, kEm000WeaponT,
     kEm000WeaponR, kNoEnemySlot, {35U, 0U, 0U}},
    {"em000", "CEm002", 8U, {{{10U, 8U}, {12U, 12U}}}, 2U, 26U, 29U, false, 9U, kEm000WeaponT,
     kEm000WeaponR, kNoEnemySlot, {35U, 0U, 0U}},
    {"em000", "CEm003", 13U, {{{15U, 14U}, {17U, 14U}}}, 2U, 27U, 30U, false, 9U, kEm000WeaponT,
     kEm000WeaponR, kNoEnemySlot, {35U, 0U, 0U}},
    {"em000", "CEm004", 18U, {{{0U, 0U}, {0U, 0U}}}, 0U, 34U, 34U, false, 9U, {2.0F, 20.0F, -72.0F},
     {-0.03490658476948738F, 0.10471975803375244F, 1.6580626964569092F}, kNoEnemySlot, {35U, 36U, 0U}},
    {"em000", "CEm005", 19U, {{{3U, 14U}, {0U, 0U}}}, 1U, kNoEnemySlot, kNoEnemySlot, false, 9U,
     {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {35U, 37U, 0U}},
    {"em000", "CEm005Shl00", 33U, {{{0U, 0U}, {0U, 0U}}}, 0U, kNoEnemySlot, kNoEnemySlot, false, 0U,
     {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 32U, {37U, 0U, 0U}},
    {"em000", "CEm005Shl01", 23U, {{{0U, 0U}, {0U, 0U}}}, 0U, kNoEnemySlot, kNoEnemySlot, false,
     0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 32U, {37U, 0U, 0U}},
}};

// Variants for an archive name ("em000.pac", any directory, any case).
[[nodiscard]] std::span<const EnemyVariant> enemy_variants_for(
    std::string_view archive_name) noexcept;

// One selectable position of an archive with several in-game looks: an
// enemy class and its weapon variant (em000.pac), or a model-object state
// (em028.pac: the dress strip, objects 2-3 of slot 5, is drawn only while
// bats are out -- 0x14012F790 sets or clears object bit 0, which the MOD draw
// loops 0x140303460 / 0x140303DE0 require).
struct ArchiveTextureOverride final {
    std::uint32_t model_slot{};
    std::uint32_t texture_slot{};
};

struct ArchivePartAttachment final {
    std::uint32_t host_model_slot{};
    std::uint32_t child_model_slot{};
    std::uint32_t host_joint{};
    bool root_local_identity{};
    // true when the host relation is supported by retained structural/corpus
    // evidence. exe_confirmed is stronger and is used when the exact runtime
    // consumer and transform record have been recovered from dmc3.exe.
    bool structural_confirmed{};
    bool exe_confirmed{};
    std::array<float, 3> translation{};
    std::array<float, 3> rotation_xyz_radians{};
    bool explicit_offset{};
};

struct ArchiveVariant final {
    std::string label;
    const EnemyVariant* enemy{};  // em000 family
    bool alternate_weapon{};      // [this+0x670] in 2..3
    std::uint32_t hide_slot{};    // model slot whose objects are hidden
    std::array<std::uint32_t, 2> hide_objects{};
    std::uint32_t hide_count{};

    // Some actor PACs carry several complete appearances and equipment models
    // in one top-level archive. When non-zero, only these top-level MOD slots
    // belong to this selectable appearance; every other MOD remains available
    // as a PAC child instead of being incorrectly overlaid at the actor origin.
    std::array<std::uint32_t, 12> include_top_level_mod_slots{};
    std::uint32_t include_top_level_mod_count{};

    // Explicit companion PTX association for appearance parts whose correct
    // texture bank is not the nearest preceding PTX in physical slot order.
    std::array<ArchiveTextureOverride, 12> texture_overrides{};
    std::uint32_t texture_override_count{};

    // Corpus/structure-backed companion placement. These records are distinct
    // from EnemyVariant because they may be known from a PAC/CLT relationship
    // before the exact class-init EXE consumer has been recovered.
    std::array<ArchivePartAttachment, 12> part_attachments{};
    std::uint32_t part_attachment_count{};
};

[[nodiscard]] std::vector<ArchiveVariant> archive_variants(std::string_view archive_name);

// Canonical boss-Lady (CEm034) equipment contract. This is intentionally
// separate from CPlWpLadyGun / player weapon state tables: CEm034 owns five
// persistent MOD managers and several dynamic CShell actors.
enum class LadyPlacementPreset : std::uint8_t {
    BodyStowed = 0,
    ActiveDeployed = 1,
};

enum class LadyControlDomain : std::uint8_t {
    BodyConstraint = 0,
    IndependentMotionScript = 1,  // slot20 / em034_013 only
};

enum class LadyEffectiveParent : std::uint8_t {
    BodyJoint = 0,
    RuntimeJointScaled = 1,  // CEm034+0x43C0 = joint13 world * S(+0x4400)
};

struct LadyPlacementRecord final {
    std::uint32_t serialized_node{};
    std::array<float, 3> translation{};
    std::array<float, 3> rotation_xyz_radians{};
    LadyEffectiveParent effective_parent{LadyEffectiveParent::BodyJoint};
    std::uint32_t runtime_parent_offset{};
    std::uint32_t runtime_scale_source_offset{};
};

struct LadyComponentContract final {
    std::uint8_t component{};
    std::uint32_t model_slot{};
    std::array<LadyPlacementRecord, 2> presets{};
};

inline constexpr std::array<LadyComponentContract, 5> kCEm034LadyComponents{{
    {0U, 20U, {{
        {3U, {-2.0F, -20.0F, -17.0F},
         {-1.570796251296997F, 0.0F, 1.0821040868759155F}},
        {9U, {-8.399999618530273F, -1.0F, -1.2999999523162842F},
         {0.0F, 0.0F, 3.141592502593994F}},
    }}},
    {1U, 21U, {{
        {14U, {-1.0F, -4.0F, 13.0F},
         {1.867502212524414F, 0.048869214951992035F, 2.4085543155670166F}},
        {9U, {-7.5F, -0.6000000238418579F, -0.800000011920929F},
         {0.0F, 0.0F, 0.0F}},
    }}},
    {2U, 22U, {{
        {16U, {-9.199999809265137F, -13.0F, -9.100000381469727F},
         {0.0F, 0.0F, 1.6580626964569092F}},
        {13U, {7.699999809265137F, -0.800000011920929F, 0.5F},
         {0.0F, 0.0F, 3.141592502593994F}},
    }}},
    {3U, 23U, {{
        {19U, {10.0F, -15.0F, -2.5F},
         {0.0F, 0.0F, -1.6580626964569092F}},
        {13U, {7.199999809265137F, -1.2000000476837158F, 2.700000047683716F},
         {0.0F, -0.1745329201221466F, 0.0F},
         LadyEffectiveParent::RuntimeJointScaled, 0x43C0U, 0x4400U},
    }}},
    {4U, 24U, {{
        {14U, {17.0F, -5.0F, -16.0F},
         {-1.2217304706573486F, -0.2356194406747818F, 0.6283184885978699F}},
        {13U, {7.199999809265137F, -0.800000011920929F, -0.4000000059604645F},
         {0.0F, 0.0F, 3.141592502593994F}},
    }}},
}};

struct LadyComponentBinding final {
    std::size_t host_part{};
    std::size_t part{};
    std::uint8_t component{};
    std::uint32_t model_slot{};
    LadyPlacementPreset preset{LadyPlacementPreset::BodyStowed};
    LadyControlDomain control_domain{LadyControlDomain::BodyConstraint};
    // CEm034+0x4400 for component3 ActiveDeployed. EXE writes 1.0 every
    // action46 update and promotes it to 1.5 while lane1/channel1 == 1.
    float runtime_uniform_scale{1.0F};
};

struct LadyDynamicActorContract final {
    std::string_view class_name;
    std::array<std::uint32_t, 2> model_slots{};
    std::uint8_t model_slot_count{};
};

inline constexpr std::array<LadyDynamicActorContract, 6> kCEm034LadyDynamicActors{{
    {"CEm034Shl00", {{0U, 0U}}, 0U},
    {"CEm034Shl01", {{0U, 0U}}, 0U},
    {"CEm034Shl02", {{25U, 0U}}, 1U},
    {"CEm034Shl03", {{26U, 30U}}, 2U},
    {"CEm034Shl04", {{0U, 0U}}, 0U},
    {"CEm034Shl05", {{0U, 0U}}, 0U},
}};

[[nodiscard]] constexpr const LadyComponentContract* lady_component_contract(
    std::uint8_t component) noexcept {
    return component < kCEm034LadyComponents.size()
        ? &kCEm034LadyComponents[component]
        : nullptr;
}

[[nodiscard]] constexpr const LadyComponentContract* lady_component_contract_for_slot(
    std::uint32_t model_slot) noexcept {
    for (const auto& contract : kCEm034LadyComponents) {
        if (contract.model_slot == model_slot) return &contract;
    }
    return nullptr;
}

// Apply an EXE-confirmed CEm034 placement preset to an assembled persistent
// component. RuntimeJointScaled is represented exactly in the contract and
// is not approximated as serialized node 13; if the live body-root/scale bridge
// is unavailable this function returns false instead of guessing.

// Slot20 may switch between body CCnsMatrix control and the independent
// em034_013 MotionScript domain. This records the canonical domain transition;
// the independent MOT controller is a separate playback layer.

// Exact CEm034 component3 active-parent scalar (+0x4400). Re-materializes
// RuntimeJointScaled when component3 is currently ActiveDeployed.

struct LadyRuntimeApplyResult final {
    bool recognized{};
    bool fully_materialized{true};
    bool runtime_side_effect{};
    std::uint32_t changed_components{};
    // -1 = no dynamic actor event; otherwise CEm034Shl00..05 index.
    std::int8_t dynamic_actor{-1};
};

// Apply the recovered CEm034 state-entry baseline/overrides. The binding state
// is updated even if an exact preview cannot be materialized (currently only
// component3 ActiveDeployed's RuntimeJointScaled parent).

struct LadyBodyScriptState final {
    std::uint16_t state{};
    // Which em034_012 controller starts this exact action at state entry:
    // bit0 = lane0 (+0x5070), bit1 = lane1 (+0x5190).
    std::uint8_t lane_mask{};
};

// Inverse of the canonical CEm034 entry dispatcher at 0x14016A410.
// This returns the primary state for a body-script action. Some states 55..82
// start a second, different action on the other lane; the returned lane mask
// says which lane(s) execute the requested action.
[[nodiscard]] constexpr std::optional<LadyBodyScriptState>
lady_state_for_body_script_action(std::size_t bank, std::size_t action) noexcept {
    if (bank == 0U) {
        if (action <= 6U || (action >= 8U && action <= 14U)) {
            return LadyBodyScriptState{static_cast<std::uint16_t>(action), 0x3U};
        }
        return std::nullopt;
    }
    if (bank == 1U) {
        if (action <= 5U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(15U + action), 0x3U};
        if (action == 7U) return LadyBodyScriptState{22U, 0x3U};
        if (action >= 9U && action <= 11U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(15U + action), 0x3U};
        if (action == 20U) return LadyBodyScriptState{35U, 0x3U};
        if (action >= 23U && action <= 26U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(15U + action), 0x3U};
        return std::nullopt;
    }
    if (bank == 2U && action <= 3U) {
        return LadyBodyScriptState{
            static_cast<std::uint16_t>(42U + action), 0x3U};
    }
    if (bank == 3U) {
        if (action == 0U) return LadyBodyScriptState{46U, 0x3U};
        if (action >= 2U && action <= 8U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(46U + action), 0x3U};
        if (action >= 9U && action <= 15U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(46U + action), 0x2U};
        if (action >= 16U && action <= 22U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(46U + action), static_cast<std::uint8_t>(action == 16U ? 0x3U : 0x2U)};
        if (action >= 23U && action <= 29U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(46U + action), static_cast<std::uint8_t>(action == 23U ? 0x3U : 0x2U)};
        if (action >= 30U && action <= 36U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(46U + action), static_cast<std::uint8_t>(action == 30U ? 0x3U : 0x2U)};
        return std::nullopt;
    }
    if (bank == 4U) {
        if (action <= 32U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(83U + action), 0x3U};
        if (action >= 40U && action <= 47U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(83U + action), 0x3U};
        if (action == 50U || action == 60U) return LadyBodyScriptState{
            static_cast<std::uint16_t>(83U + action), 0x3U};
    }
    return std::nullopt;
}

struct LadyBodyLaneAction final {
    bool valid{};
    std::uint8_t bank{};
    std::uint8_t action{};
};

struct LadyBodyStateScripts final {
    std::array<LadyBodyLaneAction, 2> lanes{};
};

// Forward form of the same 0x14016A410 entry dispatcher. This is required
// because states55..82 intentionally start different actions on lane0/lane1.
[[nodiscard]] constexpr LadyBodyStateScripts lady_body_state_scripts(
    std::uint16_t state) noexcept {
    LadyBodyStateScripts out{};
    const auto both = [&out](std::uint8_t bank, std::uint8_t action) constexpr {
        out.lanes[0] = {true, bank, action};
        out.lanes[1] = {true, bank, action};
    };
    if (state <= 6U || (state >= 8U && state <= 14U)) {
        both(0U, static_cast<std::uint8_t>(state));
    } else if (state >= 15U && state <= 20U) {
        both(1U, static_cast<std::uint8_t>(state - 15U));
    } else if (state == 22U) {
        both(1U, 7U);
    } else if (state >= 24U && state <= 26U) {
        both(1U, static_cast<std::uint8_t>(state - 15U));
    } else if (state == 35U) {
        both(1U, 20U);
    } else if (state >= 38U && state <= 41U) {
        both(1U, static_cast<std::uint8_t>(state - 15U));
    } else if (state >= 42U && state <= 45U) {
        both(2U, static_cast<std::uint8_t>(state - 42U));
    } else if (state == 46U) {
        both(3U, 0U);
    } else if (state >= 48U && state <= 54U) {
        both(3U, static_cast<std::uint8_t>(state - 46U));
    } else if (state >= 55U && state <= 61U) {
        out.lanes[0] = {true, 0U, 1U};
        out.lanes[1] = {true, 3U, static_cast<std::uint8_t>(state - 46U)};
    } else if (state >= 62U && state <= 68U) {
        out.lanes[0] = {true, 3U, 16U};
        out.lanes[1] = {true, 3U, static_cast<std::uint8_t>(state - 46U)};
    } else if (state >= 69U && state <= 75U) {
        out.lanes[0] = {true, 3U, 23U};
        out.lanes[1] = {true, 3U, static_cast<std::uint8_t>(state - 46U)};
    } else if (state >= 76U && state <= 82U) {
        out.lanes[0] = {true, 3U, 30U};
        out.lanes[1] = {true, 3U, static_cast<std::uint8_t>(state - 46U)};
    } else if (state >= 83U && state <= 115U) {
        both(4U, static_cast<std::uint8_t>(state - 83U));
    } else if (state >= 123U && state <= 130U) {
        both(4U, static_cast<std::uint8_t>(state - 83U));
    } else if (state == 133U) {
        both(4U, 50U);
    } else if (state == 143U) {
        both(4U, 60U);
    }
    return out;
}

// em034_013 action that the CEm034 entry dispatcher 0x14016A410 starts on the
// component0 controller (+0x52B0) for `state`. Only the entries that reach
// 0x14016AA73 do so: states 0x5A..0x5D and 0x64..0x73 (case 0x14016AA3D),
// 0x7B (0x14016A509) and 0x7C/0x7D (0x14016A52F). That block starts bank 4,
// action state - 0x53 and clears +0x4020, the enable byte of the slot20 node0
// CCnsMatrix at +0x4000: component0 then follows its own MOT in actor space.
// Every other entry keeps (or re-enables) the constraint through 0x1401713F0,
// which always writes +0x4020 = 1, so no component track runs there.
// States 0x59/0x61 start the same controller later, from their lane0 signal
// (0x14016988B); that delayed start is not a state-entry pairing.
[[nodiscard]] constexpr std::optional<LadyBodyLaneAction>
lady_component_action_for_state(std::uint16_t state) noexcept {
    const bool independent = (state >= 0x5AU && state <= 0x5DU) ||
                             (state >= 0x64U && state <= 0x73U) ||
                             (state >= 0x7BU && state <= 0x7DU);
    if (!independent) return std::nullopt;
    return LadyBodyLaneAction{
        true, 4U, static_cast<std::uint8_t>(state - 0x53U)};
}

// Consume one signal value exactly as CEm034's direct 0x140059350 consumers do.
// lane is 0/1 for the two em034_012 controllers; channel is 0..4.

// Canonical MOD/CEm034 local matrix: translation + XYZ Euler using the
// recovered DMC3 transform-domain composition.
[[nodiscard]] Matrix4 attach_local_matrix(const std::array<float, 3>& translation,
                                         const std::array<float, 3>& rotation_xyz_radians) noexcept;

// Translation plus Rz x Ry x Rx (0x1403304A0 order).
[[nodiscard]] Matrix4 attach_local_matrix_zyx(const std::array<float, 3>& translation,
                                              const std::array<float, 3>& rotation_xyz_radians) noexcept;

// Weapon motion banks. The weapon factory 0x1401DED20 creates the melee
// classes from ids 0-4, 11, 12, 14 and the gun classes from ids 5-10, 13;
// Dante's loader 0x1401DF6BE loads motion\\pl000\\pl000_00_N.pac with
// N = byte 0x14058ABC8[id * 4] through 0x1401B90B0 (path table 0x1405B0F30).
struct WeaponMotionBank final {
    std::uint8_t weapon_id;
    std::string_view class_name;
    std::string_view weapon_name;  // common name for the class
    std::uint8_t file_index;       // pl000_00_<file_index>.pac
};

inline constexpr std::array<WeaponMotionBank, 15> kDanteWeaponMotionBanks{{
    {0U, "CPlWpSword", "Rebellion", 3U},
    {1U, "CPlWpNunchaku", "Cerberus", 4U},
    {2U, "CPlWp2Sword", "Agni & Rudra", 5U},
    {3U, "CPlWpGuitar", "Nevan", 6U},
    {4U, "CPlWpFight", "Beowulf", 7U},
    {5U, "CPlWpGun", "Ebony & Ivory", 8U},
    {6U, "CPlWpShotGun", "Shotgun", 9U},
    {7U, "CPlWpLaser", "Artemis", 10U},
    {8U, "CPlWpRifle", "Spiral", 11U},
    {9U, "CPlWpLadyGun", "Kalina Ann", 12U},
    {10U, "CPlWpLadyGun", "Kalina Ann (id 10)", 27U},
    {11U, "CPlWpNewVergilSword", "Yamato (CPlWpNewVergilSword)", 28U},
    {12U, "CPlWpFight", "Beowulf (id 12)", 29U},
    {13U, "CPlWpFoeceEdge", "Force Edge", 30U},
    {14U, "CPlWpVergilSword", "Yamato (CPlWpVergilSword)", 31U},
}};

// Weapon attach state records: weapon+0x128 points at 24 record pointers
// (0x60-byte records, read by 0x1401FD8F0 / 0x1401FDC90 / 0x1401FDA80). The
// sword update 0x140231680 picks state [player+0x39C3] (motion script) while
// the weapon is active, else 1. Record byte +0 is the pose branch (+0x11A,
// 255 = empty), +3/+0x10/+0x20 the first part (joint, T, XYZ Euler) and
// +0x31/+0x40/+0x50 the second part (two-blade classes).
struct WeaponStateRecord final {
    std::uint8_t branch;
    std::uint8_t joint;
    std::array<float, 3> translation;
    std::array<float, 3> rotation_xyz_radians;
    std::uint8_t second_joint;
    std::array<float, 3> second_translation;
    std::array<float, 3> second_rotation_xyz_radians;
};

struct WeaponStateTable final {
    std::string_view class_name;
    std::uint64_t table_va;
    std::array<WeaponStateRecord, 24> states;
};

// Generated from dmc3.exe record tables (0x60-byte records, 24 states).
inline constexpr std::array<WeaponStateTable, 8> kWeaponStateTables{{
    {"CPlWpSword", 0x14058C010ULL, {{
        {0U, 3U, {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}, 0U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 3U, {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}, 0U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}, 0U, {7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}, 0U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}},
        {1U, 9U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, 0.0F}, 191U, {3.386688232421875F, 1.401298464324817e-45F, 3.386699676513672F}, {3.3867111206054688F, 1.401298464324817e-45F, 3.3866424560546875F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWp2Sword", 0x14058C800ULL, {{
        {0U, 3U, {16.0F, -43.0F, -15.0F}, {-1.6057028770446777F, 0.0F, 0.2617993950843811F}, 3U, {-13.0F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
        {0U, 3U, {16.0F, -43.0F, -15.0F}, {-1.6057028770446777F, 0.0F, 0.2617993950843811F}, 3U, {-13.0F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
        {0U, 3U, {16.0F, -43.0F, -15.0F}, {-1.6057028770446777F, 0.0F, 0.2617993950843811F}, 3U, {-13.0F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 3.141592502593994F}, 9U, {0.0F, 0.0F, -32.63330078125F}, {-3.141592502593994F, 0.0F, -3.141592502593994F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 3.141592502593994F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 3.141592502593994F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 3U, {16.0F, -43.0F, -15.0F}, {-1.6057028770446777F, 0.0F, 0.2617993950843811F}, 3U, {-13.0F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {0.0F, 0.0F, -32.63330078125F}, {-3.141592502593994F, 0.0F, -3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 3.141592502593994F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 3.141592502593994F, 3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 3.141592502593994F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 3.141592502593994F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 13U, {7.599999904632568F, -3.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, 9U, {-7.599999904632568F, -3.0F, 1.0F}, {3.141592502593994F, 0.0F, 0.0F}},
    }}},
    {"CPlWpGuitar", 0x14058DAE0ULL, {{
        {0U, 3U, {-30.0F, -80.0F, -23.0F}, {-1.5009831190109253F, -0.11344639956951141F, -0.5235987901687622F}, 0U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, -3.141592502593994F}},
        {0U, 3U, {-30.0F, -80.0F, -23.0F}, {-1.5009831190109253F, -0.11344639956951141F, -0.5235987901687622F}, 0U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, -3.141592502593994F}},
        {0U, 9U, {-7.599999904632568F, -3.0F, -1.0F}, {0.0F, 0.0F, -3.141592502593994F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {3U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {4U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {5U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {6U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {7U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {8U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {9U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {10U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {11U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 10U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {12U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {17U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {14U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {15U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {16U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {17U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {18U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {19U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {20U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {21U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {22U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {47U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWpLaser", 0x14058E380ULL, {{
        {1U, 8U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {1U, 8U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWpFoeceEdge", 0x14058ED30ULL, {{
        {0U, 3U, {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}, 0U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 3U, {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}, 0U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {9.0F, 0.0F, -1.0F}, {0.0F, 0.5235987901687622F, 0.0F}},
        {0U, 13U, {9.0F, 0.0F, -1.0F}, {0.0F, 0.5235987901687622F, 0.0F}, 236U, {3.389453887939453F, 1.401298464324817e-45F, 3.38946533203125F}, {3.3866424560546875F, 1.401298464324817e-45F, 3.3866424560546875F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWpNeroSword", 0x14058FAF0ULL, {{
        {0U, 3U, {-14.5F, 32.0F, -14.0F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F}, 0U, {-5.5F, -2.0F, 0.0F}, {0.0F, -0.2356194406747818F, 0.0F}},
        {0U, 9U, {-5.5F, -2.0F, 0.0F}, {0.0F, -0.2356194406747818F, 0.0F}, 250U, {3.3903045654296875F, 1.401298464324817e-45F, 3.3866424560546875F}, {3.3866424560546875F, 1.401298464324817e-45F, 3.3866424560546875F}},
        {0U, 9U, {-5.5F, -2.0F, 0.0F}, {0.0F, -0.2356194406747818F, 0.0F}, 250U, {3.3903045654296875F, 1.401298464324817e-45F, 3.3866424560546875F}, {3.3866424560546875F, 1.401298464324817e-45F, 3.3866424560546875F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWpVergilSword", 0x14058F280ULL, {{
        {0U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {51U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {6U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {7U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {8U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {22U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
    {"CPlWpNewVergilSword", 0x14058F770ULL, {{
        {0U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 14U, {15.0F, 10.0F, 20.0F}, {-3.054326057434082F, -0.8726646304130554F, -1.570796251296997F}},
        {0U, 14U, {15.0F, 10.0F, 20.0F}, {-3.054326057434082F, -0.8726646304130554F, -1.570796251296997F}, 14U, {15.0F, 10.0F, 20.0F}, {-3.054326057434082F, -0.8726646304130554F, -1.570796251296997F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 13U, {19.0F, -0.5F, 11.0F}, {0.0F, 3.839724063873291F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {0U, 9U, {-7.5F, -2.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 51U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
        {255U, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0U, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
    }}},
}};

// Record of `state` for a weapon class; nullptr for an empty record.
[[nodiscard]] const WeaponStateRecord* weapon_state_record(std::string_view class_name,
                                                           std::uint8_t state) noexcept;

// One weapon part of an assembled session whose attach record follows the
// motion script.
struct WeaponBinding final {
    std::size_t part{};
    std::string_view class_name;
    std::uint8_t state{};
};

// Re-point a bound weapon part at the record of `state` (single part: host
// joint + offset; two-blade classes: both node constraints). The next pose
// (apply_part_attachments) moves it. False when the record is empty or a
// special pose branch (Nevan's play poses, branch >= 2) that is not ported.

// Match "pl000_00_<N>.pac" (any directory, any case) to its weapon bank.
[[nodiscard]] std::optional<WeaponMotionBank> weapon_motion_bank(
    std::string_view archive_name) noexcept;

// Match a PAC file name (any directory, any case, ".pac") to a weapon record.
[[nodiscard]] std::optional<WeaponAttachRecord> weapon_record_for_archive(
    std::string_view archive_name) noexcept;

// local(T, R) built exactly like the MOD rest local (0x140330450 + 0x140031200).
[[nodiscard]] Matrix4 weapon_offset_matrix(const WeaponAttachRecord& record) noexcept;

// Hang `child_part`'s skeleton from `host_joint` of `host_part` and pose it
// immediately from the host joint's current world. Read-only: only the
// derived composite projection changes.

// Re-pose every HostJointSkeleton part from its host joint's current world.
// Called after the host moved (MOT frame) and after attachment.
// Same, advancing every attached cloth by `cloth_steps` solver frames (dt 1).

// Forget the simulated chain state (next pose restarts from the rest pose).

// Simulate the nodes listed by `clt_text` (first cloth block) on an attached
// part and settle it for `settle_steps` frames from its current pose.
// Returns the number of simulated nodes (0 = not a cloth file / no match).


}  // namespace dmc::rengine::profiles::dmc3::motion

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

// Enemy multi-model assembly recovered from dmc3.exe
// (SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082).
// Evidence: docs/research/dmc3-em028-nevan-assembly-2026-09-23.md.
namespace dmc::rengine::profiles::dmc3::enemy_node_constraint {

// Joint constraint (0xC0 bytes, vtable 0x1404CC1F8, ctor 0x1400F7AC0).
// 0x14030E680 asks an enabled constraint for the joint world instead of
// composing local x parent (0x14030E9B0). Apply 0x1402CBBE0 switches on +0x28:
// mode 1 -> world = offset(+0x80) x *host(+0x30) through 0x140030E40,
// mode 2 -> world = offset x own matrix (+0x40), otherwise the normal path.
enum class ConstraintMode : std::uint32_t {
    Normal = 0U,
    HostWorld = 1U,
    OwnMatrix = 2U,
};

struct NodeConstraint final {
    std::uint32_t child_node;   // node of the attached model
    std::uint32_t host_node;    // joint of the body model
    std::uint64_t evidence_va;  // store of the host world pointer (+0x30)
};

struct PartConstraints final {
    std::string_view pac_stem;
    std::uint32_t body_slot;
    std::uint32_t part_slot;
    std::span<const NodeConstraint> constraints;
};

// CEm028 (Nevan, vtable 0x1404D35E0). Factory 0x14012EA80 sizes one joint
// table per MOD (slots 1, 4, 5, 6); init 0x140130480 loads the four models
// with PTX slot 0, hands slot 9 (a PNST effect bank) to 0x1402C04C0 and
// installs these mode-1 constraints, all with the identity offset
// (.rdata 0x14035D580..0x14035D5B0).
inline constexpr std::array<NodeConstraint, 3> em028_slot4{{
    {0U, 3U, 0x140130911ULL},
    {1U, 4U, 0x14013095DULL},
    {2U, 5U, 0x1401309A9ULL},
}};
inline constexpr std::array<NodeConstraint, 4> em028_slot5{{
    {0U, 1U, 0x1401309F5ULL},
    {1U, 14U, 0x140130A41ULL},
    {2U, 2U, 0x140130A8DULL},
    {3U, 3U, 0x140130AD9ULL},
}};
inline constexpr std::array<NodeConstraint, 5> em028_slot6{{
    {0U, 14U, 0x140130B25ULL},
    {1U, 7U, 0x140130B71ULL},
    {6U, 11U, 0x140130BBDULL},
    {2U, 8U, 0x140130C09ULL},
    {7U, 12U, 0x140130C55ULL},
}};

inline constexpr std::array<PartConstraints, 3> part_constraints{{
    {"em028", 1U, 4U, em028_slot4},
    {"em028", 1U, 5U, em028_slot5},
    {"em028", 1U, 6U, em028_slot6},
}};

// Nevan's remaining slot-4 and slot-5 nodes are chains simulated by
// 0x1402C9DC0 (arrays at this+0x3A30 and this+0x4CF0, 20 entries each);
// a viewer without that simulation keeps them at their rest locals.
inline constexpr std::uint32_t em028_texture_slot = 0U;
inline constexpr std::uint32_t em028_effect_bank_slot = 9U;

[[nodiscard]] constexpr std::optional<PartConstraints> constraints_for(
    std::string_view pac_stem, std::uint32_t part_slot) noexcept {
    for (const auto& record : part_constraints) {
        if (record.pac_stem == pac_stem && record.part_slot == part_slot) return record;
    }
    return std::nullopt;
}

}  // namespace dmc::rengine::profiles::dmc3::enemy_node_constraint

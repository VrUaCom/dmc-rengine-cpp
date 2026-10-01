#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

// Read-only port of the DMC3 chain/cloth solver. Reverse authority:
// dmc-rengine-cpp docs/research/dmc3-cloth-chain-solver-2026-09-23.md.
//
// A .clt text (";pl000_02.clt", ";em028_01.clt", ...) lists one or more cloth
// blocks; parser 0x1402CA345 / 0x1402CA42A fills a CCnsChain (defaults
// 0x1402CA000) and gives every listed bone an axis (X Y Z NX NY NZ -> 0-5,
// joint +0x240). Each simulated joint is a node constraint whose apply
// 0x1402C9450 runs once per game frame (dt = [c+0xE0], 1 at 60 fps):
//   S.t += dt*v; point S's axis at the parent (0x14032EEE0 / 0x14032F4C0 /
//   0x14032FD90); W = blend(rest target, S) by Stiffness (0x14032DA20);
//   v += dt*(wind + Gravity); keep the rest bone length (LimitLength) and pull
//   v back by SpringForce; clamp |v| <= MaxSpeed; v *= 0.99; floor clamp.
namespace dmc::rengine::profiles::dmc3::motion {

struct ClothBone final {
    std::uint32_t node{};
    std::uint8_t axis{};  // 0 X, 1 Y, 2 Z, 3 NX, 4 NY, 5 NZ
};

struct ClothParams final {
    std::array<float, 3> gravity{0.0F, -0.2F, 0.0F};  // +0xB0
    float spring_force{0.05F};                          // +0x78
    float max_speed{50.0F};                             // +0x74
    float stiffness{0.3F};                              // +0x6C
    std::array<float, 3> wind{0.0F, 0.0F, 0.0F};        // +0xA0
    bool wind_local{false};                             // +0x80
    std::int32_t wind_parent{0};                        // +0x7C
    std::int32_t wind_type{0};                          // +0x8C
    float floor_level{-1000000.0F};                     // +0x84
    bool limit_length{true};                            // +0xE4
    float damping{0.99F};                               // +0x88
    std::vector<ClothBone> bones;
};

// Byte identity: ';' comment first line and a "ClothNo" block key.
[[nodiscard]] bool looks_like_clt(std::string_view text);

// Every block of a .clt text; empty when the text is not a cloth file.
[[nodiscard]] std::vector<ClothParams> parse_clt(std::string_view text);

// Collision capsule on a host (body) joint: segment a-b in joint space and
// radius. Set by 0x1402CA2F0 (chain +0x30 joints, +0x58 entries {flags 8,
// joint, shape}, +0x60 shapes of 0x50 bytes, +0x50 count).
struct ClothCapsule final {
    std::uint32_t host_joint{};
    std::array<float, 3> a{};
    std::array<float, 3> b{};
    float radius{};
};

// IPlayer coat capsules: entry table 0x14058B380 (6 entries) with shapes
// written from .rdata 0x14058B260 (0x140214E17): chest, spine, both legs.
inline constexpr std::array<ClothCapsule, 6> kPlayerCoatCapsules{{
    {3U, {0.0F, 20.0F, 10.0F}, {0.0F, -40.0F, 10.0F}, 15.0F},
    {2U, {0.0F, -5.0F, 0.0F}, {0.0F, -15.0F, 0.0F}, 18.0F},
    {15U, {0.0F, 0.0F, 0.0F}, {0.0F, -50.0F, 0.0F}, 10.0F},
    {16U, {0.0F, 0.0F, 0.0F}, {0.0F, -50.0F, 0.0F}, 10.0F},
    {19U, {0.0F, 0.0F, 0.0F}, {0.0F, -50.0F, 0.0F}, 10.0F},
    {20U, {0.0F, 0.0F, 0.0F}, {0.0F, -50.0F, 0.0F}, 10.0F},
}};

// CEm028 hair chain (slot 4 <- em028_01.clt): 0x140130D9A sets entry table
// 0x140576110 (3 entries) on the body joints with shapes from .rdata
// 0x140576120: neck/head segments. The dress chain gets no collision.
inline constexpr std::array<ClothCapsule, 3> kEm028HairCapsules{{
    {5U, {0.0F, 4.65F, 0.0F}, {0.0F, -4.65F, 0.0F}, 9.3F},
    {4U, {0.0F, 6.0F, 0.0F}, {0.0F, -6.0F, 0.0F}, 12.0F},
    {3U, {0.0F, 9.5F, 0.0F}, {0.0F, -9.5F, 0.0F}, 19.0F},
}};

// A capsule already placed in world space for this frame.
struct WorldCapsule final {
    std::array<float, 3> a{};
    std::array<float, 3> b{};
    float radius{};
};

// Per-part solver state (simulated world and velocity of each listed node).
struct ClothState final {
    ClothParams params;  // block 0 (ClothNo 0)
    // Every ClothNo block of the .clt (ClothNum of them) and the block that
    // owns each node; a node steps with its own block's parameters
    // (pl001_02.clt: block 0 the front and side panels, block 1 the back
    // panel). Capsules only act on block 0 nodes (0x1402151E7).
    std::vector<ClothParams> blocks;
    std::vector<std::uint8_t> block_by_node;
    [[nodiscard]] const ClothParams& params_for(std::uint32_t node) const noexcept {
        return node < block_by_node.size() && block_by_node[node] < blocks.size() ? blocks[block_by_node[node]]
                                                                                 : params;
    }
    std::vector<std::array<float, 16>> sim;
    std::vector<std::array<float, 3>> velocity;
    std::vector<std::int8_t> axis_by_node;  // -1: not simulated
    std::vector<ClothCapsule> capsules;     // on the host part's joints
    bool initialized{false};
};

// One solver step for `node`: `target` = rest local x parent world, `parent`
// = parent world, `wind_parent_world` = world of the wind parent joint.
// Returns the new node world (row-vector, translation in [12..14]).
[[nodiscard]] std::array<float, 16> step_cloth_node(ClothState& state,
                                                    std::uint32_t node,
                                                    const std::array<float, 16>& target,
                                                    const std::array<float, 16>& parent,
                                                    const std::array<float, 16>& wind_parent_world,
                                                    float rest_length,
                                                    float dt,
                                                    std::span<const WorldCapsule> capsules = {});

}  // namespace dmc::rengine::profiles::dmc3::motion

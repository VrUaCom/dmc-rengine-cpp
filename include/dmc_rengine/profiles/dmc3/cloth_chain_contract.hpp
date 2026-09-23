#pragma once

#include <array>
#include <cstdint>
#include <string_view>

// DMC3 chain/cloth constraint (.clt text, classes CChain / CChainNode /
// CCnsChain). Evidence: docs/research/dmc3-cloth-chain-solver-2026-09-23.md
// (dmc3.exe SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082).
namespace dmc::rengine::profiles::dmc3::cloth_chain {

inline constexpr std::uint64_t defaults_va = 0x1402CA000ULL;
inline constexpr std::uint64_t parser_block_va = 0x1402CA345ULL;
inline constexpr std::uint64_t parser_bone_va = 0x1402CA42AULL;
inline constexpr std::uint64_t solve_node_va = 0x1402C9450ULL;
inline constexpr std::uint64_t align_x_va = 0x14032EEE0ULL;
inline constexpr std::uint64_t align_y_va = 0x14032F4C0ULL;
inline constexpr std::uint64_t align_z_va = 0x14032FD90ULL;
inline constexpr std::uint64_t blend_va = 0x14032DA20ULL;

struct Field final {
    std::string_view keyword;
    std::uint16_t offset;  // in the chain constraint
};

inline constexpr std::array<Field, 11> fields{{
    {"Stiffness", 0x6CU},
    {"MaxSpeed", 0x74U},
    {"SpringForce", 0x78U},
    {"WindParent", 0x7CU},
    {"WindLocal", 0x80U},
    {"FloorLevel", 0x84U},
    {"WindType", 0x8CU},
    {"Wind", 0xA0U},
    {"Gravity", 0xB0U},
    {"LimitLength", 0xE4U},
    {"<dt>", 0xE0U},
}};

inline constexpr std::uint16_t damping_offset = 0x88U;
inline constexpr std::uint16_t joint_axis_offset = 0x240U;      // Bone axis 0..5
inline constexpr std::uint16_t joint_sim_world_offset = 0xA0U;  // previous result
inline constexpr std::uint16_t joint_velocity_offset = 0x250U;

// "Bone <node> <axis>": index in this table is the stored axis id; 3..5
// point the axis away from the parent.
inline constexpr std::array<std::string_view, 6> axis_names{"X", "Y", "Z", "NX", "NY", "NZ"};

struct Defaults final {
    float stiffness;
    float stiffness_b;  // +0x70
    float max_speed;
    float spring_force;
    std::array<float, 3> gravity;
    float damping;
    float floor_level;
    float dt;
    bool limit_length;
};

inline constexpr Defaults defaults{0.3F, 0.3F, 50.0F, 0.05F, {0.0F, -0.2F, 0.0F},
                                   0.99F, -1000000.0F, 1.0F, true};

inline constexpr float wind_gain = 10.0F;  // w *= 10 * (1 - |cos(w, d)|)

// Sample archives: .clt slot -> simulated model slot.
struct Binding final {
    std::string_view archive_stem;
    std::uint8_t clt_slot;
    std::uint8_t model_slot;
};

inline constexpr std::array<Binding, 9> bindings{{
    {"pl000", 13U, 12U},  // ";pl000_02.clt", coat
    {"em028", 7U, 4U},    // hair (CEm028 init 0x140130480)
    {"em028", 8U, 5U},    // dress
    {"em000", 2U, 3U},
    {"em000", 6U, 7U},
    {"em000", 9U, 10U},
    {"em000", 11U, 12U},
    {"em000", 14U, 15U},
    {"em000", 16U, 17U},
}};

}  // namespace dmc::rengine::profiles::dmc3::cloth_chain

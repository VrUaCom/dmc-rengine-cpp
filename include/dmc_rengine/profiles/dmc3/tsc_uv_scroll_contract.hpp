#pragma once

#include <array>
#include <cstdint>
#include <string_view>

// DMC3 .tsc texture UV scroll (class CDrawUV). Evidence:
// docs/research/dmc3-tsc-uv-scroll-2026-09-23.md
// (dmc3.exe SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082).
namespace dmc::rengine::profiles::dmc3::tsc_uv_scroll {

inline constexpr std::uint64_t cdrawuv_vtable_va = 0x1404C9838ULL;
inline constexpr std::uint64_t idrawuv_vtable_va = 0x1404C9810ULL;
inline constexpr std::uint64_t setup_va = 0x14008BE50ULL;       // vtbl +0x08 (model, text)
inline constexpr std::uint64_t update_va = 0x14008BEE0ULL;      // vtbl +0x10 (dt)
inline constexpr std::uint64_t load_va = 0x14030B5C0ULL;
inline constexpr std::uint64_t parse_va = 0x14030A9B0ULL;
inline constexpr std::uint64_t parse_block_va = 0x14030ABE0ULL;
inline constexpr std::uint64_t tokenizer_va = 0x140322AB0ULL;
inline constexpr std::uint64_t frame_va = 0x14030BDD0ULL;
inline constexpr std::uint64_t record_step_va = 0x14030C1C0ULL;
inline constexpr std::uint64_t type_jump_table_va = 0x14030C2A0ULL;
inline constexpr std::uint64_t mesh_offset_store_va = 0x140309570ULL;

inline constexpr std::uint32_t record_size = 0x80U;
inline constexpr std::uint16_t record_count_header_byte = 0x18U;  // MOD header
inline constexpr std::uint32_t object_scroll_flag_shift = 24U;    // source +0x10, nibble - 1
inline constexpr float offset_scale = 4096.0F;                   // & 0xFFF in the mesh
inline constexpr std::uint16_t mesh_offset_u = 0xF0U;
inline constexpr std::uint16_t mesh_offset_v = 0xF4U;

struct Field final {
    std::string_view keyword;
    std::uint16_t offset;
};

inline constexpr std::array<Field, 12> fields{{
    {"ScrlNo", 0x04U},
    {"ScrlType", 0x06U},
    {"JntNo", 0x07U},
    {"TexNo", 0x08U},
    {"RateUV", 0x20U},      // u; v at +0x28
    {"MinimumUV", 0x24U},   // u; v at +0x2C, flag 4
    {"TimeUV", 0x30U},      // u; v at +0x38
    {"TurnTimeUV", 0x34U},  // u; v at +0x3C (also +0x44 / +0x4C)
    {"InterUV", 0x50U},     // u; v at +0x54 (also counters +0x40 / +0x48)
    {"DirUV", 0x58U},       // u; v at +0x5A
    {"<model>", 0x60U},
    {"RndUV", 0x68U},       // four floats, flag 2
}};

// Jump table 0x14030C2A0 (ScrlType 0..10).
inline constexpr std::array<std::uint64_t, 11> type_handlers{
    0x14030B6F0ULL, 0x14030B720ULL, 0x14030B750ULL, 0x14030B780ULL,
    0x14030B820ULL, 0x14030B980ULL, 0x14030C298ULL, 0x14030C298ULL,
    0x14030C298ULL, 0x14030C298ULL, 0x14030BB50ULL};

// DirUV words: u left 1 / right -1, v up 1 / down -1, stay 0.
inline constexpr std::array<std::string_view, 5> direction_words{"stay", "left", "right", "up",
                                                                 "down"};

struct Binding final {
    std::string_view archive_stem;
    std::uint8_t tsc_slot;
    std::uint8_t model_slot;
    std::string_view owner;
};

inline constexpr std::array<Binding, 3> bindings{{
    {"em028", 13U, 5U, "CEm028 this+0x2D00 (0x1401307FD)"},
    {"em028", 13U, 6U, "CEm028 this+0x2D38 (0x14013089E)"},
    {"em000", 24U, 23U, "CEm005Shl01 this+0xAD0 on the EFM model (0x1400AD757)"},
}};

}  // namespace dmc::rengine::profiles::dmc3::tsc_uv_scroll

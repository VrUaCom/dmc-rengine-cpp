#pragma once

#include <cstdint>

namespace dmc::rengine::analysis::mod {

inline constexpr std::uint32_t source_flag_00100000 = 0x00100000U;
inline constexpr std::uint32_t source_flag_00200000 = 0x00200000U;

inline constexpr std::uint32_t runtime_baseline_flags_offset = 0x10U;
inline constexpr std::uint32_t runtime_effective_flags_offset = 0x14U;

// 0x140305911..0x14030593B refreshes the low mode nibble and bit 20 from
// baseline +0x10 into effective +0x14.
[[nodiscard]] constexpr std::uint32_t restore_low_mode_effective_flags(
    std::uint32_t baseline_flags,
    std::uint32_t effective_flags) noexcept {
    return (effective_flags & 0xFFEFFFF0U) |
           (baseline_flags & 0x0010000FU);
}

// Canonical helper 0x140302640 constructs legacy GS state at runtime +0x80.
// 0x1403027E0/0x1403028F0 place these values in an A+D GIF packet whose
// register IDs prove packet+0x00 -> ZBUF_1 (0x4E) and packet+0x08 -> TEST_1
// (0x47). This is a hardware-state semantic, not an artistic material label.
struct SourceFlag00100000PacketProjection final {
    bool active{};
    std::uint64_t packet00_without_dynamic_low_fields{};
    std::uint64_t packet08{};
};

[[nodiscard]] constexpr SourceFlag00100000PacketProjection
project_source_flag_00100000(std::uint32_t source_flags,
                             std::uint32_t low_mode,
                             std::uint64_t packet00_base) noexcept {
    SourceFlag00100000PacketProjection out{};
    out.active = (source_flags & source_flag_00100000) != 0U;
    out.packet00_without_dynamic_low_fields = packet00_base;

    if ((low_mode & 0x0FU) == 4U) {
        out.packet08 = 0x0000000000050007ULL;
        return out;
    }

    out.packet08 = out.active
        ? 0x000000000005010DULL
        : 0x000000000005000DULL;

    // Active low-mode path: source bit clear sets GS ZBUF_1.ZMSK (bit 32),
    // source bit set leaves ZMSK clear. The base value carries ZBP/PSM.
    if ((low_mode & 0x0FU) != 0U && !out.active) {
        out.packet00_without_dynamic_low_fields |= 0x0000000100000000ULL;
    }
    return out;
}

struct GsTestState final {
    bool alpha_test_enable{};
    std::uint8_t alpha_test_method{};
    std::uint8_t alpha_reference{};
    std::uint8_t alpha_fail_method{};
    bool destination_alpha_test_enable{};
    bool destination_alpha_mode{};
    bool z_test_enable{};
    std::uint8_t z_test_method{};
};

[[nodiscard]] constexpr GsTestState decode_gs_test(std::uint64_t value) noexcept {
    return GsTestState{
        .alpha_test_enable = (value & 0x1ULL) != 0ULL,
        .alpha_test_method = static_cast<std::uint8_t>((value >> 1U) & 0x7ULL),
        .alpha_reference = static_cast<std::uint8_t>((value >> 4U) & 0xFFULL),
        .alpha_fail_method = static_cast<std::uint8_t>((value >> 12U) & 0x3ULL),
        .destination_alpha_test_enable = ((value >> 14U) & 0x1ULL) != 0ULL,
        .destination_alpha_mode = ((value >> 15U) & 0x1ULL) != 0ULL,
        .z_test_enable = ((value >> 16U) & 0x1ULL) != 0ULL,
        .z_test_method = static_cast<std::uint8_t>((value >> 17U) & 0x3ULL),
    };
}

[[nodiscard]] constexpr bool decode_gs_zbuf_zmsk(std::uint64_t value) noexcept {
    return ((value >> 32U) & 0x1ULL) != 0ULL;
}

// 0x140302AB2 reads serialized object +0x10 and 0x140302ABF/0x140302AC9 copy
// the complete word to runtime object +0x14 and +0x10. Source bit 0x00200000
// is therefore EXE_CONFIRMED as carried baseline/effective state, while its
// distinct terminal semantic remains unresolved.
struct SourceFlag00200000CarryProjection final {
    bool active{};
    std::uint32_t runtime_flags10{};
    std::uint32_t runtime_flags14{};
};

[[nodiscard]] constexpr SourceFlag00200000CarryProjection
project_source_flag_00200000_carry(std::uint32_t source_flags) noexcept {
    return SourceFlag00200000CarryProjection{
        .active = (source_flags & source_flag_00200000) != 0U,
        .runtime_flags10 = source_flags,
        .runtime_flags14 = source_flags,
    };
}

// Whole-model follow-up census outside the local 0x140302640 state builder.
// 0x1402F28E0 reads effective +0x14 and sets bit17 (0x00020000), preserving all
// pre-existing high bits including source-carried bit21. This is a mutation of
// the effective container, not a semantic interpretation of bit21.
inline constexpr std::uintptr_t effective_flag_mutator_0x1402f28e0 =
    0x1402F28E0ULL;
inline constexpr std::uint32_t effective_flag_mutator_set_mask = 0x00020000U;

// The MOD/EFM material construction paths at 0x1402F9ED9 and 0x1402FA042 read
// runtime effective +0x14 and pass the whole dword as R8D to 0x1402F9890.
// Direct disassembly of that helper shows its only flag mask is 0x00004000,
// selecting the legacy GS TEX1 filtering state. Therefore source bit21 reaches
// a common material consumer but is not interpreted there either.
inline constexpr std::uintptr_t common_material_flag_helper = 0x1402F9890ULL;
inline constexpr std::uintptr_t material_effective_flags_read_mod_path =
    0x1402F9ED9ULL;
inline constexpr std::uintptr_t material_effective_flags_read_parallel_path =
    0x1402FA042ULL;
inline constexpr std::uint32_t common_material_interpreted_flag_mask =
    0x00004000U;

static_assert(runtime_baseline_flags_offset == 0x10U);
static_assert(runtime_effective_flags_offset == 0x14U);
static_assert(restore_low_mode_effective_flags(0x00100005U, 0x0020000AU) ==
              0x00300005U);

constexpr auto bit20_set = project_source_flag_00100000(0x00100001U, 1U, 0U);
constexpr auto bit20_clear = project_source_flag_00100000(0x00000001U, 1U, 0U);
static_assert(bit20_set.active);
static_assert(bit20_set.packet08 == 0x000000000005010DULL);
static_assert(bit20_clear.packet08 == 0x000000000005000DULL);
static_assert(decode_gs_test(bit20_set.packet08).alpha_test_enable);
static_assert(decode_gs_test(bit20_set.packet08).alpha_test_method == 6U);
static_assert(decode_gs_test(bit20_set.packet08).alpha_reference == 0x10U);
static_assert(decode_gs_test(bit20_clear.packet08).alpha_reference == 0x00U);
static_assert(decode_gs_test(bit20_set.packet08).z_test_enable);
static_assert(decode_gs_test(bit20_set.packet08).z_test_method == 2U);
static_assert(decode_gs_zbuf_zmsk(bit20_clear.packet00_without_dynamic_low_fields));
static_assert(!decode_gs_zbuf_zmsk(bit20_set.packet00_without_dynamic_low_fields));
static_assert(project_source_flag_00100000(0x00100004U, 4U, 0U).packet08 ==
              0x0000000000050007ULL);

constexpr auto source_00200000_synthetic =
    project_source_flag_00200000_carry(0xA5200000U);
static_assert(source_00200000_synthetic.active);
static_assert(source_00200000_synthetic.runtime_flags10 == 0xA5200000U);
static_assert(source_00200000_synthetic.runtime_flags14 == 0xA5200000U);
static_assert((effective_flag_mutator_set_mask & source_flag_00200000) == 0U);
static_assert((common_material_interpreted_flag_mask & source_flag_00200000) == 0U);

} // namespace dmc::rengine::analysis::mod

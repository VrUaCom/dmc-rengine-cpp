#pragma once

#include <cstdint>

namespace dmc::rengine::analysis::mod {

inline constexpr std::uint32_t source_flag_00100000 = 0x00100000U;
inline constexpr std::uint32_t source_flag_00200000 = 0x00200000U;

// Canonical helper 0x140302640 receives the source/runtime flag word in EDX.
// This projection deliberately stops at the packet/state bytes directly proven
// by the executable. It does not assign an artistic/material name to the bit.
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

    // 0x1403026B8..0x1403026F4: low mode 4 is a separate fixed packet path and
    // does not use the 0x00100000 choice.
    if ((low_mode & 0x0FU) == 4U) {
        out.packet08 = 0x0000000000050007ULL;
        return out;
    }

    // 0x1403026D1..0x1403026F3.
    out.packet08 = out.active
        ? 0x000000000005010DULL
        : 0x000000000005000DULL;

    // 0x140302747..0x14030278B: with an active low-mode path, the clear-bit
    // branch ORs 0x0000000100000000 into packet +0x00 while the set-bit branch
    // leaves that high flag clear. The low-mode/index pieces are assembled by
    // the same helper and are intentionally outside this bounded projection.
    if ((low_mode & 0x0FU) != 0U && !out.active) {
        out.packet00_without_dynamic_low_fields |= 0x0000000100000000ULL;
    }
    return out;
}

// 0x140302AB2 reads serialized object +0x10 and 0x140302ABF/0x140302AC9 copy
// the complete word to runtime object +0x14 and +0x10. Therefore serialized
// bit 0x00200000 is EXE_CONFIRMED as carried state even though a distinct
// downstream semantic consumer has not yet been proven.
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

// Compile-time regression guards for the exact recovered packet/carry rules.
// These protect proven byte/state projection only, not a guessed visual label.
static_assert(project_source_flag_00100000(0x00100001U, 1U, 0U).active);
static_assert(project_source_flag_00100000(0x00100001U, 1U, 0U).packet08 ==
              0x000000000005010DULL);
static_assert(project_source_flag_00100000(0x00000001U, 1U, 0U).packet08 ==
              0x000000000005000DULL);
static_assert((project_source_flag_00100000(0x00000001U, 1U, 0U)
                   .packet00_without_dynamic_low_fields &
               0x0000000100000000ULL) != 0ULL);
static_assert(project_source_flag_00100000(0x00100004U, 4U, 0U).packet08 ==
              0x0000000000050007ULL);

constexpr auto source_00200000_synthetic =
    project_source_flag_00200000_carry(0xA5200000U);
static_assert(source_00200000_synthetic.active);
static_assert(source_00200000_synthetic.runtime_flags10 == 0xA5200000U);
static_assert(source_00200000_synthetic.runtime_flags14 == 0xA5200000U);

} // namespace dmc::rengine::analysis::mod

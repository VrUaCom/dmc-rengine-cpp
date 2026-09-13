#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm::runtime {

// Source object +0x10 masks recovered from the canonical DMC3 executable.
// Names are intentionally operational unless a downstream semantic has been
// independently closed.
inline constexpr std::uint32_t source_low_mode_mask = 0x0000000FU;
inline constexpr std::uint32_t source_mask_00000020 = 0x00000020U;
inline constexpr std::uint32_t source_mask_00010000 = 0x00010000U;
inline constexpr std::uint32_t source_mask_00020000 = 0x00020000U;
inline constexpr std::uint32_t source_mask_00040000 = 0x00040000U;
inline constexpr std::uint32_t source_mask_00080000 = 0x00080000U;
inline constexpr std::uint32_t source_mask_00100000 = 0x00100000U;

// Observed on 19 current-corpus objects. The canonical SCM object pipeline
// copies this bit into both baseline (+0x10) and mutable effective (+0x14)
// runtime flags and preserves it across low-mode mutations. The bounded
// consumers recovered so far do not decode its terminal semantic.
// This is therefore preserved/undecoded evidence, NOT a global-unused claim.
inline constexpr std::uint32_t source_mask_00200000 = 0x00200000U;

inline constexpr std::uint32_t source_high_mode_mask = 0x0F000000U;

// Union of source flag bits observed across the original structural SCM corpus.
// The expanded 51-unique stage-PAC census is compatible with this superset.
// EXE-supported masks are broader; absence from a corpus is not rejection.
inline constexpr std::uint32_t observed_corpus_source_mask = 0x003A0003U;

inline constexpr std::uint32_t runtime_flag_bit_4 = 1U << 4U;
inline constexpr std::uint32_t runtime_flag_bit_5 = 1U << 5U;
inline constexpr std::uint32_t runtime_flag_bit_7 = 1U << 7U;
inline constexpr std::uint32_t runtime_flag_bit_8 = 1U << 8U;
inline constexpr std::uint32_t runtime_flag_bit_9 = 1U << 9U;
inline constexpr std::uint32_t runtime_flag_bit_10 = 1U << 10U;
inline constexpr std::uint32_t runtime_flag_bit_15 = 1U << 15U;

// Deep HD compatibility-path closure:
//   source object flag 0x00080000
//     -> 0x1403033E3..0x1403033FC sets runtimeObject+0x00 bit 0x20
//     -> 0x14030DB50 writes runtimeObject+0x05 = 2 (otherwise 3)
//     -> 0x14030DBA0 emits 0x5C000000 | runtimeObject+0x05
//     -> 0x1400331E6 installs the compatibility object selector
//     -> 0x140044310 jump table.
// Selector 2 and selector 3 deliberately alias the same VS base key 13, so the
// source bit changes compatibility selector state without changing the base
// vertex-shader table key on this SCM converter path.
inline constexpr std::uint8_t scm_compat_selector_with_source_00080000 = 2U;
inline constexpr std::uint8_t scm_compat_selector_without_source_00080000 = 3U;
inline constexpr std::uint8_t scm_compat_invalid_vs_base_key = 0xFFU;

[[nodiscard]] constexpr std::uint8_t scm_compatibility_object_selector(
    std::uint32_t source_flags) noexcept {
    return (source_flags & source_mask_00080000) != 0U
        ? scm_compat_selector_with_source_00080000
        : scm_compat_selector_without_source_00080000;
}

// Exact selector table at 0x1400446C4 for selector values 2..12. Selector 6
// exits the converter before shader binding and therefore has no VS base key.
// The table is kept numeric because no original/artistic shader names are
// evidenced. SCM object initialization currently produces only 2 or 3.
[[nodiscard]] constexpr std::uint8_t scm_compatibility_vs_base_key(
    std::uint8_t selector) noexcept {
    switch (selector) {
    case 2U:
    case 3U: return 13U;
    case 4U: return 5U;
    case 5U: return 8U;
    case 6U: return scm_compat_invalid_vs_base_key;
    case 7U: return 10U;
    case 8U: return 11U;
    case 9U: return 9U;
    case 10U: return 7U;
    case 11U: return 12U;
    case 12U: return 6U;
    default: return scm_compat_invalid_vs_base_key;
    }
}

static_assert(scm_compatibility_object_selector(0U) == 3U);
static_assert(scm_compatibility_object_selector(source_mask_00080000) == 2U);
static_assert(scm_compatibility_vs_base_key(2U) == 13U);
static_assert(scm_compatibility_vs_base_key(3U) == 13U);
static_assert(scm_compatibility_vs_base_key(6U) == scm_compat_invalid_vs_base_key);

struct Projection final {
    // Bits OR/set by the recovered SCM-like runtime object initialization path.
    std::uint32_t runtime_flags_to_set{};

    // Inputs selected for helper 0x140302640. These are exact numeric runtime
    // contracts; their higher-level semantic names remain unresolved.
    std::uint8_t helper_mode{};
    std::uint32_t helper_state_selector{};
    bool helper_secondary_boolean{};

    // Source 0x00020000 causes +0x160/+0x164/+0x168 = 1.0 and +0x16C = 0.
    bool initialize_unit_vector{};

    // Source high nibble causes runtime flag bit 15 and stores nibble-1 at +0x0D.
    bool high_mode_present{};
    std::uint8_t high_mode_minus_one{};

    // Bits that the bounded recovered projection intentionally does not
    // interpret. They remain part of source/effective flags and must survive
    // round-trip authoring unchanged.
    std::uint32_t preserved_undecoded_source_bits{};
};

[[nodiscard]] constexpr Projection project(std::uint32_t source_flags) noexcept {
    Projection out{};
    const auto low_mode = static_cast<std::uint8_t>(
        source_flags & source_low_mode_mask);
    const auto high_mode = static_cast<std::uint8_t>(
        (source_flags & source_high_mode_mask) >> 24U);

    if (low_mode != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_8;
    }
    if ((source_flags & source_mask_00000020) != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_10;
    }
    if ((source_flags & source_mask_00020000) != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_9;
        out.initialize_unit_vector = true;
    }
    if ((source_flags & source_mask_00010000) != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_7;
    }
    if ((source_flags & source_mask_00040000) != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_4;
    }
    if (high_mode != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_15;
        out.high_mode_present = true;
        out.high_mode_minus_one = static_cast<std::uint8_t>(high_mode - 1U);
    }
    if ((source_flags & source_mask_00080000) != 0U) {
        out.runtime_flags_to_set |= runtime_flag_bit_5;
    }

    out.helper_secondary_boolean =
        (source_flags & source_mask_00010000) == 0U;

    if (low_mode == 0U) {
        out.helper_mode = 9U;
        out.helper_state_selector = 0x0005080BU;
    } else {
        out.helper_mode = low_mode;
        if (low_mode == 4U) {
            out.helper_state_selector = 0x00050007U;
        } else if ((source_flags & source_mask_00100000) != 0U) {
            out.helper_state_selector = 0x0005010DU;
        } else {
            out.helper_state_selector = 0x0005000DU;
        }
    }

    out.preserved_undecoded_source_bits =
        source_flags & source_mask_00200000;
    return out;
}

} // namespace dmc::rengine::formats::scm::runtime

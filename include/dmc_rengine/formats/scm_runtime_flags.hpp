#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm::runtime {

// Source object +0x10 masks recovered from the canonical DMC3 executable.
// Names are intentionally operational rather than semantic: the EXE behavior
// is proven, but gameplay/rendering meanings of these bits are not yet closed.
inline constexpr std::uint32_t source_low_mode_mask = 0x0000000FU;
inline constexpr std::uint32_t source_mask_00000020 = 0x00000020U;
inline constexpr std::uint32_t source_mask_00010000 = 0x00010000U;
inline constexpr std::uint32_t source_mask_00020000 = 0x00020000U;
inline constexpr std::uint32_t source_mask_00040000 = 0x00040000U;
inline constexpr std::uint32_t source_mask_00080000 = 0x00080000U;
inline constexpr std::uint32_t source_mask_00100000 = 0x00100000U;
inline constexpr std::uint32_t source_high_mode_mask = 0x0F000000U;

// Union of source flag bits observed across the current 68-file SCM corpus.
// EXE-supported masks are broader; absence from this corpus is not rejection.
inline constexpr std::uint32_t observed_corpus_source_mask = 0x003A0003U;

inline constexpr std::uint32_t runtime_flag_bit_4 = 1U << 4U;
inline constexpr std::uint32_t runtime_flag_bit_5 = 1U << 5U;
inline constexpr std::uint32_t runtime_flag_bit_7 = 1U << 7U;
inline constexpr std::uint32_t runtime_flag_bit_8 = 1U << 8U;
inline constexpr std::uint32_t runtime_flag_bit_9 = 1U << 9U;
inline constexpr std::uint32_t runtime_flag_bit_10 = 1U << 10U;
inline constexpr std::uint32_t runtime_flag_bit_15 = 1U << 15U;

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

    return out;
}

} // namespace dmc::rengine::formats::scm::runtime

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the `EFM` effect-model payload.
//
// The third of the four type handlers, and the closest of them to another:
// `EFM` is `MOD`'s batch with one more array. Everything else — the document
// shell, the indexed batches, the two-byte control word, the strip rebuild,
// the cleared break flag — is the same instruction for instruction.
//
// **No corpus.** The supplied stage and HUD data contain no `EFM` payload at
// all, so unlike `SCM` and `MOD` this layout has never been checked against a
// real file. It is recovered, not corroborated, and the difference is recorded
// rather than smoothed over.
struct EfmContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t relocate_va = 0x1402F7A90ULL;

    static constexpr std::string_view magic = "EFM";
    static constexpr std::size_t magic_bytes = 3U;

    static constexpr std::size_t batch_stride = 0x50U;
    static constexpr bool batches_are_indexed = true;

    static constexpr std::size_t batch_index_count_offset = 0x00U;
    static constexpr std::size_t batch_position_offset = 0x10U;
    static constexpr std::size_t batch_normal_offset = 0x18U;
    static constexpr std::size_t batch_attribute_offset = 0x20U;
    static constexpr std::size_t batch_secondary_offset = 0x28U;
    static constexpr std::size_t batch_index_offset = 0x30U;
    // The one field `MOD` does not relocate and this does. What it holds is
    // not recovered — only that it is an offset into the document, which is
    // enough to bound it and not enough to name it.
    static constexpr std::size_t batch_extra_offset = 0x38U;
    static constexpr std::size_t batch_strip_offset = 0x40U;
    static constexpr std::size_t batch_strip_length_offset = 0x48U;

    static constexpr std::size_t position_element_bytes = 12U;
    static constexpr std::size_t normal_element_bytes = 12U;
    static constexpr std::size_t attribute_element_bytes = 4U;
    static constexpr std::size_t secondary_element_bytes = 4U;
    static constexpr std::size_t index_element_bytes = 2U;
    static constexpr std::size_t array_alignment = 16U;

    static constexpr std::uint16_t index_break_mask = 0x8000U;
    static constexpr std::uint16_t index_value_mask = 0x7FFFU;
    static constexpr bool break_flag_cleared_after_rebuild = true;
    static constexpr std::uint32_t first_strip_index = 2U;

    static constexpr std::uint16_t strip_buffer_marker = 0x1212U;
    static constexpr std::size_t strip_element_bytes = 2U;
    static constexpr bool strip_length_always_written = true;

    // `MOD` tests two bytes of its secondary array under `document_mode == 1`.
    // This routine has no such branch, so the mode byte is carried and never
    // acted on here.
    static constexpr bool reads_document_mode = false;

    // The element width of the extra array is unknown, so its extent cannot be
    // bounded the way the others can. A reader may only check that its base is
    // inside the document.
    static constexpr bool extra_array_extent_is_known = false;
};

} // namespace dmc::rengine::profiles::dmc3

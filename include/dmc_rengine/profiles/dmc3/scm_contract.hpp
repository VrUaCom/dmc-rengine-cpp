#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the `SCM` scene-model payload.
//
// `ResourceTypeContract` recovered which handler the runtime calls for an
// `SCM` payload; this is that handler read out. It is the routine that turns
// the stored file into something the renderer can walk: every stored offset
// becomes a pointer, and a triangle-strip index list is rebuilt in place.
//
// Reading it gives the layout without a single guess, and the layout then
// verifies against the corpus exactly — 77 primitive batches in `st001` and 72
// in `st114`, every relocated offset in range, every strip buffer carrying the
// marker the routine tests for, and the array packing reproducing every
// recorded offset arithmetically.
struct ScmContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // The handler `ResourceTypeContract::tagged_types` names for tag `SCM`.
    static constexpr std::uint64_t relocate_va = 0x1403051B0ULL;

    static constexpr std::string_view magic = "SCM";
    static constexpr std::size_t magic_bytes = 3U;

    // Document header.
    //
    // The group count is read as a *byte*, not a dword: the routine does
    // `movzx eax, byte ptr [rax+0x10]`. The upper three bytes of that dword
    // are something else and are not part of the count.
    static constexpr std::size_t group_count_offset = 0x10U;
    static constexpr std::size_t document_pointer_offset = 0x20U;
    static constexpr std::size_t group_table_offset = 0x40U;
    static constexpr std::size_t group_stride = 0x40U;

    // Group. The batch count is likewise a byte.
    static constexpr std::size_t group_batch_count_offset = 0x00U;
    static constexpr std::size_t group_batch_pointer_offset = 0x08U;

    // Primitive batch. Every offset below is stored relative to the document
    // base and rewritten in place to an absolute pointer — except the strip
    // buffer, which is relative to the batch itself. Mixing those two bases up
    // is the one mistake this layout invites, so both are named.
    static constexpr std::size_t batch_index_count_offset = 0x00U;
    static constexpr std::size_t batch_position_offset = 0x10U;
    static constexpr std::size_t batch_normal_offset = 0x18U;
    static constexpr std::size_t batch_attribute_offset = 0x20U;
    static constexpr std::size_t batch_next_stride_offset = 0x28U;
    static constexpr std::size_t batch_index_offset = 0x38U;
    static constexpr std::size_t batch_strip_offset = 0x40U;
    static constexpr std::size_t batch_strip_length_offset = 0x48U;
    static constexpr std::size_t batch_stride = 0x50U;

    static constexpr std::size_t position_element_bytes = 12U;
    static constexpr std::size_t normal_element_bytes = 12U;
    static constexpr std::size_t attribute_element_bytes = 4U;
    static constexpr std::size_t index_element_bytes = 4U;
    static constexpr std::size_t array_alignment = 16U;

    // The arrays of a group are stored one *kind* at a time: every batch's
    // positions, then every batch's normals, and so on. A reader that assumed
    // each batch owned a contiguous run of its own four arrays would compute
    // the right offsets only for a group with a single batch — which is most
    // of them, and none of the interesting ones.
    static constexpr bool arrays_are_grouped_by_kind = true;

    // Per-index skip flag: byte 3 of the index element, mask 2. An index the
    // routine finds flagged breaks the strip run rather than extending it.
    static constexpr std::size_t index_flag_byte = 3U;
    static constexpr std::uint8_t index_skip_mask = 0x02U;

    // The first two indices of a batch never start a run.
    static constexpr std::uint32_t first_strip_index = 2U;

    // The strip buffer is scratch. The runtime writes the rebuilt index list
    // into it and updates the length field only when the buffer opens with
    // this marker, which every batch in the corpus does.
    static constexpr std::uint16_t strip_buffer_marker = 0x1212U;
    static constexpr std::size_t strip_element_bytes = 2U;

    [[nodiscard]] static consteval std::size_t aligned_array_bytes(
        std::size_t count,
        std::size_t element_bytes) noexcept {
        const auto raw = count * element_bytes;
        return (raw + array_alignment - 1U) / array_alignment * array_alignment;
    }
};

} // namespace dmc::rengine::profiles::dmc3

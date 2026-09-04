#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the `MOD` model payload.
//
// `MOD` and `SCM` share a document shell and nothing else. Both relocation
// routines read a byte count at `+0x10`, a pointer at `+0x20` and a table of
// `0x40`-byte groups at `+0x40` — and then disagree about every field of a
// batch. Treating the two as one format would have produced a reader that
// works on stages and silently mis-addresses every object model.
struct ModContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // The handler `ResourceTypeContract::tagged_types` names for tag `MOD`.
    static constexpr std::uint64_t relocate_va = 0x1402FE3B0ULL;

    static constexpr std::string_view magic = "MOD";
    static constexpr std::size_t magic_bytes = 3U;

    // Batches are reached by index from the group pointer, not by walking a
    // stored stride. `SCM` chains its batches through a field this structure
    // uses for an array instead, so the same walk applied to both would read
    // an array pointer as a stride.
    static constexpr std::size_t batch_stride = 0x50U;
    static constexpr bool batches_are_indexed = true;

    // Batch fields. Everything here relocates against the document except the
    // strip buffer, which relocates against its own batch.
    static constexpr std::size_t batch_index_count_offset = 0x00U;
    static constexpr std::size_t batch_position_offset = 0x10U;
    static constexpr std::size_t batch_normal_offset = 0x18U;
    static constexpr std::size_t batch_attribute_offset = 0x20U;
    static constexpr std::size_t batch_secondary_offset = 0x28U;
    static constexpr std::size_t batch_index_offset = 0x30U;
    static constexpr std::size_t batch_strip_offset = 0x40U;
    static constexpr std::size_t batch_strip_length_offset = 0x48U;

    static constexpr std::size_t position_element_bytes = 12U;
    static constexpr std::size_t normal_element_bytes = 12U;
    static constexpr std::size_t attribute_element_bytes = 4U;
    static constexpr std::size_t secondary_element_bytes = 4U;
    // Two bytes, where `SCM` uses four. This is the difference that decides
    // where every array after it begins.
    static constexpr std::size_t index_element_bytes = 2U;
    static constexpr std::size_t array_alignment = 16U;

    // Per-vertex control word. The high bit breaks the strip run; the routine
    // clears it in place once the list is built, so a loaded document no
    // longer carries the flags a stored one does.
    static constexpr std::uint16_t index_break_mask = 0x8000U;
    static constexpr std::uint16_t index_value_mask = 0x7FFFU;
    static constexpr bool break_flag_cleared_after_rebuild = true;

    // The first two vertices of a batch never start a run.
    static constexpr std::uint32_t first_strip_index = 2U;

    // The strip buffer opens with this marker, as in `SCM`. Unlike `SCM`, the
    // rebuilt length is written whether or not the marker is there.
    static constexpr std::uint16_t strip_buffer_marker = 0x1212U;
    static constexpr std::size_t strip_element_bytes = 2U;
    static constexpr bool strip_length_always_written = true;

    // `document_mode == 1` gates an extra test the routine performs on bytes 1
    // and 2 of the secondary array. What that test guards is not recovered, so
    // the value is carried and not acted on.
    static constexpr std::uint8_t document_mode_with_secondary_test = 1U;

    [[nodiscard]] static consteval std::size_t aligned_array_bytes(
        std::size_t count,
        std::size_t element_bytes) noexcept {
        const auto raw = count * element_bytes;
        return (raw + array_alignment - 1U) / array_alignment * array_alignment;
    }
};

} // namespace dmc::rengine::profiles::dmc3

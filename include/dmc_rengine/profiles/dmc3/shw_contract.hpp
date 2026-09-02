#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the `SHW` shadow payload.
//
// The fourth type handler, and the one that breaks the pattern. `SCM`, `MOD`
// and `EFM` share a document shell — a byte count at `+0x10`, a pointer at
// `+0x20`, a group table at `+0x40`. `SHW` keeps the count and nothing else:
// its table starts at `+0x30`, it relocates four pointers per entry, and it
// has no groups, no batches, no strip rebuild and no `+0x20` pointer.
//
// Recording that is the point. Three formats agreeing made the shell look
// universal, and a fourth reader written on that assumption would have read a
// shadow document's first entry as a group header.
//
// **No corpus.** The supplied data contains no `SHW` payload, so this is
// recovered and not corroborated.
struct ShwContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t relocate_va = 0x1403204C0ULL;

    static constexpr std::string_view magic = "SHW";
    static constexpr std::size_t magic_bytes = 3U;

    // The one field it shares with the model shell.
    static constexpr std::size_t entry_count_offset = 0x10U;

    static constexpr std::size_t entry_table_offset = 0x30U;
    static constexpr std::size_t entry_stride = 0x40U;

    // Four offsets per entry, all relocated against the document. The routine
    // touches nothing else in the entry, so the remaining 32 bytes of each
    // record are unread here.
    static constexpr std::size_t entry_pointer_count = 4U;
    static constexpr std::size_t entry_pointer_stride = 0x08U;

    // The count is a byte, as in the model shell.
    static constexpr std::uint32_t max_entry_count = 256U;

    // No element widths are recoverable from this routine: it relocates the
    // four bases and never indexes through them. A reader can bound the bases
    // and must not claim an extent.
    static constexpr bool array_extents_are_known = false;

    [[nodiscard]] static consteval std::size_t entry_offset(
        std::size_t index) noexcept {
        return entry_table_offset + index * entry_stride;
    }
};

} // namespace dmc::rengine::profiles::dmc3

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Does the original runtime read a stage container's slot 0 name list?
//
// `st001.pac` slot 0 is exactly `st001.ptx\r\nst001.scm\r\nst001.sch\r\n`, and
// this project has carried an open question about it: are those three lines a
// runtime naming authority that binds slots 1, 2 and 3, or are they pack-time
// metadata the game never looks at?
//
// **They are pack-time metadata.** The runtime reaches slot 0, hands it to the
// type dispatcher, and the dispatcher finds it is of no known type and returns.
// That is stronger than "ignored": the payload is consumed and discarded.
//
// The evidence is below, and it is a proof rather than a failure to find a
// consumer.
struct SlotZeroManifestContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    // --- 1. Slot 0 is reached ------------------------------------------------
    //
    // The PAC walk's index register starts at 2, and the load is
    // `[base + index*4]`, so the first iteration reads byte offset 8 — the
    // first offset-table entry, which is slot 0. In `st001.pac` that entry is
    // 0x30, non-zero, so the walk forms a pointer and dispatches it.
    static constexpr std::uint64_t pac_walk_index_init_va = 0x1401B9316ULL;
    static constexpr std::uint32_t pac_walk_first_dword_index = 2U;
    static constexpr std::size_t offset_table_offset = 8U;
    static constexpr bool slot_zero_is_dispatched = true;

    // --- 2. And the dispatcher does nothing with it --------------------------
    //
    // Every branch of the dispatcher turns on the payload's first byte, tested
    // against these four. A stage manifest begins `st001…`, so the first byte
    // is 's', 0x73, and no branch is taken.
    static constexpr std::uint64_t dispatcher_va = 0x1401B9FA0ULL;
    static constexpr std::array<char, 4> dispatcher_first_byte_alternatives{
        'M', 'E', 'S', 'P'};
    static constexpr char manifest_first_byte = 's';
    static constexpr bool dispatcher_handles_the_manifest = false;

    // --- 3. Nothing else walks a container -----------------------------------
    //
    // A sweep of every container-relative slot walk — an indexed dword load
    // from a base, added back to that base — finds 46 sites image-wide. Only
    // two of them are preceded by a comparison establishing the base is a
    // relative-slot container, and there is exactly one such comparison for
    // each magic in the whole image.
    static constexpr std::size_t container_relative_walk_sites = 46U;
    static constexpr std::size_t magic_checked_walks = 2U;
    static constexpr std::uint64_t pac_magic_compare_va = 0x1401B92FEULL;
    static constexpr std::uint64_t pnst_magic_compare_va = 0x1401BA029ULL;

    // Neither the namespace resolver nor the semantic materialization path
    // contains one, so a name never reaches a container's slot table through
    // either. Recorded as the ranges that were checked and found clear.
    static constexpr std::uint64_t resolver_range_first = 0x14002F000ULL;
    static constexpr std::uint64_t resolver_range_last = 0x140030000ULL;
    static constexpr std::uint64_t materialization_range_first = 0x140338000ULL;
    static constexpr std::uint64_t materialization_range_last = 0x140339000ULL;
    static constexpr bool walk_in_resolver_range = false;
    static constexpr bool walk_in_materialization_range = false;

    // --- 4. The names could not be used even if they were read ---------------
    //
    // Two of the three extensions have no representation anywhere in the
    // image, in any case, and no format string can construct one. The third,
    // `.ptx`, exists only as a registry literal and in two catalogue names
    // that are not stage textures.
    static constexpr std::size_t sch_occurrences_in_image = 0U;
    static constexpr std::size_t scm_string_occurrences_in_image = 0U;
    static constexpr bool names_are_constructible_by_format_string = false;

    // And routing them would type nothing. `SCM` is recognized by its magic,
    // never by an extension, so a `.scm` line adds nothing a tag does not
    // already say. `.sch` matches no entry in either registry, and the
    // animation registry refuses an unmatched name outright.
    static constexpr bool scm_has_an_extension_entry = false;
    static constexpr bool sch_has_an_extension_entry = false;

    // --- The verdict ---------------------------------------------------------
    static constexpr bool runtime_consults_the_manifest = false;
    static constexpr bool manifest_is_build_metadata = true;

    // What this does *not* say. The manifest is still a true statement about
    // the file, and this project still shows it — attributed as the
    // container's own text, corroborated where a payload's independently read
    // type agrees. Establishing that the game ignores it changes its
    // authority, not its accuracy.
    static constexpr bool manifest_remains_worth_showing = true;

    // The original runtime's own naming authority for a loose container is
    // `.lst`, recovered separately: the representation selector prefers the
    // packed `.pac` and rewrites the extension to `.lst` when it is absent.
    static constexpr std::uint64_t loose_representation_selector_va =
        0x1401B79E0ULL;
    static constexpr std::uint64_t loose_extension_rewrite_va = 0x1401B9390ULL;
};

} // namespace dmc::rengine::profiles::dmc3

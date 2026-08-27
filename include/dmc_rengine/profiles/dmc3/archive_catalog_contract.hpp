#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// How the original runtime turns a requested archive name into a file, and the
// catalogue of names it carries.
//
// This contract deliberately holds two claims of different strength, and says
// which is which. The search-path table is recovered: two independent routines
// load it, index it, and bound the loop at six. The name catalogue is only
// observed: it is unmistakably there — 4,039 ordered pointers into the .rdata
// string pool — but a 99.87%-coverage linear sweep of `.text` finds no
// instruction that references its base, and no instruction that references any
// of the strings it points at directly either.
//
// The honest consequence is stated below as `catalog_read_site_found = false`.
// A catalogue whose reader cannot be located is a strong source of candidate
// names and not an authority on their order, and a tool built on it must
// attribute names that way rather than presenting them as the game's own.
struct ArchiveCatalogContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // --- Recovered: the search path ---------------------------------------

    // The resolver: it walks the requested name backwards to the last '\' or
    // '/', keeps the basename, then formats prefix + basename into a 0x400
    // buffer and tries to open it, once per prefix.
    static constexpr std::uint64_t resolver_va = 0x14002E7D0ULL;
    // The indexed fetch and the bound. `cmp edi, 6` is the whole reason the
    // table's length is a fact rather than a count of the entries that happen
    // to look like strings.
    static constexpr std::uint64_t resolver_table_fetch_va = 0x14002E840ULL;
    // A second routine over the same table with the same bound. One routine
    // reading six entries could be reading past the end of a five-entry table.
    // Two agreeing is what settles it.
    static constexpr std::uint64_t second_resolver_va = 0x14002FD55ULL;

    static constexpr std::uint64_t search_prefix_table_va = 0x14055AEF8ULL;
    static constexpr std::size_t search_prefix_count = 6U;
    static constexpr std::size_t search_prefix_entry_bytes = 8U;

    // In the order the resolvers try them. The last is the empty string, which
    // is how a name with no prefix still resolves.
    static constexpr std::array<std::string_view, search_prefix_count>
        search_prefixes{
            "GDataX360.afs/",
            "GData.afs/",
            "Video/",
            "afs/sound/",
            "SAVEDATA/",
            "",
        };

    // The prefix this project already used as the logical root for a mounted
    // volume, chosen from the corpus before this was read. It is in the table,
    // second, which corroborates the choice rather than establishing it.
    static constexpr std::string_view volume_prefix = "GData.afs/";
    static constexpr std::size_t volume_prefix_index = 1U;

    // The resolver strips a directory component before applying a prefix, so a
    // catalogue entry that already carries one (`scr\st307.pac`) resolves by
    // its basename.
    static constexpr bool resolver_strips_directory = true;
    static constexpr char directory_separator_backslash = '\\';
    static constexpr char directory_separator_slash = '/';

    // --- Observed: the catalogue ------------------------------------------

    static constexpr std::uint64_t catalog_va = 0x140553050ULL;
    static constexpr std::size_t catalog_entry_count = 4039U;
    static constexpr std::size_t catalog_entry_bytes = 8U;
    static constexpr std::uint64_t catalog_last_entry_va = 0x14055AE80ULL;

    // The negative, stated so nothing downstream can quietly assume otherwise.
    // A linear sweep with resynchronization covering 99.87% of `.text` finds
    // no reference to the catalogue's base, and none to the individual name
    // strings either.
    static constexpr bool catalog_read_site_found = false;
    static constexpr bool catalog_index_is_a_known_identifier = false;

    // What the catalogue does say, checked over its whole extent.
    static constexpr std::size_t catalog_pac_entry_count = 3398U;

    // A stage contributes four consecutive entries, not three: the stage, its
    // config, its effects and its sound bank.
    //
    //     [276] st001.pac  [277] st001cfg.pac
    //     [278] st001_effect.pac  [279] snd_r001.pac
    //
    // This started as a claim of three, made from the two stages the corpus
    // contains, and an assertion over the whole catalogue refused it: 380 - 276
    // is 104, which is not a multiple of three. Checked properly, 175 of the
    // 182 stage groups are complete quadruples. The seven exceptions are all in
    // the `st6xx` range, which carries a stage and a config and neither an
    // effect pack nor a sound bank.
    static constexpr std::size_t stage_group_stride = 4U;
    static constexpr std::size_t stage_group_count = 182U;
    static constexpr std::size_t complete_stage_group_count = 175U;
    static constexpr std::size_t st001_catalog_index = 276U;
    static constexpr std::size_t st114_catalog_index = 380U;
    static constexpr std::string_view st001_name = "st001.pac";
    static constexpr std::string_view st001_config_name = "st001cfg.pac";
    static constexpr std::string_view st001_effect_name = "st001_effect.pac";
    static constexpr std::string_view st001_sound_name = "snd_r001.pac";

    [[nodiscard]] static constexpr std::uint64_t search_prefix_entry_va(
        std::size_t index) noexcept {
        return search_prefix_table_va + index * search_prefix_entry_bytes;
    }

    [[nodiscard]] static constexpr std::uint64_t catalog_entry_va(
        std::size_t index) noexcept {
        return catalog_va + index * catalog_entry_bytes;
    }
};

} // namespace dmc::rengine::profiles::dmc3

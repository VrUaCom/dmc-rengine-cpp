#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// The effect container carries its own names.
//
// This project has been attributing slot names as invented, because a
// relative-slot container stores none. That is true of `PAC` and of `PNST` in
// general. It is not true of `*_effect.pac`, which is a two-slot `PNST` whose
// first slot is an ASCII manifest and whose second slot holds exactly as many
// children as the manifest has lines.
//
// Each line is a kind letter and a decimal identifier. The letter is not a
// guess: it predicts the child's extent, and it does so across two independent
// files with different line counts.
//
//   st001_effect.pac   9 lines,  9 slots
//   st114_effect.pac  11 lines, 11 slots
//
// | kind | records | extent                        |
// |------|---------|-------------------------------|
// | V    |       4 | 368, in both files            |
// | E    |       9 | 544, in both files            |
// | P    |       2 | 704                           |
// | A    |       2 | 336, in both files            |
// | T    |       3 | variable — a texture          |
//
// This is corpus evidence, not instruction evidence: no read site for the
// manifest has been located in the image. What raises it above a coincidence
// is that four of the five kinds have one extent shared by every record of
// that kind in two files authored separately, and that the line count matches
// the slot count exactly in both.
struct EffectPackContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    // The outer container: a PNST of exactly two slots.
    static constexpr std::size_t outer_slot_count = 2U;
    static constexpr std::size_t manifest_slot_index = 0U;
    static constexpr std::size_t records_slot_index = 1U;

    // The manifest is CRLF-terminated ASCII, NUL-padded to its slot, and its
    // comment lines begin with '#'. The corpus ends every one with "# End".
    static constexpr char comment_prefix = '#';
    static constexpr std::string_view terminator_line = "# End";
    static constexpr char field_separator = ' ';

    struct KindExtent final {
        char kind{};
        std::size_t extent{};   // zero where the kind's extent varies
        std::size_t observed{}; // records of this kind in the corpus
    };

    static constexpr std::array<KindExtent, 5> kinds{
        KindExtent{.kind = 'V', .extent = 368U, .observed = 4U},
        KindExtent{.kind = 'E', .extent = 544U, .observed = 9U},
        KindExtent{.kind = 'P', .extent = 704U, .observed = 2U},
        KindExtent{.kind = 'T', .extent = 0U, .observed = 3U},
        KindExtent{.kind = 'A', .extent = 336U, .observed = 2U},
    };

    // A `T` record is a texture, and it is the same texture descriptor this
    // project already reads elsewhere: its packed dimensions sit at +0x10 as
    // (height << 16) | width, which is exactly
    // `kDescriptorDimensionsOffset` in the texture slot framing. The two
    // corpus records read 128x128 and 256x256.
    static constexpr std::size_t texture_dimensions_offset = 0x10U;
    static constexpr std::uint32_t texture_observed_square_small = 128U;
    static constexpr std::uint32_t texture_observed_square_large = 256U;

    // No instruction in the image has been shown to read the manifest.
    static constexpr bool manifest_read_site_found = false;

    [[nodiscard]] static constexpr std::size_t extent_for(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return entry.extent;
            }
        }
        return 0U;
    }

    [[nodiscard]] static constexpr bool is_known_kind(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dmc::rengine::profiles::dmc3

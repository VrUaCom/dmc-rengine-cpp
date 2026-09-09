#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// The DMC3 motion payload.
//
// **Status is deliberately mixed, and the two halves must not be confused.**
//
// That a `.mot` file is a motion at all is *recovered*: the animation registry
// at `AnimationTypeContract::register_and_classify_va` types it by extension.
// The layout below is *structural* — arithmetic that closes on the one real
// payload in the supplied corpus, `st001.pac` slot 7. No routine that reads
// this layout has been found, and the `MOT` tag the file carries is compared
// nowhere in the image, so the runtime reaches it by name and never looks.
//
// What makes the layout worth trusting anyway is that the files check
// themselves. Each track declares a size that its key count has to account
// for, and the chain of those sizes has to stay inside the document. One track
// agreeing could be coincidence; 5,118 agreeing across 82 payloads is a
// structure.
//
// **Corrected 2026-09-08 against a complete `em000` extraction.** The single
// payload this layout was first recovered from carried only kind-3 tracks, and
// `32 + 8 x key_count` was written down as *the* track identity. It is one
// kind's identity. The full corpus holds two:
//
//   * kind 3 — a 32-byte header and 8-byte keys, 4,962 tracks;
//   * kind 2 — a 16-byte header and 4-byte keys, 156 tracks.
//
// Neither is a guess: every one of the 5,118 tracks satisfies its own kind's
// arithmetic exactly, with zero exceptions. Applying kind 3's formula to every
// track is what made this reader refuse 72 of 82 real motions — the ten it
// accepted were simply the ten that happen to contain no kind-2 track.
//
// The timeline closes too, from two directions: the header carries `650.0`
// twice, and every track's key stamps run from `-32768` to `-32118` — a span
// of exactly 650.
struct MotContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::string_view magic = "MOT";
    static constexpr std::size_t magic_offset = 0x04U;
    static constexpr std::size_t magic_bytes = 3U;

    // Header.
    static constexpr std::size_t data_offset_field = 0x00U;
    static constexpr std::size_t duration_field = 0x0CU;
    static constexpr std::size_t duration_mirror_field = 0x14U;
    static constexpr std::size_t header_table_offset = 0x18U;
    static constexpr std::size_t observed_data_offset = 0x50U;

    // Track chain, at the data offset.
    static constexpr std::size_t track_count_bytes = 4U;
    static constexpr std::size_t track_size_offset = 0x00U;
    static constexpr std::size_t track_key_count_offset = 0x02U;
    static constexpr std::size_t track_kind_offset = 0x04U;
    static constexpr std::size_t track_header_bytes = 0x20U;
    static constexpr std::size_t track_float_offset = 0x08U;
    static constexpr std::size_t track_float_count = 6U;

    // The two track kinds observed. Recorded as observed, not as the only
    // kinds that can exist — which is exactly the caveat that turned out to
    // matter when the second one appeared.
    static constexpr std::uint32_t observed_track_kind = 3U;
    static constexpr std::uint32_t compact_track_kind = 2U;

    // Kind 2's geometry, from the em000 corpus.
    static constexpr std::size_t compact_track_header_bytes = 0x10U;
    static constexpr std::size_t compact_key_bytes = 4U;

    /** True for a track kind whose geometry this reader knows. */
    [[nodiscard]] static constexpr bool track_kind_is_known(
        std::uint32_t kind) noexcept {
        return kind == observed_track_kind || kind == compact_track_kind;
    }

    [[nodiscard]] static constexpr std::size_t header_bytes_for_kind(
        std::uint32_t kind) noexcept {
        return kind == compact_track_kind ? compact_track_header_bytes
                                          : track_header_bytes;
    }

    [[nodiscard]] static constexpr std::size_t key_bytes_for_kind(
        std::uint32_t kind) noexcept {
        return kind == compact_track_kind ? compact_key_bytes : key_bytes;
    }

    // Key: the leading 16-bit value is a *flagged* stamp — the low 15 bits are
    // the timeline position, the top bit is a flag. The rest is unnamed here,
    // because nothing in the image says what it is.
    //
    // This was previously read as a signed int16, which is why the first stamp
    // was recorded as -32768: that is 0x8000, the flag set over a stamp of
    // zero. Read as 15 bits, every one of the 5,118 tracks in the em000 corpus
    // is strictly increasing and 4,915 of them span exactly the duration the
    // header declares twice. Read as int16, three payloads look corrupt.
    static constexpr std::size_t key_bytes = 8U;
    static constexpr std::size_t key_components = 4U;
    static constexpr std::size_t key_stamp_component = 0U;
    static constexpr std::uint16_t key_stamp_mask = 0x7FFFU;
    static constexpr std::uint16_t key_flag_mask = 0x8000U;
    static constexpr std::uint16_t observed_first_stamp = 0U;

    /** The timeline position a key's leading value carries. */
    [[nodiscard]] static constexpr std::uint16_t key_stamp(
        std::uint16_t raw) noexcept {
        return static_cast<std::uint16_t>(raw & key_stamp_mask);
    }

    /** The flag that rides on it. Observed set on all but 465 of 94,875 keys. */
    [[nodiscard]] static constexpr bool key_flag(std::uint16_t raw) noexcept {
        return (raw & key_flag_mask) != 0U;
    }

    // The smallest track header any known kind uses. Bounds must be checked
    // against this before the kind is known, and against the kind's own header
    // afterwards — checking the larger one first refuses a short compact track
    // sitting legitimately at the end of a payload.
    static constexpr std::size_t minimum_track_header_bytes =
        compact_track_header_bytes;

    // The chain ends on a zero dword rather than at the last track's end.
    static constexpr std::size_t terminator_bytes = 4U;

    // The identity every track must satisfy, per kind. This is the whole
    // basis for reading the format, so it is stated once and checked
    // everywhere — and it takes the kind, because the arithmetic is the
    // kind's and not the format's.
    [[nodiscard]] static constexpr std::size_t track_bytes(
        std::size_t key_count,
        std::uint32_t kind = observed_track_kind) noexcept {
        return header_bytes_for_kind(kind) + key_count * key_bytes_for_kind(kind);
    }

    // Every kind-3 track in the corpus opens at -32768; every kind-2 track
    // opens at 0. Recorded, never enforced: a first stamp is an observation
    // about these payloads, not a rule the format states.
    static constexpr std::int16_t observed_compact_first_stamp = 0;

    // Documents are padded to this boundary, so the track chain closes to
    // within one unit of the end rather than exactly on it. All 82 payloads in
    // the em000 corpus are an exact multiple of it and every tail byte after
    // the last track is zero, which is what lets a stray trailing byte still
    // be refused now that the old four-byte "terminator" is understood as
    // padding.
    static constexpr std::size_t document_alignment = 0x10U;
};

} // namespace dmc::rengine::profiles::dmc3

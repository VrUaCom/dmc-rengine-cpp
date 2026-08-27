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
// What makes the layout worth trusting anyway is that the file checks itself
// 69 times over. Each track declares a size that must equal
// `32 + 8 x key_count`, and the chain of those sizes has to land exactly on
// the terminator. One track agreeing could be coincidence; sixty-nine
// agreeing, with the chain closing to the byte, is a structure.
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

    // The only track kind in the corpus. Recorded as observed, not as the
    // only kind that can exist.
    static constexpr std::uint32_t observed_track_kind = 3U;

    // Key: four signed 16-bit values. The first is a stamp and is strictly
    // increasing within a track; the other three are unnamed here, because
    // nothing in the image says what they are.
    static constexpr std::size_t key_bytes = 8U;
    static constexpr std::size_t key_components = 4U;
    static constexpr std::size_t key_stamp_component = 0U;
    static constexpr std::int16_t observed_first_stamp = -32768;

    // The chain ends on a zero dword rather than at the last track's end.
    static constexpr std::size_t terminator_bytes = 4U;

    // The identity every track must satisfy. This is the whole basis for
    // reading the format, so it is stated once and checked everywhere.
    [[nodiscard]] static constexpr std::size_t track_bytes(
        std::size_t key_count) noexcept {
        return track_header_bytes + key_count * key_bytes;
    }
};

} // namespace dmc::rengine::profiles::dmc3

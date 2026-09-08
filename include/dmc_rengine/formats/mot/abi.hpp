#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::formats::mot {

struct HeaderAbi final {
    static constexpr std::size_t header_size_field = 0x00U;
    static constexpr std::size_t magic_field = 0x04U;
    static constexpr std::size_t raw_id_field = 0x04U; // overlaps MOT\0 bytes as signed/raw id in legacy tooling
    static constexpr std::size_t raw_start_frame_field = 0x08U;
    static constexpr std::size_t raw_end_frame_field = 0x0CU;
    static constexpr std::size_t raw_start_frame_2_field = 0x10U;
    static constexpr std::size_t raw_end_frame_2_field = 0x14U;
    static constexpr std::size_t raw_flag_field = 0x18U;
    static constexpr std::size_t raw_selector_field = 0x1AU;
    static constexpr std::size_t channel_domain_count_field = 0x1CU;
    static constexpr std::size_t channel_mask_table = 0x1EU;
    static constexpr std::size_t alignment = 0x10U;
};

struct TrackAbi final {
    static constexpr std::size_t span_field = 0x00U;
    static constexpr std::size_t key_count_field = 0x02U;
    static constexpr std::size_t compression_field = 0x04U;
    static constexpr std::size_t start_time_field = 0x06U;
    static constexpr std::size_t quantization_field = 0x08U;

    static constexpr std::uint16_t compression_linear_int16 = 2U;
    static constexpr std::uint16_t compression_hermite_int16 = 3U;

    static constexpr std::size_t linear_int16_prefix_size = 0x10U;
    static constexpr std::size_t linear_int16_key_size = 0x04U;
    static constexpr std::size_t hermite_int16_prefix_size = 0x20U;
    static constexpr std::size_t hermite_int16_key_size = 0x08U;
};

struct ChannelMaskAbi final {
    // The nine bits are structurally confirmed as three 3-bit groups by em000.
    // High-level translation/rotation/scale names are not made ABI authority
    // until canonical dmc3.exe evaluation code is directly rebound.
    static constexpr std::uint16_t low_triplet = 0x007U;
    static constexpr std::uint16_t middle_triplet = 0x038U;
    static constexpr std::uint16_t high_triplet = 0x1C0U;
    static constexpr std::uint16_t all_known_bits = 0x1FFU;
};

[[nodiscard]] constexpr std::size_t align16(std::size_t value) noexcept {
    return (value + 0x0FU) & ~std::size_t{0x0FU};
}

[[nodiscard]] constexpr std::size_t header_size_for_channel_domain(
    std::size_t channel_domain_count) noexcept {
    return align16(HeaderAbi::channel_mask_table + channel_domain_count * sizeof(std::uint16_t));
}

[[nodiscard]] constexpr std::size_t expected_track_span(
    std::uint16_t compression,
    std::uint16_t key_count) noexcept {
    if (compression == TrackAbi::compression_linear_int16) {
        return TrackAbi::linear_int16_prefix_size +
            static_cast<std::size_t>(key_count) * TrackAbi::linear_int16_key_size;
    }
    if (compression == TrackAbi::compression_hermite_int16) {
        return TrackAbi::hermite_int16_prefix_size +
            static_cast<std::size_t>(key_count) * TrackAbi::hermite_int16_key_size;
    }
    return 0U;
}

[[nodiscard]] constexpr std::uint16_t key_time_index(std::uint16_t raw) noexcept {
    return raw & 0x7FFFU;
}

[[nodiscard]] constexpr bool key_high_flag(std::uint16_t raw) noexcept {
    return (raw & 0x8000U) != 0U;
}

[[nodiscard]] constexpr std::size_t selected_track_count(std::uint16_t mask) noexcept {
    return static_cast<std::size_t>(std::popcount(static_cast<unsigned int>(mask)));
}

static_assert(header_size_for_channel_domain(22U) == 0x50U);
static_assert(header_size_for_channel_domain(4U) == 0x30U);
static_assert(header_size_for_channel_domain(3U) == 0x30U);
static_assert(expected_track_span(2U, 1U) == 0x14U);
static_assert(expected_track_span(2U, 11U) == 0x3CU);
static_assert(expected_track_span(3U, 1U) == 0x28U);
static_assert(expected_track_span(3U, 3U) == 0x38U);

} // namespace dmc::rengine::formats::mot

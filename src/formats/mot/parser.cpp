#include "dmc_rengine/formats/mot/parser.hpp"

#include "dmc_rengine/formats/mot/abi.hpp"

#include <bit>
#include <utility>

namespace dmc::rengine::formats::mot {
namespace {

[[nodiscard]] ParseResult fail(ParseError error, std::string message) {
    return ParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] std::uint16_t read_u16(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset + 0U]) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::uint32_t read_u32(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] float read_f32(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::bit_cast<float>(read_u32(bytes, offset));
}

[[nodiscard]] bool magic_ok(std::span<const std::byte> bytes) noexcept {
    return bytes.size() >= 8U &&
        bytes[4U] == std::byte{'M'} &&
        bytes[5U] == std::byte{'O'} &&
        bytes[6U] == std::byte{'T'} &&
        bytes[7U] == std::byte{0};
}

[[nodiscard]] bool monotonic_key_times(const TrackRecord& track) noexcept {
    std::uint16_t previous = 0U;
    bool first = true;
    if (track.compression == TrackAbi::compression_2) {
        for (const auto& key : track.keys2) {
            const auto current = key_time_index(key.time_control);
            if (!first && current < previous) {
                return false;
            }
            previous = current;
            first = false;
        }
    } else if (track.compression == TrackAbi::compression_3) {
        for (const auto& key : track.keys3) {
            const auto current = key_time_index(key.time_control);
            if (!first && current < previous) {
                return false;
            }
            previous = current;
            first = false;
        }
    }
    return true;
}

} // namespace

bool Document::valid() const noexcept {
    if (physical_size < HeaderAbi::channel_mask_table ||
        channel_masks.size() != static_cast<std::size_t>(channel_domain_count) ||
        tracks.size() != static_cast<std::size_t>(record_count)) {
        return false;
    }
    if (header_size != header_size_for_channel_domain(channel_domain_count)) {
        return false;
    }
    std::size_t selected = 0U;
    for (const auto mask : channel_masks) {
        selected += selected_track_count(mask);
    }
    return selected == static_cast<std::size_t>(record_count);
}

ParseResult Parser::parse(std::span<const std::byte> bytes) {
    if (bytes.size() < HeaderAbi::channel_mask_table) {
        return fail(ParseError::truncated_header, "MOT header prefix is truncated");
    }
    if (!magic_ok(bytes)) {
        return fail(ParseError::invalid_magic, "MOT\\0 marker at +0x04 is absent");
    }

    const auto header_size = read_u32(bytes, HeaderAbi::header_size_field);
    const auto channel_domain_count = read_u16(bytes, HeaderAbi::channel_domain_count_field);
    const auto expected_header_size = header_size_for_channel_domain(channel_domain_count);
    if (header_size != expected_header_size) {
        return fail(
            ParseError::invalid_header_size,
            "MOT header size does not match the aligned channel-mask table extent");
    }
    const auto header_extent = static_cast<std::size_t>(header_size);
    if (header_extent > bytes.size() || bytes.size() - header_extent < 4U) {
        return fail(ParseError::truncated_header, "MOT header/count envelope crosses EOF");
    }

    Document document{
        .physical_size = bytes.size(),
        .header_size = header_size,
        .raw_u32_08 = read_u32(bytes, HeaderAbi::raw_u32_08_field),
        .raw_f32_0c = read_f32(bytes, HeaderAbi::raw_f32_0c_field),
        .raw_f32_10 = read_f32(bytes, HeaderAbi::raw_f32_10_field),
        .raw_f32_14 = read_f32(bytes, HeaderAbi::raw_f32_14_field),
        .raw_u16_18 = read_u16(bytes, HeaderAbi::raw_u16_18_field),
        .raw_u16_1a = read_u16(bytes, HeaderAbi::raw_u16_1a_field),
        .channel_domain_count = channel_domain_count,
    };

    const auto mask_count = static_cast<std::size_t>(channel_domain_count);
    document.channel_masks.reserve(mask_count);
    std::size_t selected_tracks = 0U;
    for (std::size_t index = 0U; index < mask_count; ++index) {
        const auto mask = read_u16(
            bytes,
            HeaderAbi::channel_mask_table + index * sizeof(std::uint16_t));
        if ((mask & static_cast<std::uint16_t>(~ChannelMaskAbi::all_known_bits)) != 0U) {
            return fail(
                ParseError::unknown_channel_mask_bits,
                "MOT channel mask uses bits outside the em000 nine-bit domain");
        }
        document.channel_masks.push_back(mask);
        selected_tracks += selected_track_count(mask);
    }

    const auto mask_table_end = HeaderAbi::channel_mask_table +
        mask_count * sizeof(std::uint16_t);
    document.raw_header_tail.assign(
        bytes.begin() + static_cast<std::ptrdiff_t>(mask_table_end),
        bytes.begin() + static_cast<std::ptrdiff_t>(header_extent));

    const auto record_count = read_u32(bytes, header_extent);
    if (record_count > k_safety_max_records) {
        return fail(
            ParseError::unreasonable_record_count,
            "MOT record count exceeds the product-side safety bound");
    }
    if (selected_tracks != static_cast<std::size_t>(record_count)) {
        return fail(
            ParseError::record_count_mask_mismatch,
            "MOT record count differs from channel-mask popcount sum");
    }
    document.record_count = record_count;
    document.tracks.reserve(static_cast<std::size_t>(record_count));

    std::size_t cursor = header_extent + 4U;
    for (std::uint32_t index = 0U; index < record_count; ++index) {
        static_cast<void>(index);
        if (cursor > bytes.size() || bytes.size() - cursor < 8U) {
            return fail(ParseError::truncated_track, "MOT track prefix crosses EOF");
        }

        TrackRecord track{
            .offset = cursor,
            .span = read_u16(bytes, cursor + TrackAbi::span_field),
            .key_count = read_u16(bytes, cursor + TrackAbi::key_count_field),
            .compression = read_u16(bytes, cursor + TrackAbi::compression_field),
            .start_time_raw = read_u16(bytes, cursor + TrackAbi::start_time_field),
        };
        const auto extent = static_cast<std::size_t>(track.span);
        if (extent < 8U) {
            return fail(ParseError::invalid_track_span, "MOT track span is smaller than its common prefix");
        }
        if (extent > bytes.size() - cursor) {
            return fail(ParseError::truncated_track, "MOT track span crosses EOF");
        }

        const auto expected = expected_track_span(track.compression, track.key_count);
        if (expected != 0U && expected != extent) {
            return fail(
                ParseError::known_compression_span_mismatch,
                "MOT compression-2/3 track span does not match its key count");
        }

        track.source_bytes.assign(
            bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
            bytes.begin() + static_cast<std::ptrdiff_t>(cursor + extent));

        if (track.compression == TrackAbi::compression_2) {
            track.quantization_float_count = 2U;
            track.quantization_raw[0U] = read_f32(bytes, cursor + 0x08U);
            track.quantization_raw[1U] = read_f32(bytes, cursor + 0x0CU);
            track.keys2.reserve(track.key_count);
            std::size_t key_cursor = cursor + TrackAbi::compression_2_prefix_size;
            for (std::uint16_t key_index = 0U; key_index < track.key_count; ++key_index) {
                static_cast<void>(key_index);
                track.keys2.push_back(QuantizedKey2{
                    .time_control = read_u16(bytes, key_cursor + 0U),
                    .value = read_u16(bytes, key_cursor + 2U),
                });
                key_cursor += TrackAbi::compression_2_key_size;
            }
        } else if (track.compression == TrackAbi::compression_3) {
            track.quantization_float_count = 6U;
            for (std::size_t float_index = 0U; float_index < 6U; ++float_index) {
                track.quantization_raw[float_index] = read_f32(
                    bytes,
                    cursor + TrackAbi::quantization_field + float_index * sizeof(float));
            }
            track.keys3.reserve(track.key_count);
            std::size_t key_cursor = cursor + TrackAbi::compression_3_prefix_size;
            for (std::uint16_t key_index = 0U; key_index < track.key_count; ++key_index) {
                static_cast<void>(key_index);
                track.keys3.push_back(QuantizedKey3{
                    .time_control = read_u16(bytes, key_cursor + 0U),
                    .value = read_u16(bytes, key_cursor + 2U),
                    .auxiliary_a = read_u16(bytes, key_cursor + 4U),
                    .auxiliary_b = read_u16(bytes, key_cursor + 6U),
                });
                key_cursor += TrackAbi::compression_3_key_size;
            }
        }

        if (!monotonic_key_times(track)) {
            return fail(
                ParseError::non_monotonic_key_time,
                "MOT known-compression key times are not monotonic after masking bit15");
        }

        document.tracks.push_back(std::move(track));
        cursor += extent;
    }

    for (std::size_t index = cursor; index < bytes.size(); ++index) {
        if (bytes[index] != std::byte{0}) {
            return fail(ParseError::nonzero_tail, "MOT post-track tail is not zero padding");
        }
    }
    document.zero_padding_size = bytes.size() - cursor;

    return ParseResult{
        .document = std::move(document),
        .error = ParseError::none,
        .message = {},
    };
}

bool Parser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats::mot

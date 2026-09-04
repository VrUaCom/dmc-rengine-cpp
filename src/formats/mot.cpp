#include "dmc_rengine/formats/mot.hpp"

#include "dmc_rengine/profiles/dmc3/mot_contract.hpp"

#include <bit>
#include <cmath>
#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::MotContract;

[[nodiscard]] bool fits(
    std::uint64_t offset,
    std::uint64_t size,
    std::uint64_t total) noexcept {
    return offset <= total && size <= total - offset;
}

[[nodiscard]] std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset]) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::int16_t read_i16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::int16_t>(read_u16_le(bytes, offset));
}

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint32_t value = 0U;
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= std::to_integer<std::uint32_t>(bytes[offset + index])
            << (8U * index);
    }
    return value;
}

[[nodiscard]] MotParseResult fail(MotParseError error, std::string message) {
    return MotParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

bool MotDocument::valid() const noexcept {
    if (document_size == 0U || tracks.size() != track_count) {
        return false;
    }
    for (const auto& track : tracks) {
        if (track.key_count == 0U ||
            track.key_offset + static_cast<std::uint64_t>(track.key_count) *
                Contract::key_bytes > document_size) {
            return false;
        }
    }
    return true;
}

MotParseResult MotParser::parse(std::span<const std::byte> bytes) {
    const auto total = static_cast<std::uint64_t>(bytes.size());
    if (total < Contract::observed_data_offset + Contract::track_count_bytes) {
        return fail(
            MotParseError::truncated_header,
            "motion payload is smaller than its own header");
    }
    for (std::size_t index = 0U; index < Contract::magic_bytes; ++index) {
        if (std::to_integer<char>(bytes[Contract::magic_offset + index]) !=
            Contract::magic[index]) {
            return fail(
                MotParseError::invalid_magic,
                "motion payload does not carry the MOT tag at its fixed offset");
        }
    }

    MotDocument document;
    document.document_size = total;
    document.data_offset = read_u32_le(bytes, Contract::data_offset_field);
    document.duration = std::bit_cast<float>(
        read_u32_le(bytes, Contract::duration_field));

    if (!fits(document.data_offset, Contract::track_count_bytes, total)) {
        return fail(
            MotParseError::data_offset_out_of_bounds,
            "the declared data offset lies outside the motion payload");
    }

    document.track_count = read_u32_le(
        bytes, static_cast<std::size_t>(document.data_offset));
    if (document.track_count == 0U || document.track_count > k_max_track_count) {
        return fail(
            MotParseError::track_count_limit,
            "declared track count is zero or above the product parser limit");
    }

    auto cursor = document.data_offset + Contract::track_count_bytes;
    document.tracks.reserve(document.track_count);
    bool spans_match = true;

    for (std::uint32_t index = 0U; index < document.track_count; ++index) {
        if (!fits(cursor, Contract::track_header_bytes, total)) {
            return fail(
                MotParseError::truncated_track,
                "a track header extends past the end of the motion payload");
        }
        const auto at = static_cast<std::size_t>(cursor);
        const auto size = read_u16_le(bytes, at + Contract::track_size_offset);
        const auto keys = read_u16_le(bytes, at + Contract::track_key_count_offset);

        // The identity the whole format rests on. A track that declares a size
        // its key count cannot account for is not a track this reader knows,
        // and reading it anyway would be reading something else.
        if (size != Contract::track_bytes(keys)) {
            return fail(
                MotParseError::track_size_mismatch,
                "a track's declared size does not equal its key count's");
        }
        if (keys == 0U || !fits(cursor, size, total)) {
            return fail(
                MotParseError::truncated_track,
                "a track extends past the end of the motion payload");
        }

        MotTrack track{
            .track_index = index,
            .track_offset = cursor,
            .key_count = keys,
            .kind = read_u32_le(bytes, at + Contract::track_kind_offset),
            .key_offset = cursor + Contract::track_header_bytes,
            .first_stamp = 0,
            .last_stamp = 0,
        };

        auto previous = std::int32_t{0};
        for (std::uint32_t key = 0U; key < keys; ++key) {
            const auto stamp = read_i16_le(
                bytes,
                static_cast<std::size_t>(
                    track.key_offset + static_cast<std::uint64_t>(key) *
                        Contract::key_bytes));
            if (key == 0U) {
                track.first_stamp = stamp;
            } else if (stamp <= previous) {
                // A stamp that does not advance means the run is not a
                // timeline, which is the one thing these four values are known
                // to be about.
                return fail(
                    MotParseError::stamp_not_increasing,
                    "a track's key stamps do not strictly increase");
            }
            previous = stamp;
            track.last_stamp = stamp;
        }

        spans_match = spans_match &&
            static_cast<std::int32_t>(document.duration) == track.span();
        document.total_key_count += keys;
        document.tracks.push_back(track);
        cursor += size;
    }

    // The chain must land exactly on the terminator. Landing early or late
    // means the sizes describe a different file than the one supplied.
    if (cursor + Contract::terminator_bytes != total) {
        return fail(
            MotParseError::chain_does_not_close,
            "the track chain does not end exactly at the payload terminator");
    }
    document.duration_matches_stamps = spans_match;

    if (!document.valid()) {
        return fail(
            MotParseError::invalid_document,
            "motion payload decoded to an inconsistent document");
    }

    return MotParseResult{
        .document = std::move(document),
        .error = MotParseError::none,
        .message = {},
    };
}

bool MotParser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats

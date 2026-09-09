#include "dmc_rengine/formats/mot.hpp"

#include <cmath>
#include <utility>

namespace dmc::rengine::formats {
namespace {
MotParseResult fail(MotParseError error, std::string message) {
    return {std::nullopt, error, std::move(message)};
}
MotParseError legacy_error(mot::ParseError error) {
    switch (error) {
    case mot::ParseError::truncated_header: return MotParseError::truncated_header;
    case mot::ParseError::invalid_magic: return MotParseError::invalid_magic;
    case mot::ParseError::invalid_header_size: return MotParseError::data_offset_out_of_bounds;
    case mot::ParseError::unreasonable_record_count: return MotParseError::track_count_limit;
    case mot::ParseError::truncated_track: return MotParseError::truncated_track;
    case mot::ParseError::invalid_track_span:
    case mot::ParseError::known_compression_span_mismatch: return MotParseError::track_size_mismatch;
    case mot::ParseError::non_monotonic_key_time: return MotParseError::stamp_not_increasing;
    case mot::ParseError::record_count_mask_mismatch:
    case mot::ParseError::nonzero_tail: return MotParseError::chain_does_not_close;
    default: return MotParseError::invalid_document;
    }
}
}

bool MotDocument::valid() const noexcept {
    if (document_size == 0U || tracks.size() != track_count) return false;
    for (const auto& track : tracks) {
        const auto stride = track.kind == mot::TrackAbi::compression_2 ? 4U : 8U;
        if (track.key_count == 0U || track.key_offset > document_size ||
            track.key_count > (document_size - track.key_offset) / stride) return false;
    }
    return true;
}

MotParseResult MotParser::parse(std::span<const std::byte> bytes) {
    const auto parsed = mot::Parser::parse(bytes);
    if (!parsed.ok()) return fail(legacy_error(parsed.error), parsed.message);
    const auto& source = *parsed.document;
    if (source.record_count == 0U || source.record_count > k_max_track_count)
        return fail(MotParseError::track_count_limit, "legacy MOT summary record limit exceeded");
    if (bytes.size() % 16U != 0U || source.zero_padding_size >= 16U)
        return fail(MotParseError::chain_does_not_close, "MOT summary requires bounded alignment padding");

    MotDocument result;
    result.document_size = source.physical_size;
    result.data_offset = source.header_size;
    result.duration = source.raw_f32_0c; // Historical summary name, not a new semantic claim.
    result.track_count = source.record_count;
    result.duration_matches_stamps = true;
    for (const auto& record : source.tracks) {
        if (record.key_count == 0U || (record.compression != 2U && record.compression != 3U))
            return fail(MotParseError::track_size_mismatch, "MOT summary requires decoded compression 2/3 keys");
        MotTrack track;
        track.track_index = static_cast<std::uint32_t>(result.tracks.size());
        track.track_offset = record.offset;
        track.key_count = record.key_count;
        track.kind = record.compression; // +0x06 start_time_raw is a separate u16.
        track.key_offset = record.offset + (record.compression == 2U ? 0x10U : 0x20U);
        for (std::size_t i = 0; i < record.key_count; ++i) {
            const auto raw = record.compression == 2U ? record.keys2[i].time_control : record.keys3[i].time_control;
            const auto stamp = mot::key_time_index(raw);
            if (i == 0U) track.first_stamp = stamp;
            else if (stamp <= track.last_stamp)
                return fail(MotParseError::stamp_not_increasing, "legacy summary requires strictly increasing key times");
            track.last_stamp = stamp;
            track.flagged_key_count += mot::key_high_flag(raw) ? 1U : 0U;
        }
        result.duration_matches_stamps = result.duration_matches_stamps &&
            std::isfinite(result.duration) && result.duration == static_cast<float>(track.span());
        result.total_key_count += record.key_count;
        result.tracks.push_back(track);
    }
    return {std::move(result), MotParseError::none, {}};
}

bool MotParser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    try { return parse(bytes).ok(); } catch (...) { return false; }
}
} // namespace dmc::rengine::formats

#pragma once

#include "dmc_rengine/analysis/mot/key_decode.hpp"

#include <cmath>
#include <cstdint>
#include <optional>

namespace dmc::rengine::analysis::mot {

struct EvaluatedCompression3Sample final {
    float value{};
    std::int32_t cached_index{};
    SegmentSelectionKind selection_kind{SegmentSelectionKind::single_key};
    std::size_t left_index{};
    std::size_t right_index{};
};

[[nodiscard]] constexpr std::int32_t signed_track_time_offset(
    std::uint16_t raw) noexcept {
    return raw < 0x8000U
        ? static_cast<std::int32_t>(raw)
        : static_cast<std::int32_t>(raw) - 0x10000;
}

// Bounded composition of the recovered compression-3 scalar evaluator:
//
//   global evaluation time
//     -> subtract signed track+0x06 offset
//     -> cached segment search at 0x1402E8C80..0x1402E8E10
//     -> key decode at 0x1402E9650
//     -> linear/Hermite segment evaluation at 0x1402E9170/0x1402E9880
//
// This closes semantic scalar evaluation for the supported compression-3 path.
// It does not claim bit-identical SSE operation order, looping, blending,
// invalid-cache repair or alternate header-flag paths.
[[nodiscard]] inline std::optional<EvaluatedCompression3Sample>
evaluate_compression3_track(
    const formats::mot::TrackRecord& track,
    float evaluation_time,
    std::int32_t cached_index) noexcept {
    if (!std::isfinite(evaluation_time)) return std::nullopt;

    const auto offset = signed_track_time_offset(track.start_time_raw);
    const auto local_time =
        evaluation_time - static_cast<float>(offset);
    const auto selection =
        select_cached_segment3(track, local_time, cached_index);
    if (!selection.has_value()) return std::nullopt;

    const auto left = decode_key(track, selection->left_index);
    if (!left.has_value()) return std::nullopt;

    if (!selection->is_segment()) {
        return EvaluatedCompression3Sample{
            left->value,
            selection->cached_index,
            selection->kind,
            selection->left_index,
            selection->right_index};
    }

    const auto right = decode_key(track, selection->right_index);
    if (!right.has_value()) return std::nullopt;

    const auto value = evaluate_segment3(*left, *right, evaluation_time);
    if (!value.has_value()) return std::nullopt;

    return EvaluatedCompression3Sample{
        *value,
        selection->cached_index,
        selection->kind,
        selection->left_index,
        selection->right_index};
}

} // namespace dmc::rengine::analysis::mot

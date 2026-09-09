#pragma once

#include "dmc_rengine/formats/mot/ir.hpp"

#include <array>
#include <cmath>
#include <optional>

namespace dmc::rengine::analysis::mot {

// Canonical EXE evidence: docs/research/dmc3-mot-key-evaluation-2026-09-09.md.
// Normal binding path only; this is not a complete animation player.
inline constexpr std::array<std::uint16_t, 9> binding_bit_order{
    0x40, 0x80, 0x100, 0x08, 0x10, 0x20, 0x01, 0x02, 0x04};

struct DecodedKey {
    std::int32_t time{};
    bool cubic_segment{};
    float value{};
    float incoming_slope{};
    float outgoing_slope{};
};

[[nodiscard]] inline std::optional<DecodedKey> decode_key(
    const formats::mot::TrackRecord& track, std::size_t index) noexcept {
    if (index >= track.key_count) return std::nullopt;
    std::uint16_t control{}, value{}, incoming{}, outgoing{};
    if (track.compression == 2 && track.quantization_float_count == 2 &&
        index < track.keys2.size()) {
        control = track.keys2[index].time_control;
        value = track.keys2[index].value;
    } else if (track.compression == 3 && track.quantization_float_count == 6 &&
               index < track.keys3.size()) {
        const auto& key = track.keys3[index];
        control = key.time_control;
        value = key.value;
        incoming = key.auxiliary_a;
        outgoing = key.auxiliary_b;
    } else {
        return std::nullopt;
    }
    const auto offset = track.start_time_raw < 0x8000U
        ? static_cast<std::int32_t>(track.start_time_raw)
        : static_cast<std::int32_t>(track.start_time_raw) - 0x10000;
    const auto& q = track.quantization_raw;
    const auto unpack = [&q](std::uint16_t raw, std::size_t pair) {
        return static_cast<float>(raw) * q[pair + 1] / 65535.0F + q[pair];
    };
    return DecodedKey{
        static_cast<std::int32_t>(control & 0x7FFFU) + offset,
        (control & 0x8000U) != 0, unpack(value, 0),
        track.compression == 3 ? unpack(incoming, 2) : 0.0F,
        track.compression == 3 ? unpack(outgoing, 4) : 0.0F};
}

// Caller supplies an already selected compression-3 segment. Reject unsupported
// boundary cases rather than guessing the game's segment search behaviour.
// Algebraically recovered Hermite; not a bit-identical SSE implementation.
[[nodiscard]] inline std::optional<float> evaluate_segment3(
    const DecodedKey& left, const DecodedKey& right, float time) noexcept {
    const float a = static_cast<float>(left.time);
    const float b = static_cast<float>(right.time);
    if (!std::isfinite(time) || b <= a || time < a || time > b) return std::nullopt;
    const float d = b - a;
    const float u = (time - a) / d;
    if (!left.cubic_segment) return (1.0F - u) * left.value + u * right.value;
    const float u2 = u * u;
    const float u3 = u2 * u;
    return (2.0F * u3 - 3.0F * u2 + 1.0F) * left.value
        + (3.0F * u2 - 2.0F * u3) * right.value
        + (u3 - 2.0F * u2 + u) * d * left.outgoing_slope
        + (u3 - u2) * d * right.incoming_slope;
}

} // namespace dmc::rengine::analysis::mot

#include "dmc_rengine/analysis/mot/key_decode.hpp"
#include <cassert>
#include <cmath>
#include <limits>

int main() {
    namespace mot = dmc::rengine::analysis::mot;
    dmc::rengine::formats::mot::TrackRecord track;
    track.compression = 3;
    track.key_count = 2;
    track.start_time_raw = 0xFFF6; // -10, not unsigned 65526.
    track.quantization_float_count = 6;
    track.quantization_raw = {-2, 4, -3, 6, -5, 10};
    track.keys3 = {{0x800A, 0, 65535, 0}, {20, 65535, 0, 65535}};
    const auto left = mot::decode_key(track, 0);
    const auto right = mot::decode_key(track, 1);
    assert(left && right);
    assert(left->time == 0 && right->time == 10 && left->cubic_segment);
    assert(left->value == -2 && right->value == 2);
    assert(left->incoming_slope == 3 && left->outgoing_slope == -5);
    assert(right->incoming_slope == -3 && right->outgoing_slope == 5);
    assert(!mot::decode_key(track, 2));
    assert(mot::evaluate_segment3(*left, *right, 0) == -2);
    assert(mot::evaluate_segment3(*left, *right, 10) == 2);
    // Midpoint slope contribution = d/8 * (left outgoing - right incoming).
    assert(mot::evaluate_segment3(*left, *right, 5) == -2.5F);
    auto linear = *left;
    linear.cubic_segment = false;
    assert(mot::evaluate_segment3(linear, *right, 5) == 0);
    assert(!mot::evaluate_segment3(*left, *left, 0));
    assert(!mot::evaluate_segment3(*left, *right, -1));
    assert(!mot::evaluate_segment3(*left, *right, std::numeric_limits<float>::quiet_NaN()));
    track.compression = 2;
    track.quantization_float_count = 2;
    track.keys2 = {{10, 65535}};
    const auto key2 = mot::decode_key(track, 0);
    assert(key2 && key2->value == 2 && key2->time == 0);
    assert(key2->incoming_slope == 0 && key2->outgoing_slope == 0);
    assert(!mot::decode_key(track, 1));
    track.compression = 7;
    assert(!mot::decode_key(track, 0));
    static_assert(mot::binding_bit_order[0] == 0x40);
    static_assert(mot::binding_bit_order[6] == 0x01);
}

#include "dmc_rengine/analysis/mot/animated_local.hpp"
#include "dmc_rengine/analysis/mot/track_evaluation.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

[[nodiscard]] bool near(float a, float b, float epsilon = 0.0005F) {
    return std::fabs(a - b) < epsilon;
}

} // namespace

int main() {
    namespace mot = dmc::rengine::analysis::mot;
    constexpr float pi = std::numbers::pi_v<float>;

    // 0x140310310 16-bit angle wrap.
    assert(near(mot::quantize_motion_angle(0.5F), 0.5F, 0.0002F));
    assert(near(mot::quantize_motion_angle(2.0F * pi + 0.5F), 0.5F));
    assert(near(mot::quantize_motion_angle(1.5F * pi), -0.5F * pi));
    assert(mot::quantize_motion_angle(std::numeric_limits<float>::quiet_NaN()) == 0.0F);
    assert(mot::quantize_motion_angle(1.0e12F) == 0.0F);

    mot::JointChannelValues values;
    values.translation = {1.0F, 2.0F, 3.0F};
    auto local = mot::build_animated_local_matrix(values);
    assert(local.values[0] == 1.0F && local.values[5] == 1.0F && local.values[10] == 1.0F);
    assert(local.values[12] == 1.0F && local.values[13] == 2.0F && local.values[14] == 3.0F);
    assert(local.values[15] == 1.0F);

    // A quarter turn about Z maps the X basis row onto +Y (row-vector form).
    values.rotation = {0.0F, 0.0F, 0.5F * pi};
    local = mot::build_animated_local_matrix(values);
    assert(near(local.values[0], 0.0F) && near(local.values[1], 1.0F));

    assert(!mot::has_non_unit_scale(values));
    values.scale = {2.0F, 1.0F, 1.0F};
    assert(mot::has_non_unit_scale(values));

    dmc::rengine::formats::mod::transform_domain::LocalTransformRecord record{};
    record.translation = {4.0F, 5.0F, 6.0F};
    record.rotation_xyz_radians = {0.1F, 0.2F, 0.3F};
    const auto rest = mot::rest_channel_values(record);
    assert(rest.translation[2] == 6.0F && rest.rotation[1] == 0.2F);
    assert(rest.scale[0] == 1.0F && rest.scale[2] == 1.0F);

    // Compression 2: keys (0 -> q0) and (10 -> q0 + q1), linear only.
    dmc::rengine::formats::mot::TrackRecord track;
    track.compression = 2;
    track.key_count = 2;
    track.start_time_raw = 0;
    track.quantization_float_count = 2;
    track.quantization_raw = {10.0F, 10.0F, 0.0F, 0.0F, 0.0F, 0.0F};
    track.keys2 = {{0x8000U, 0U}, {10U, 0xFFFFU}};  // flag bit ignored.
    auto sample = mot::evaluate_compression2_track(track, 5.0F, 0);
    assert(sample && near(sample->value, 15.0F));
    assert(sample->cached_index == 0);
    sample = mot::evaluate_compression2_track(track, 50.0F, sample->cached_index);
    assert(sample && near(sample->value, 20.0F) && sample->cached_index == 1);
    sample = mot::evaluate_compression2_track(track, 2.5F, sample->cached_index);
    assert(sample && near(sample->value, 12.5F));

    track.start_time_raw = 0xFFF6U;  // -10: the timeline shifts left.
    sample = mot::evaluate_compression2_track(track, -5.0F, 0);
    assert(sample && near(sample->value, 15.0F));

    track.compression = 3;
    assert(!mot::evaluate_compression2_track(track, 1.0F, 0));
    return 0;
}

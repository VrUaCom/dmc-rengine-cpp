#pragma once

#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace dmc::rengine::analysis::mot {

// Current values of the nine CMotionJoint channels (+0x120 .. +0x220, stride
// 0x20) in the normal channel order: translation xyz, rotation xyz, scale xyz.
struct JointChannelValues final {
    std::array<float, 3> translation{};
    std::array<float, 3> rotation{};
    std::array<float, 3> scale{1.0F, 1.0F, 1.0F};
};

// .rdata constants used by 0x140310310 (0x140507AA8 / 0x140507AA4).
inline constexpr float angle_to_int16 = 10430.376953125F;       // 0x4622F982
inline constexpr float int16_to_angle = 9.587380918674171e-05F; // 0x38C90FDC

// 0x14030E9B0 runs its scale pass only outside this band
// (0x14035D568 / 0x1405068E0).
inline constexpr float unit_scale_low = 0.9999899864196777F;    // 0x3F7FFF58
inline constexpr float unit_scale_high = 1.0000100135803223F;   // 0x3F800054

// 0x14030F800: rest defaults are the MOD record translation/rotation and a
// unit scale (+0x1E4/+0x204/+0x224 <- 1.0).
[[nodiscard]] inline JointChannelValues rest_channel_values(
    const formats::mod::transform_domain::LocalTransformRecord& record) noexcept {
    JointChannelValues out;
    out.translation = {record.translation.x, record.translation.y,
                       record.translation.z};
    out.rotation = {record.rotation_xyz_radians.x,
                    record.rotation_xyz_radians.y,
                    record.rotation_xyz_radians.z};
    return out;
}

// 0x140310310 rotation quantization: cvttss2si(r * 65536/2pi), low word taken
// as int16, re-expanded by 2pi/65536. NaN or out-of-int32 inputs produce the
// x86 integer indefinite 0x80000000, whose low word is 0.
[[nodiscard]] inline float quantize_motion_angle(float radians) noexcept {
    const float scaled = radians * angle_to_int16;
    std::int32_t truncated = 0;
    if (std::isfinite(scaled) &&
        scaled > static_cast<float>(std::numeric_limits<std::int32_t>::min()) &&
        scaled < static_cast<float>(std::numeric_limits<std::int32_t>::max())) {
        truncated = static_cast<std::int32_t>(scaled);
    }
    const auto wrapped = static_cast<std::int16_t>(static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(truncated) & 0xFFFFU));
    return static_cast<float>(wrapped) * int16_to_angle;
}

[[nodiscard]] inline bool has_non_unit_scale(const JointChannelValues& values) noexcept {
    for (const float factor : values.scale) {
        if (!(factor > unit_scale_low && factor < unit_scale_high)) return true;
    }
    return false;
}

// Reconstruct CMotionJoint+0x108 as written by 0x140310310: identity, XYZ
// Euler basis through 0x140330450 (the rest-pose helper), translation in row
// 3, W = 1. Scale is not part of this matrix in the EXE; it is a later pass
// (0x14030E9B0 via 0x14032ED30) that also compensates the parent scale. This
// helper applies only the row scaling of 0x14032ED30 when a factor is not
// unit, and does not claim the parent compensation.
[[nodiscard]] inline formats::mod::world_transform::Matrix4f
build_animated_local_matrix(const JointChannelValues& values) noexcept {
    formats::mod::transform_domain::LocalTransformRecord record{};
    record.translation = {values.translation[0], values.translation[1],
                          values.translation[2]};
    record.rotation_xyz_radians = {quantize_motion_angle(values.rotation[0]),
                                   quantize_motion_angle(values.rotation[1]),
                                   quantize_motion_angle(values.rotation[2])};
    auto local = formats::mod::world_transform::build_local_matrix(record);
    local.values[12] = values.translation[0];
    local.values[13] = values.translation[1];
    local.values[14] = values.translation[2];
    local.values[15] = 1.0F;
    if (has_non_unit_scale(values)) {
        for (std::size_t row = 0U; row < 3U; ++row) {
            for (std::size_t column = 0U; column < 3U; ++column) {
                local.values[row * 4U + column] *= values.scale[row];
            }
        }
    }
    return local;
}

} // namespace dmc::rengine::analysis::mot

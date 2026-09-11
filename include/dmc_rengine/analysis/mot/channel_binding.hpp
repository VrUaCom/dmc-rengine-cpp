#pragma once

#include "dmc_rengine/analysis/mot/key_decode.hpp"
#include "dmc_rengine/formats/mot/ir.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace dmc::rengine::analysis::mot {

// Canonical normal-path channel semantics recovered from dmc3.exe.
// The runtime binding path at 0x140310A61 walks the mask in this exact order.
enum class JointChannel : std::uint8_t {
    translation_x,
    translation_y,
    translation_z,
    rotation_x,
    rotation_y,
    rotation_z,
    scale_x,
    scale_y,
    scale_z,
};

struct JointChannelDescriptor final {
    std::uint16_t mask_bit{};
    JointChannel channel{};
    std::uint16_t joint_channel_base_offset{};
};

inline constexpr std::array<JointChannelDescriptor, 9>
    normal_joint_channel_order{{
        {0x040U, JointChannel::translation_x, 0x120U},
        {0x080U, JointChannel::translation_y, 0x140U},
        {0x100U, JointChannel::translation_z, 0x160U},
        {0x008U, JointChannel::rotation_x, 0x180U},
        {0x010U, JointChannel::rotation_y, 0x1A0U},
        {0x020U, JointChannel::rotation_z, 0x1C0U},
        {0x001U, JointChannel::scale_x, 0x1E0U},
        {0x002U, JointChannel::scale_y, 0x200U},
        {0x004U, JointChannel::scale_z, 0x220U},
    }};

struct BoundJointTrack final {
    std::size_t node_index{};
    std::size_t track_index{};
    std::uint16_t mask_bit{};
    JointChannel channel{};
    std::uint16_t joint_channel_base_offset{};
};

struct NormalBindingProjection final {
    std::vector<BoundJointTrack> tracks;
};

// Reconstruct the normal MOT track -> CMotionJoint channel projection.
//
// Evidence boundary:
// - mask traversal and channel offsets are EXE_CONFIRMED;
// - track ordinals are consumed in table order for every set bit;
// - this helper deliberately requires MOT channel-domain cardinality to equal
//   the supplied model node count. The executable path is bounded by model node
//   count, but arbitrary mismatched domains are not proven safe.
// - header flag 0x2 alternate binding remains outside this helper.
[[nodiscard]] inline std::optional<NormalBindingProjection>
project_normal_binding(
    const formats::mot::Document& document,
    std::size_t model_node_count) {
    if (model_node_count == 0U ||
        model_node_count != static_cast<std::size_t>(document.channel_domain_count) ||
        document.channel_masks.size() != model_node_count ||
        document.tracks.size() != static_cast<std::size_t>(document.record_count)) {
        return std::nullopt;
    }

    std::size_t expected_track_count = 0U;
    for (const auto mask : document.channel_masks) {
        for (const auto& descriptor : normal_joint_channel_order) {
            if ((mask & descriptor.mask_bit) != 0U) ++expected_track_count;
        }
    }
    if (expected_track_count != document.tracks.size()) return std::nullopt;

    NormalBindingProjection projection;
    projection.tracks.reserve(expected_track_count);

    std::size_t track_index = 0U;
    for (std::size_t node_index = 0U;
         node_index < model_node_count;
         ++node_index) {
        const auto mask = document.channel_masks[node_index];
        for (const auto& descriptor : normal_joint_channel_order) {
            if ((mask & descriptor.mask_bit) == 0U) continue;
            if (track_index >= document.tracks.size()) return std::nullopt;
            projection.tracks.push_back(BoundJointTrack{
                node_index,
                track_index,
                descriptor.mask_bit,
                descriptor.channel,
                descriptor.joint_channel_base_offset});
            ++track_index;
        }
    }

    if (track_index != document.tracks.size()) return std::nullopt;
    return projection;
}

} // namespace dmc::rengine::analysis::mot

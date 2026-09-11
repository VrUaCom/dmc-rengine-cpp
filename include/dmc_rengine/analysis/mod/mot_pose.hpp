#pragma once

#include "dmc_rengine/analysis/mod/animation_binding.hpp"
#include "dmc_rengine/analysis/mot/channel_binding.hpp"
#include "dmc_rengine/analysis/mot/track_evaluation.hpp"
#include "dmc_rengine/formats/mot/ir.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::analysis::mod {

struct EvaluatedJointChannelSample final {
    std::size_t node_index{};
    std::uint8_t motion_group{};
    std::size_t track_index{};
    dmc::rengine::analysis::mot::JointChannel channel{};
    std::uint16_t joint_channel_base_offset{};
    float value{};
    std::int32_t cached_index{};
};

struct MotionGroupChannelEvaluation final {
    std::vector<EvaluatedJointChannelSample> samples;
    std::vector<std::int32_t> updated_track_caches;
};

// Compose the EXE-confirmed normal binding with the recovered MOD motion-group
// selector for the currently supported compression-3 path.
//
// Runtime evidence establishes that track ordinals follow the MOT table even
// when a joint is excluded by the requested motion group. This helper therefore
// projects every track first, then evaluates only channels belonging to joints
// whose CMotionJoint +0xF8 selector matches `requested_group`.
//
// Excluded-track cache entries are left unchanged. This models only the
// selected-group scalar evaluation boundary; it does not claim complete CMotion
// scheduler/cache behavior, compression-2 evaluation, flag-0x2 binding, local
// matrix construction, blending or looping.
[[nodiscard]] inline std::optional<MotionGroupChannelEvaluation>
evaluate_motion_group_compression3_channels(
    const dmc::rengine::formats::mot::Document& document,
    const AnimationBindingProjection& model_binding,
    std::uint8_t requested_group,
    float evaluation_time,
    std::span<const std::int32_t> track_caches) {
    const auto node_count = model_binding.by_node_index.size();
    if (node_count == 0U ||
        model_binding.node_at_order_position.size() != node_count ||
        node_count != static_cast<std::size_t>(document.channel_domain_count) ||
        track_caches.size() != document.tracks.size()) {
        return std::nullopt;
    }

    const auto mot_binding =
        dmc::rengine::analysis::mot::project_normal_binding(
            document, node_count);
    if (!mot_binding.has_value()) return std::nullopt;

    MotionGroupChannelEvaluation result;
    result.updated_track_caches.assign(
        track_caches.begin(), track_caches.end());

    for (const auto& bound : mot_binding->tracks) {
        if (bound.node_index >= model_binding.by_node_index.size() ||
            bound.track_index >= document.tracks.size()) {
            return std::nullopt;
        }

        const auto& joint = model_binding.by_node_index[bound.node_index];
        if (static_cast<std::size_t>(joint.node_index) != bound.node_index)
            return std::nullopt;

        // Track ordinal ownership is already fixed by project_normal_binding;
        // group filtering affects application, not serialized traversal.
        if (joint.motion_group != requested_group) continue;

        const auto evaluated =
            dmc::rengine::analysis::mot::evaluate_compression3_track(
                document.tracks[bound.track_index],
                evaluation_time,
                track_caches[bound.track_index]);
        if (!evaluated.has_value()) return std::nullopt;

        result.updated_track_caches[bound.track_index] =
            evaluated->cached_index;
        result.samples.push_back(EvaluatedJointChannelSample{
            bound.node_index,
            joint.motion_group,
            bound.track_index,
            bound.channel,
            bound.joint_channel_base_offset,
            evaluated->value,
            evaluated->cached_index});
    }

    return result;
}

} // namespace dmc::rengine::analysis::mod

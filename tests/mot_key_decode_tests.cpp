#include "dmc_rengine/analysis/mod/mot_pose.hpp"
#include "dmc_rengine/analysis/mot/channel_binding.hpp"
#include "dmc_rengine/analysis/mot/key_decode.hpp"
#include "dmc_rengine/analysis/mot/track_evaluation.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <limits>

int main() {
    namespace mod_analysis = dmc::rengine::analysis::mod;
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
    assert(!mot::evaluate_segment3(
        *left, *right, std::numeric_limits<float>::quiet_NaN()));
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

    // 0x1402E8C80..0x1402E8E10: cached compression-3 segment search.
    dmc::rengine::formats::mot::TrackRecord search_track;
    search_track.compression = 3;
    search_track.quantization_float_count = 6;
    search_track.key_count = 3;
    search_track.keys3 = {
        {0x8000, 0, 0, 0},
        {0x000A, 0, 0, 0},
        {0x8014, 0, 0, 0},
    };

    const auto forward = mot::select_cached_segment3(search_track, 15.0F, 1);
    assert(forward && forward->is_segment());
    assert(forward->left_index == 1U && forward->right_index == 2U);
    assert(forward->cached_index == 1);

    const auto backward = mot::select_cached_segment3(search_track, 5.0F, 1);
    assert(backward && backward->is_segment());
    assert(backward->left_index == 0U && backward->right_index == 1U);
    assert(backward->cached_index == 0);

    const auto before_first =
        mot::select_cached_segment3(search_track, -1.0F, 1);
    assert(before_first && !before_first->is_segment());
    assert(before_first->left_index == 0U && before_first->right_index == 0U);
    assert(before_first->cached_index == 0);

    const auto after_last =
        mot::select_cached_segment3(search_track, 25.0F, 1);
    assert(after_last && !after_last->is_segment());
    assert(after_last->left_index == 2U && after_last->right_index == 2U);
    assert(after_last->cached_index == 2);

    // Equal-time runs are intentionally cache-dependent in the recovered loop.
    search_track.key_count = 4;
    search_track.keys3 = {
        {0x0000, 0, 0, 0},
        {0x000A, 0, 0, 0},
        {0x800A, 0, 0, 0},
        {0x0014, 0, 0, 0},
    };
    const auto duplicate_from_left =
        mot::select_cached_segment3(search_track, 10.0F, 0);
    assert(duplicate_from_left && !duplicate_from_left->is_segment());
    assert(duplicate_from_left->left_index == 1U);
    assert(duplicate_from_left->cached_index == 1);

    const auto duplicate_from_right =
        mot::select_cached_segment3(search_track, 10.0F, 2);
    assert(duplicate_from_right && duplicate_from_right->is_segment());
    assert(duplicate_from_right->left_index == 2U &&
           duplicate_from_right->right_index == 3U);
    assert(duplicate_from_right->cached_index == 2);

    assert(!mot::select_cached_segment3(
        search_track, std::numeric_limits<float>::quiet_NaN(), 0));
    assert(!mot::select_cached_segment3(search_track, 10.0F, -1));
    assert(!mot::select_cached_segment3(search_track, 10.0F, 4));

    // Fail closed on parser-valid but runtime-ineligible signed key counts.
    search_track.key_count = 0x8000U;
    search_track.keys3.resize(0x8000U);
    assert(!mot::select_cached_segment3(search_track, 0.0F, 0));

    // Fail closed when masked key times are not sorted.
    search_track.key_count = 3;
    search_track.keys3 = {
        {0x0000, 0, 0, 0},
        {0x0014, 0, 0, 0},
        {0x000A, 0, 0, 0},
    };
    assert(!mot::select_cached_segment3(search_track, 5.0F, 0));

    // Normal MOT mask traversal -> CMotionJoint channel semantics.
    dmc::rengine::formats::mot::Document binding_document;
    binding_document.channel_domain_count = 2;
    binding_document.channel_masks = {0x1C0U, 0x038U};
    binding_document.record_count = 6;
    binding_document.tracks.resize(6);

    const auto binding = mot::project_normal_binding(binding_document, 2U);
    assert(binding && binding->tracks.size() == 6U);
    assert(binding->tracks[0].node_index == 0U);
    assert(binding->tracks[0].track_index == 0U);
    assert(binding->tracks[0].channel == mot::JointChannel::translation_x);
    assert(binding->tracks[0].joint_channel_base_offset == 0x120U);
    assert(binding->tracks[2].channel == mot::JointChannel::translation_z);
    assert(binding->tracks[2].joint_channel_base_offset == 0x160U);
    assert(binding->tracks[3].node_index == 1U);
    assert(binding->tracks[3].track_index == 3U);
    assert(binding->tracks[3].channel == mot::JointChannel::rotation_x);
    assert(binding->tracks[5].channel == mot::JointChannel::rotation_z);
    assert(binding->tracks[5].joint_channel_base_offset == 0x1C0U);
    assert(!mot::project_normal_binding(binding_document, 1U));
    binding_document.tracks.pop_back();
    assert(!mot::project_normal_binding(binding_document, 2U));

    // Full bounded compression-3 scalar path: signed offset + cached search +
    // key decode + interpolation. With start offset -10, raw key times 10/20
    // become global times 0/10.
    dmc::rengine::formats::mot::TrackRecord eval_track;
    eval_track.compression = 3;
    eval_track.key_count = 2;
    eval_track.start_time_raw = 0xFFF6U;
    eval_track.quantization_float_count = 6;
    eval_track.quantization_raw = {0.0F, 10.0F, 0.0F, 0.0F, 0.0F, 0.0F};
    eval_track.keys3 = {
        {0x000AU, 0U, 0U, 0U},
        {0x0014U, 65535U, 0U, 0U},
    };

    const auto scalar_mid =
        mot::evaluate_compression3_track(eval_track, 5.0F, 0);
    assert(scalar_mid && scalar_mid->value == 5.0F);
    assert(scalar_mid->selection_kind == mot::SegmentSelectionKind::segment);
    assert(scalar_mid->left_index == 0U && scalar_mid->right_index == 1U);
    assert(scalar_mid->cached_index == 0);

    const auto scalar_before =
        mot::evaluate_compression3_track(eval_track, -5.0F, 0);
    assert(scalar_before && scalar_before->value == 0.0F);
    assert(scalar_before->selection_kind ==
           mot::SegmentSelectionKind::single_key);

    const auto scalar_after =
        mot::evaluate_compression3_track(eval_track, 20.0F, 0);
    assert(scalar_after && scalar_after->value == 10.0F);
    assert(scalar_after->cached_index == 1);

    eval_track.compression = 2;
    assert(!mot::evaluate_compression3_track(eval_track, 5.0F, 0));
    eval_track.compression = 3;
    assert(!mot::evaluate_compression3_track(
        eval_track, std::numeric_limits<float>::quiet_NaN(), 0));

    // MOT -> MOD group-aware scalar bridge. Track ordinals for the excluded
    // group remain present, but only matching CMotionJoint +0xF8 selectors are
    // evaluated/applied.
    binding_document.tracks.assign(6U, eval_track);
    mod_analysis::AnimationBindingProjection model_binding;
    model_binding.by_node_index = {
        mod_analysis::AnimationJointBinding{0U, -1, 0U, 0U},
        mod_analysis::AnimationJointBinding{1U, 0, 1U, 1U},
    };
    model_binding.node_at_order_position = {0U, 1U};
    const std::array<std::int32_t, 6> caches{0, 0, 0, 0, 0, 0};

    const auto group0 = mod_analysis::evaluate_motion_group_compression3_channels(
        binding_document, model_binding, 0U, 5.0F, caches);
    assert(group0 && group0->samples.size() == 3U);
    assert(group0->samples[0].node_index == 0U);
    assert(group0->samples[0].track_index == 0U);
    assert(group0->samples[0].channel == mot::JointChannel::translation_x);
    assert(group0->samples[0].value == 5.0F);
    assert(group0->samples[2].track_index == 2U);

    const auto group1 = mod_analysis::evaluate_motion_group_compression3_channels(
        binding_document, model_binding, 1U, 5.0F, caches);
    assert(group1 && group1->samples.size() == 3U);
    assert(group1->samples[0].node_index == 1U);
    assert(group1->samples[0].track_index == 3U);
    assert(group1->samples[0].channel == mot::JointChannel::rotation_x);
    assert(group1->samples[2].track_index == 5U);

    auto wrong_caches = std::array<std::int32_t, 5>{0, 0, 0, 0, 0};
    assert(!mod_analysis::evaluate_motion_group_compression3_channels(
        binding_document, model_binding, 0U, 5.0F, wrong_caches));
}

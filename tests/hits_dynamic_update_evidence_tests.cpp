#include "dmc_rengine/profiles/dmc3/hits_dynamic_update_evidence.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_dynamic_update_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    assert(matches_canonical_target(canonical_sha));
    assert(!matches_canonical_target(packed_sha));

    const auto pair_body = canonical_function_body(canonical_sha, DynamicUpdateBodyRole::pair_resolver);
    assert(pair_body.has_value());
    assert(pair_body->begin_va == 0x14005B460ULL);
    assert(pair_body->end_va == 0x14005B6E6ULL);
    assert(pair_body->size_bytes == 646U);
    assert(pair_body->sha256 ==
           "cba77cf4bc20dedfbd590991012fa471e6b5ae02b9cec57c66fce69333a67972");

    const auto compatibility_body =
        canonical_function_body(canonical_sha, DynamicUpdateBodyRole::category_compatibility);
    assert(compatibility_body.has_value());
    assert(compatibility_body->begin_va == 0x14005B6F0ULL);
    assert(compatibility_body->size_bytes == 184U);

    const auto dispatcher_body =
        canonical_function_body(canonical_sha, DynamicUpdateBodyRole::dispatcher);
    assert(dispatcher_body.has_value());
    assert(dispatcher_body->begin_va == 0x14005B7B0ULL);
    assert(dispatcher_body->size_bytes == 301U);

    const auto post_body =
        canonical_function_body(canonical_sha, DynamicUpdateBodyRole::post_update_step);
    assert(post_body.has_value());
    assert(post_body->begin_va == 0x14005B8E0ULL);
    assert(post_body->end_va == 0x14005BA64ULL);
    assert(post_body->size_bytes == 388U);
    assert(post_body->sha256 ==
           "1d149357ffa3fe5024c0a6264e356f476c14c6366d89b38bb50a1a3f18e2f477");

    const auto pair = pair_resolver_abi(canonical_sha);
    assert(pair.has_value());
    assert(pair->function_va == 0x14005B460ULL);
    assert(pair->manager_arg_index == 1U);
    assert(pair->source_object_arg_index == 2U);
    assert(pair->candidate_list_head_arg_index == 3U);
    assert(pair->source_category_arg_index == 4U);
    assert(pair->target_category_arg_index == 5U);
    assert(pair->linked_next_offset == 0x328U);
    assert(pair->candidate_type_offset == 0x00U);
    assert(pair->required_candidate_type == 2U);
    assert(pair->owner_pointer_offset == 0xD0U);
    assert(pair->reciprocal_category_bits_offset == 0x10U);
    assert(pair->rejects_self_pair);
    assert(pair->rejects_shared_owner);
    assert(pair->calls_category_compatibility_filter);
    assert(pair->applies_contact_displacement);
    assert(pair->sets_reciprocal_category_bits);

    const auto compatibility = category_compatibility_evidence(canonical_sha);
    assert(compatibility.has_value());
    assert(compatibility->function_va == 0x14005B6F0ULL);
    assert(compatibility->special_category_id == 0x0EU);
    assert(compatibility->raw_flags_offset == 0xDAU);
    assert(compatibility->raw_flags_width_bytes == 2U);
    assert(compatibility->neither_special_category_is_allowed);
    assert(compatibility->both_special_category_is_allowed);
    assert(compatibility->one_special_category_uses_other_category_mask);

    const std::uint32_t categories[] = {0x02U, 0x05U, 0x08U, 0x0BU};
    const std::uint16_t masks[] = {0x0040U, 0x0002U, 0x0010U, 0x0020U};
    for (std::size_t i = 0; i < 4U; ++i) {
        const auto bridge = category_mask_bridge(canonical_sha, categories[i]);
        assert(bridge.has_value());
        assert(bridge->raw_reject_mask == masks[i]);
    }
    assert(!category_mask_bridge(canonical_sha, 0x0EU).has_value());

    const std::uint32_t flags[] = {0x1000U, 0x2000U, 0x4000U, 0x8000U, 0x10000U, 0x20000U};
    const std::uint32_t offsets[] = {0x10U, 0x28U, 0x40U, 0x58U, 0x70U, 0x88U};
    const std::uint32_t target_categories[] = {0x02U, 0x05U, 0x08U, 0x0BU, 0x0EU, 0x11U};
    for (std::size_t i = 0; i < 6U; ++i) {
        const auto binding = dispatcher_binding(canonical_sha, i);
        assert(binding.has_value());
        assert(binding->activation_flag == flags[i]);
        assert(binding->manager_list_offset == offsets[i]);
        assert(binding->target_category_id == target_categories[i]);
    }
    assert(!dispatcher_binding(canonical_sha, 6U).has_value());

    const auto dispatcher = dispatcher_evidence(canonical_sha);
    assert(dispatcher.has_value());
    assert(dispatcher->function_va == 0x14005B7B0ULL);
    assert(dispatcher->pair_resolver_va == 0x14005B460ULL);
    assert(dispatcher->post_update_step_va == 0x14005B8E0ULL);
    assert(dispatcher->iterates_source_object_list);
    assert(dispatcher->dispatches_enabled_target_categories);
    assert(dispatcher->runs_post_update_step_after_pair_dispatch);

    const auto post = post_update_step_evidence(canonical_sha);
    assert(post.has_value());
    assert(post->function_va == 0x14005B8E0ULL);
    assert(post->result_category_bits_offset == 0x10U);
    assert(post->dynamic_contact_result_bit == 0x00020000U);
    assert(post->uses_category_to_reject_mask_bridge);
    assert(post->invokes_specialized_hits_queries);
    assert(post->may_apply_position_correction);

    assert(!canonical_function_body(packed_sha, DynamicUpdateBodyRole::pair_resolver).has_value());
    assert(!pair_resolver_abi(packed_sha).has_value());
    assert(!category_compatibility_evidence(packed_sha).has_value());
    assert(!category_mask_bridge(packed_sha, 0x02U).has_value());
    assert(!dispatcher_binding(packed_sha, 0U).has_value());
    assert(!dispatcher_evidence(packed_sha).has_value());
    assert(!post_update_step_evidence(packed_sha).has_value());

    return 0;
}

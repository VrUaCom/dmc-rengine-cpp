#include "dmc_rengine/profiles/dmc3/hits_contact_normal_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_contact_normal_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    assert(matches_canonical_target(canonical_sha));
    assert(!matches_canonical_target(packed_sha));

    const auto common = common_surface_normal_evidence(canonical_sha);
    assert(common.has_value());
    assert(common->metadata_offset == 0x28U);
    assert(common->component_count == 3U);
    assert(common->static_raw_record_offset == 0x28U);
    assert(common->dynamic_metadata_output_offset == 0x28U);
    assert(common->static_facing_dot_product_confirmed);
    assert(common->dynamic_sphere_radial_normal_confirmed);
    assert(common->dynamic_three_vertex_face_normal_confirmed);
    assert(common->dynamic_cylinder_cap_and_radial_normal_confirmed);
    assert(common->reflection_consumer_confirmed);
    assert(common->common_collision_surface_normal_semantics_confirmed);
    assert(!common->exact_orientation_or_handedness_confirmed);

    const auto static_normal = static_surface_normal_evidence(canonical_sha);
    assert(static_normal.has_value());
    assert(static_normal->static_query_va == 0x14005E880ULL);
    assert(static_normal->raw_record_normal_offset == 0x28U);
    assert(static_normal->direction_normalizer_va == 0x140330390ULL);
    assert(static_normal->facing_dot3_va == 0x14032E5C0ULL);
    assert(static_normal->query_direction_is_normalized);
    assert(static_normal->record_vector_is_used_as_facing_normal);

    const auto dynamic_normal = dynamic_surface_normal_evidence(canonical_sha);
    assert(dynamic_normal.has_value());
    assert(dynamic_normal->dynamic_query_va == 0x14005BCF0ULL);
    assert(dynamic_normal->metadata_normal_offset == 0x28U);
    assert(dynamic_normal->sphere_runtime_type == 2U);
    assert(dynamic_normal->three_vertex_face_runtime_type == 5U);
    assert(dynamic_normal->cylinder_runtime_type == 6U);
    assert(dynamic_normal->three_vertex_face_object_normal_offset == 0x160U);
    assert(dynamic_normal->sphere_uses_normalized_contact_minus_center);
    assert(dynamic_normal->face_uses_pre_normalized_object_direction);
    assert(dynamic_normal->cylinder_uses_axial_or_normalized_radial_direction);

    const auto reflection = reflection_consumer_evidence(canonical_sha);
    assert(reflection.has_value());
    assert(reflection->consumer_va == 0x1402C64F0ULL);
    assert(reflection->query_callsite_va == 0x1402C65B2ULL);
    assert(reflection->metadata_normal_offset == 0x28U);
    assert(reflection->dot3_helper_va == 0x140030D30ULL);
    assert(reflection->xyz_scale_preserve_w_helper_va == 0x1400311E0ULL);
    assert(reflection->computes_v_minus_two_dot_v_n_times_n);

    const auto dot3 = canonical_function_body(canonical_sha, ContactNormalBodyRole::static_facing_dot3);
    assert(dot3.has_value());
    assert(dot3->begin_va == 0x14032E5C0ULL);
    assert(dot3->end_va == 0x14032E5E5ULL);
    assert(dot3->size_bytes == 37U);
    assert(dot3->sha256 ==
           "d9dba88696b4f17f15617eb2dd1b2e39713da0d0225f4e2130e0efc0931c0779");

    const auto normalize =
        canonical_function_body(canonical_sha, ContactNormalBodyRole::normalize_xyz_direction);
    assert(normalize.has_value());
    assert(normalize->begin_va == 0x140330390ULL);
    assert(normalize->size_bytes == 186U);

    const auto direction_transform =
        canonical_function_body(canonical_sha, ContactNormalBodyRole::transform_direction_3x3);
    assert(direction_transform.has_value());
    assert(direction_transform->begin_va == 0x14032DC70ULL);
    assert(direction_transform->size_bytes == 134U);

    const auto consumer =
        canonical_function_body(canonical_sha, ContactNormalBodyRole::reflection_consumer);
    assert(consumer.has_value());
    assert(consumer->begin_va == 0x1402C64F0ULL);
    assert(consumer->end_va == 0x1402C6805ULL);
    assert(consumer->size_bytes == 789U);
    assert(consumer->sha256 ==
           "86b77b7431411f7b2ed1c6f7b1109aebd36c305e2e8684c6d6897f915b76e7c4");

    assert(!common_surface_normal_evidence(packed_sha).has_value());
    assert(!static_surface_normal_evidence(packed_sha).has_value());
    assert(!dynamic_surface_normal_evidence(packed_sha).has_value());
    assert(!reflection_consumer_evidence(packed_sha).has_value());
    assert(!canonical_function_body(packed_sha, ContactNormalBodyRole::reflection_consumer)
                .has_value());

    return 0;
}

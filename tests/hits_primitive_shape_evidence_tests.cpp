#include "dmc_rengine/profiles/dmc3/hits_primitive_shape_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_primitive_shape_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    assert(matches_canonical_target(canonical_sha));
    assert(!matches_canonical_target(packed_sha));

    const auto sphere = parsed_shape_mapping(canonical_sha, ParsedShapeKind::sphere);
    const auto box = parsed_shape_mapping(canonical_sha, ParsedShapeKind::box);
    const auto cylinder = parsed_shape_mapping(canonical_sha, ParsedShapeKind::cylinder);
    const auto capsule = parsed_shape_mapping(canonical_sha, ParsedShapeKind::capsule);
    assert(sphere.has_value() && sphere->token == "sphere");
    assert(sphere->parser_enum_value == 0U);
    assert(sphere->internal_descriptor_type == 2U);
    assert(sphere->primary_runtime_type == 2U);
    assert(box.has_value() && box->parser_enum_value == 1U);
    assert(box->internal_descriptor_type == 3U && box->primary_runtime_type == 3U);
    assert(cylinder.has_value() && cylinder->parser_enum_value == 2U);
    assert(cylinder->internal_descriptor_type == 6U && cylinder->primary_runtime_type == 6U);
    assert(capsule.has_value() && capsule->parser_enum_value == 3U);
    assert(capsule->internal_descriptor_type == 4U && capsule->primary_runtime_type == 4U);

    const auto flow = parser_to_converter_flow(canonical_sha);
    assert(flow.has_value());
    assert(flow->parser_va == 0x140247AD0ULL);
    assert(flow->builder_va == 0x140249710ULL);
    assert(flow->converter_va == 0x1402481E0ULL);
    assert(flow->parser_frame_shape_offset == -0x38);
    assert(flow->builder_shape_member_offset == 0x98U);
    assert(flow->same_parsed_shape_storage_proven);

    const auto parser_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::shape_parser_cluster);
    assert(parser_body.has_value());
    assert(parser_body->begin_va == 0x140247AD0ULL);
    assert(parser_body->end_va == 0x1402481E0ULL);
    assert(parser_body->size_bytes == 1808U);
    assert(parser_body->sha256 ==
           "1bd8a8e369105512166e5f2557d274c86e3801b56631f0d4f7022983adfda6e2");

    const auto converter_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::parsed_shape_converter);
    assert(converter_body.has_value());
    assert(converter_body->begin_va == 0x1402481E0ULL);
    assert(converter_body->end_va == 0x1402482C5ULL);
    assert(converter_body->size_bytes == 229U);

    const auto dispatcher_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::runtime_constructor_dispatcher);
    assert(dispatcher_body.has_value());
    assert(dispatcher_body->begin_va == 0x1402CC530ULL);
    assert(dispatcher_body->end_va == 0x1402CC5EFULL);
    assert(dispatcher_body->size_bytes == 191U);

    const auto box_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::runtime_box_constructor);
    assert(box_body.has_value());
    assert(box_body->begin_va == 0x1402CC610ULL);
    assert(box_body->end_va == 0x1402CC884ULL);
    assert(box_body->size_bytes == 628U);
    assert(box_body->sha256 ==
           "d5d6e4d714626cafeaee21697d856887c4d6505c092c3498f14e82a838eea821");

    const auto capsule_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::runtime_capsule_constructor);
    assert(capsule_body.has_value());
    assert(capsule_body->end_va == 0x1402CC994ULL);
    assert(capsule_body->size_bytes == 260U);

    const auto cylinder_body =
        canonical_function_body(canonical_sha, PrimitiveBodyRole::runtime_cylinder_constructor);
    assert(cylinder_body.has_value());
    assert(cylinder_body->end_va == 0x1402CCA26ULL);
    assert(cylinder_body->size_bytes == 134U);

    const auto sphere_body = canonical_function_body(
        canonical_sha, PrimitiveBodyRole::runtime_sphere_or_swept_sphere_constructor);
    assert(sphere_body.has_value());
    assert(sphere_body->begin_va == 0x1402CCB50ULL);
    assert(sphere_body->end_va == 0x1402CCCB0ULL);
    assert(sphere_body->size_bytes == 352U);

    const auto type4 = runtime_type4_evidence(canonical_sha);
    assert(type4.has_value());
    assert(type4->runtime_type == 4U);
    assert(type4->parsed_capsule_descriptor_type == 4U);
    assert(type4->swept_sphere_descriptor_type == 2U);
    assert(type4->first_endpoint_offset == 0x130U);
    assert(type4->second_endpoint_offset == 0x140U);
    assert(type4->radius_offset == 0x150U);
    assert(type4->parsed_capsule_builds_runtime_type4);
    assert(type4->moving_sphere_may_promote_to_runtime_type4);
    assert(type4->sphere_promotion_threshold_is_two_times_radius);

    const auto type5_body = canonical_function_body(
        canonical_sha, PrimitiveBodyRole::runtime_three_vertex_face_constructor);
    assert(type5_body.has_value());
    assert(type5_body->begin_va == 0x1402CCCB0ULL);
    assert(type5_body->end_va == 0x1402CCD5BULL);
    assert(type5_body->size_bytes == 171U);
    assert(type5_body->sha256 ==
           "766bf79c39056c7baaa6bb1cc097fab863651f7b5350edd2b07ff0509f432a2e");

    const auto type5 = runtime_type5_structural_evidence(canonical_sha);
    assert(type5.has_value());
    assert(type5->runtime_type == 5U);
    assert(type5->internal_descriptor_type == 5U);
    assert(type5->first_vertex_offset == 0x130U);
    assert(type5->second_vertex_offset == 0x140U);
    assert(type5->third_vertex_offset == 0x150U);
    assert(type5->auxiliary_vector_offset == 0x160U);
    assert(type5->vertex_count == 3U);
    assert(type5->transforms_three_vertices);
    assert(type5->normalizes_auxiliary_vector);
    assert(type5->three_vertex_face_representation_confirmed);
    assert(!type5->source_text_shape_name_confirmed);

    assert(!parsed_shape_mapping(packed_sha, ParsedShapeKind::sphere).has_value());
    assert(!parser_to_converter_flow(packed_sha).has_value());
    assert(!canonical_function_body(packed_sha, PrimitiveBodyRole::runtime_box_constructor)
                .has_value());
    assert(!runtime_type4_evidence(packed_sha).has_value());
    assert(!runtime_type5_structural_evidence(packed_sha).has_value());

    return 0;
}

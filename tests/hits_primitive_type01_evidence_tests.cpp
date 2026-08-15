#include "dmc_rengine/profiles/dmc3/hits_primitive_type01_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_primitive_type01_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    const auto type0 = runtime_type0_evidence(canonical_sha);
    assert(type0.has_value());
    assert(type0->runtime_type == 0U);
    assert(type0->descriptor_point_offset == 0x10U);
    assert(type0->runtime_point_offset == 0x130U);
    assert(type0->transforms_one_position);
    assert(type0->one_point_representation_confirmed);
    assert(!type0->source_token_confirmed);

    const auto type1 = runtime_type1_evidence(canonical_sha);
    assert(type1.has_value());
    assert(type1->runtime_type == 1U);
    assert(type1->descriptor_first_endpoint_offset == 0x10U);
    assert(type1->descriptor_second_endpoint_offset == 0x20U);
    assert(type1->runtime_first_endpoint_offset == 0x130U);
    assert(type1->runtime_second_endpoint_offset == 0x140U);
    assert(type1->runtime_midpoint_offset == 0xE0U);
    assert(type1->runtime_endpoint_distance_offset == 0x18U);
    assert(type1->transforms_two_endpoints);
    assert(type1->computes_midpoint);
    assert(type1->computes_xyz_endpoint_distance);
    assert(type1->contact_update_may_replace_first_endpoint);
    assert(type1->two_endpoint_segment_representation_confirmed);
    assert(!type1->source_token_confirmed);

    const auto type0_body =
        canonical_function_body(canonical_sha, Primitive01BodyRole::runtime_type0_constructor);
    assert(type0_body.has_value());
    assert(type0_body->begin_va == 0x1402CCA30ULL);
    assert(type0_body->end_va == 0x1402CCA64ULL);
    assert(type0_body->size_bytes == 52U);
    assert(type0_body->sha256 ==
           "8033e51cb704386397c4ee988304b55dbffd538e0710514eaedc785da26ba132");

    const auto type1_body =
        canonical_function_body(canonical_sha, Primitive01BodyRole::runtime_type1_constructor);
    assert(type1_body.has_value());
    assert(type1_body->begin_va == 0x1402CCA70ULL);
    assert(type1_body->end_va == 0x1402CCB46ULL);
    assert(type1_body->size_bytes == 214U);
    assert(type1_body->sha256 ==
           "c06cf1554f9b98b3d012ab0f83b3d2ae85d1a0b0c7a49ea69c573cd4820218fd");

    const auto distance_body =
        canonical_function_body(canonical_sha, Primitive01BodyRole::xyz_endpoint_distance);
    assert(distance_body.has_value());
    assert(distance_body->begin_va == 0x14032E5F0ULL);
    assert(distance_body->end_va == 0x14032E62DULL);
    assert(distance_body->size_bytes == 61U);
    assert(distance_body->sha256 ==
           "7ca8c6b01c910665e3f70490b9339010ab6a17a5193779f29f0863278520f7d1");

    assert(!runtime_type0_evidence(packed_sha).has_value());
    assert(!runtime_type1_evidence(packed_sha).has_value());
    assert(!canonical_function_body(packed_sha, Primitive01BodyRole::runtime_type1_constructor)
                .has_value());

    return 0;
}

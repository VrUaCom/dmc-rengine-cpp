#include "dmc_rengine/profiles/dmc3/hits_source1_query_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_source1_query_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    assert(matches_canonical_target(canonical_sha));
    assert(!matches_canonical_target(packed_sha));

    const auto fec0_body = canonical_function_body(canonical_sha, Source1QueryRole::segment_correction_fec0);
    assert(fec0_body.has_value());
    assert(fec0_body->begin_va == 0x14005FEC0ULL);
    assert(fec0_body->end_va == 0x1400601D3ULL);
    assert(fec0_body->size_bytes == 787U);
    assert(fec0_body->sha256 ==
           "cfaca9752adf3a581969875e99c6c1b9ffaa7f83d19d22fddbe8dec2d5cab09e");

    const auto fec0 = segment_correction_evidence(canonical_sha);
    assert(fec0.has_value());
    assert(fec0->runtime_arg_index == 1U);
    assert(fec0->mutable_point_arg_index == 2U);
    assert(fec0->reference_point_arg_index == 3U);
    assert(fec0->direct_static_caller_count == 1U);
    assert(fec0->hard_rejected_raw_record_bit == 0x00080000U);
    assert(fec0->computes_reference_minus_mutable_direction);
    assert(fec0->xyz_length_controls_degenerate_return);
    assert(fec0->direction_normalizer_zeros_fourth_component);
    assert(fec0->nondegenerate_path_writes_mutable_point);
    assert(fec0->nondegenerate_path_returns_true_even_without_accepted_record);
    assert(!fec0->return_value_is_hit_boolean);

    const auto p601e0_body =
        canonical_function_body(canonical_sha, Source1QueryRole::displacement_accumulator_601e0);
    assert(p601e0_body.has_value());
    assert(p601e0_body->begin_va == 0x1400601E0ULL);
    assert(p601e0_body->end_va == 0x14006078EULL);
    assert(p601e0_body->size_bytes == 1454U);
    assert(p601e0_body->sha256 ==
           "7fb7fb7ce9447a5e200ba1471774eafe7e5f21877da71902f047563bd9f7597a");

    const auto p601e0 = displacement_accumulator_evidence(canonical_sha);
    assert(p601e0.has_value());
    assert(p601e0->runtime_arg_index == 1U);
    assert(p601e0->mutable_point_arg_index == 2U);
    assert(p601e0->reference_point_arg_index == 3U);
    assert(p601e0->direct_static_caller_count == 5U);
    assert(p601e0->performs_initial_record_correction_pass);
    assert(p601e0->performs_secondary_normal_accumulation_pass);
    assert(p601e0->mutates_point_in_place);
    assert(p601e0->return_reports_xyz_displacement_from_original);
    assert(p601e0->xyz_norm_and_dot_exclude_fourth_component);
    assert(p601e0->four_component_subtract_scale_add_is_used);
    assert(!p601e0->fourth_component_gameplay_semantics_resolved);

    assert(!canonical_function_body(packed_sha, Source1QueryRole::segment_correction_fec0).has_value());
    assert(!segment_correction_evidence(packed_sha).has_value());
    assert(!displacement_accumulator_evidence(packed_sha).has_value());

    return 0;
}

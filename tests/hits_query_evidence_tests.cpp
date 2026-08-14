#include "dmc_rengine/profiles/dmc3/hits_query_evidence.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    assert(matches_canonical_target(canonical_sha));
    assert(!matches_canonical_target(packed_sha));
    assert(!matches_canonical_target("not-a-sha256"));
    assert(query_evidence_count(canonical_sha) == 9U);
    assert(query_evidence_count(packed_sha) == 0U);

    const auto ebe0 = query_evidence(canonical_sha, QueryVariant::inout_correction_05ebe0);
    assert(ebe0.has_value());
    assert(ebe0->function_va == 0x14005EBE0ULL);
    assert(ebe0->direct_static_caller_count == 1U);
    assert(ebe0->success_observed_in_al);
    assert(ebe0->mutable_16_byte_inout_confirmed);
    assert(!ebe0->object_f0_argument_confirmed);
    assert(!ebe0->independently_rejected_raw_flag_bit.has_value());

    const auto f070 = query_evidence(canonical_sha, QueryVariant::inout_correction_extended_05f070);
    assert(f070.has_value());
    assert(f070->function_va == 0x14005F070ULL);
    assert(f070->direct_static_caller_count == 1U);
    assert(f070->abi_kind == SpecializedAbiKind::mutable_vec4_inout_plus_aux);
    assert(f070->object_f0_argument_confirmed);

    const auto ee40 = query_evidence(canonical_sha, QueryVariant::bool_observed_05ee40);
    assert(ee40.has_value());
    assert(ee40->success_observed_in_al);
    assert(!ee40->mutable_16_byte_inout_confirmed);

    const auto e7a0 = query_evidence(canonical_sha, QueryVariant::generic_combined_05e7a0);
    assert(e7a0.has_value());
    assert(e7a0->direct_static_caller_count == 51U);
    assert(e7a0->abi_kind == SpecializedAbiKind::unresolved);
    assert(!e7a0->mutable_16_byte_inout_confirmed);

    const auto fec0 = query_evidence(canonical_sha, QueryVariant::source_selectable_segment_05fec0);
    assert(fec0.has_value());
    assert(fec0->direct_static_caller_count == 1U);
    assert(fec0->independently_rejected_raw_flag_bit.has_value());
    assert(*fec0->independently_rejected_raw_flag_bit == 0x00080000U);
    assert(fec0->abi_kind == SpecializedAbiKind::unresolved);

    const auto correction = query_evidence(canonical_sha, QueryVariant::positional_correction_0601e0);
    assert(correction.has_value());
    assert(correction->direct_static_caller_count == 5U);
    assert(correction->abi_kind == SpecializedAbiKind::unresolved);

    assert(!query_evidence(packed_sha, QueryVariant::generic_combined_05e7a0).has_value());

    const std::uint32_t expected_categories[] = {0x02U, 0x05U, 0x08U, 0x0BU, 0x0EU, 0x11U};
    const std::uint16_t expected_masks[] = {0x0040U, 0x0002U, 0x0010U, 0x0020U, 0x0000U, 0x0000U};
    const std::uint32_t expected_flags[] = {0x1000U, 0x2000U, 0x4000U, 0x8000U, 0x10000U, 0x20000U};
    const std::uint32_t expected_offsets[] = {0x10U, 0x28U, 0x40U, 0x58U, 0x70U, 0x88U};

    for (std::size_t i = 0; i < 6U; ++i) {
        const auto by_category = dynamic_category_binding(canonical_sha, expected_categories[i]);
        assert(by_category.has_value());
        assert(by_category->dispatcher_static_hits_reject_mask == expected_masks[i]);
        assert(by_category->activation_flag == expected_flags[i]);
        assert(by_category->manager_field_offset == expected_offsets[i]);

        const auto by_activation =
            dynamic_category_binding_from_activation(canonical_sha, expected_flags[i]);
        assert(by_activation.has_value());
        assert(by_activation->category_id == expected_categories[i]);
    }

    assert(!dynamic_category_binding(canonical_sha, 0x03U).has_value());
    assert(!dynamic_category_binding_from_activation(canonical_sha, 0x40000U).has_value());
    assert(!dynamic_category_binding(packed_sha, 0x02U).has_value());

    const auto source0 = wrapper_source_evidence(canonical_sha, WrapperSourceIndex::stage_member_3);
    assert(source0.has_value());
    assert(source0->stage_pac_backed_confirmed);
    assert(source0->direct_static_selection_confirmed);

    const auto source1 = wrapper_source_evidence(canonical_sha, WrapperSourceIndex::stage_member_6);
    assert(source1.has_value());
    assert(source1->stage_pac_backed_confirmed);
    assert(source1->direct_static_selection_confirmed);

    const auto source2 =
        wrapper_source_evidence(canonical_sha, WrapperSourceIndex::external_global_source);
    assert(source2.has_value());
    assert(source2->structurally_present);
    assert(!source2->stage_pac_backed_confirmed);
    assert(!source2->direct_static_selection_confirmed);
    assert(!wrapper_source_evidence(packed_sha, WrapperSourceIndex::stage_member_3).has_value());

    const auto selector0 = direct_source_selector_callsite(canonical_sha, 0U);
    const auto selector1 = direct_source_selector_callsite(canonical_sha, 1U);
    const auto selector2 = direct_source_selector_callsite(canonical_sha, 2U);
    const auto selector3 = direct_source_selector_callsite(canonical_sha, 3U);
    assert(selector0.has_value() && selector0->callsite_va == 0x140056832ULL);
    assert(selector0->requested_source == WrapperSourceIndex::stage_member_6);
    assert(selector1.has_value() && selector1->callsite_va == 0x14005686EULL);
    assert(selector1->requested_source == WrapperSourceIndex::stage_member_3);
    assert(selector2.has_value() && selector2->callsite_va == 0x1400568F0ULL);
    assert(selector2->requested_source == WrapperSourceIndex::stage_member_6);
    assert(selector3.has_value() && selector3->callsite_va == 0x140056936ULL);
    assert(selector3->requested_source == WrapperSourceIndex::stage_member_3);
    assert(!direct_source_selector_callsite(canonical_sha, 4U).has_value());
    assert(!direct_source_selector_callsite(packed_sha, 0U).has_value());

    const auto path0 = temporary_source1_query_path(canonical_sha, 0U);
    const auto path1 = temporary_source1_query_path(canonical_sha, 1U);
    assert(path0.has_value());
    assert(path0->select_callsite_va == 0x140056832ULL);
    assert(path0->query_variant == QueryVariant::positional_correction_0601e0);
    assert(path0->restore_callsite_va == 0x14005686EULL);
    assert(path1.has_value());
    assert(path1->select_callsite_va == 0x1400568F0ULL);
    assert(path1->query_variant == QueryVariant::source_selectable_segment_05fec0);
    assert(path1->restore_callsite_va == 0x140056936ULL);
    assert(!temporary_source1_query_path(packed_sha, 0U).has_value());

    const auto selector =
        runtime_helper_evidence(canonical_sha, RuntimeHelperObservedRole::source_selector);
    assert(selector.has_value());
    assert(selector->function_va == 0x14005EBC0ULL);
    assert(selector->static_call_count == 4U);

    const auto broadphase = runtime_helper_evidence(
        canonical_sha, RuntimeHelperObservedRole::candidate_cell_collector);
    assert(broadphase.has_value());
    assert(broadphase->function_va == 0x1402D2A10ULL);
    assert(broadphase->static_call_count == 7U);

    const auto record_resolver =
        runtime_helper_evidence(canonical_sha, RuntimeHelperObservedRole::record_resolver);
    assert(record_resolver.has_value());
    assert(record_resolver->function_va == 0x1402D3050ULL);
    assert(record_resolver->static_call_count == 13U);
    assert(!runtime_helper_evidence(packed_sha, RuntimeHelperObservedRole::record_resolver)
                .has_value());

    return 0;
}

#include "dmc_rengine/profiles/dmc3/hits_query_evidence.hpp"

#include <cassert>
#include <cstdint>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_evidence;

    static_assert(k_dynamic_category_bindings.size() == 6U);
    static_assert(k_query_evidence.size() == 9U);
    static_assert(k_wrapper_sources.size() == 3U);

    const auto ebe0 = query_evidence(QueryVariant::inout_correction_05ebe0);
    assert(ebe0.has_value());
    assert(ebe0->function_va == 0x14005EBE0ULL);
    assert(ebe0->success_observed_in_al);
    assert(ebe0->mutable_16_byte_inout_confirmed);
    assert(!ebe0->object_f0_argument_confirmed);

    const auto f070 = query_evidence(QueryVariant::inout_correction_extended_05f070);
    assert(f070.has_value());
    assert(f070->function_va == 0x14005F070ULL);
    assert(f070->abi_kind == SpecializedAbiKind::mutable_vec4_inout_plus_aux);
    assert(f070->object_f0_argument_confirmed);

    const auto ee40 = query_evidence(QueryVariant::bool_observed_05ee40);
    assert(ee40.has_value());
    assert(ee40->success_observed_in_al);
    assert(!ee40->mutable_16_byte_inout_confirmed);

    const auto e7a0 = query_evidence(QueryVariant::generic_combined_05e7a0);
    assert(e7a0.has_value());
    assert(e7a0->abi_kind == SpecializedAbiKind::unresolved);
    assert(!e7a0->mutable_16_byte_inout_confirmed);

    const std::uint32_t expected_categories[] = {0x02U, 0x05U, 0x08U, 0x0BU, 0x0EU, 0x11U};
    const std::uint16_t expected_masks[] = {0x0040U, 0x0002U, 0x0010U, 0x0020U, 0x0000U, 0x0000U};
    const std::uint32_t expected_flags[] = {0x1000U, 0x2000U, 0x4000U, 0x8000U, 0x10000U, 0x20000U};
    const std::uint32_t expected_offsets[] = {0x10U, 0x28U, 0x40U, 0x58U, 0x70U, 0x88U};

    for (std::size_t i = 0; i < k_dynamic_category_bindings.size(); ++i) {
        const auto& binding = k_dynamic_category_bindings[i];
        assert(binding.category_id == expected_categories[i]);
        assert(binding.static_hits_reject_mask == expected_masks[i]);
        assert(binding.activation_flag == expected_flags[i]);
        assert(binding.manager_field_offset == expected_offsets[i]);

        const auto by_category = dynamic_category_binding(binding.category_id);
        assert(by_category.has_value());
        assert(by_category->activation_flag == binding.activation_flag);

        const auto by_activation = dynamic_category_binding_from_activation(binding.activation_flag);
        assert(by_activation.has_value());
        assert(by_activation->category_id == binding.category_id);
    }

    assert(!dynamic_category_binding(0x03U).has_value());
    assert(!dynamic_category_binding_from_activation(0x40000U).has_value());

    assert(k_wrapper_sources[0].source == WrapperSourceIndex::stage_member_3);
    assert(k_wrapper_sources[0].stage_pac_backed_confirmed);
    assert(k_wrapper_sources[0].direct_static_selection_confirmed);

    assert(k_wrapper_sources[1].source == WrapperSourceIndex::stage_member_6);
    assert(k_wrapper_sources[1].stage_pac_backed_confirmed);
    assert(k_wrapper_sources[1].direct_static_selection_confirmed);

    assert(k_wrapper_sources[2].source == WrapperSourceIndex::external_global_source);
    assert(k_wrapper_sources[2].structurally_present);
    assert(!k_wrapper_sources[2].stage_pac_backed_confirmed);
    assert(!k_wrapper_sources[2].direct_static_selection_confirmed);

    return 0;
}

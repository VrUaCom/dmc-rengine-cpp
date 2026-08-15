#include "dmc_rengine/profiles/dmc3/hits_primitive_descriptor_ownership_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_primitive_descriptor_ownership_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    const auto table = manager_descriptor_table_evidence(canonical_sha);
    assert(table.has_value());
    assert(table->manager_source_initializer_va == 0x14005C260ULL);
    assert(table->entry_table_arg_index == 2U);
    assert(table->descriptor_table_arg_index == 3U);
    assert(table->manager_entry_table_pointer_offset == 0x108U);
    assert(table->manager_descriptor_table_pointer_offset == 0x110U);
    assert(table->entry_stride_bytes == 4U);
    assert(table->entry_flags_offset == 0U);
    assert(table->entry_transform_selector_offset == 1U);
    assert(table->entry_descriptor_index_offset == 2U);
    assert(table->entry_descriptor_index_width_bytes == 2U);
    assert(table->descriptor_stride_bytes == 0x50U);

    const auto ownership = runtime_descriptor_ownership_evidence(canonical_sha);
    assert(ownership.has_value());
    assert(ownership->runtime_object_initializer_va == 0x14005C8D0ULL);
    assert(ownership->entry_arg_index == 2U);
    assert(ownership->primitive_descriptor_arg_index == 3U);
    assert(ownership->runtime_object_arg_index == 4U);
    assert(ownership->transform_pointer_arg_index == 5U);
    assert(ownership->runtime_entry_pointer_offset == 0x110U);
    assert(ownership->runtime_primitive_descriptor_pointer_offset == 0x118U);
    assert(ownership->runtime_transform_pointer_offset == 0x20U);
    assert(ownership->constructor_dispatcher_va == 0x1402CC530ULL);
    assert(ownership->constructor_reads_transform_from_runtime_offset_0x20);
    assert(ownership->constructor_reads_descriptor_from_runtime_offset_0x118);
    assert(ownership->descriptor_first_byte_dispatches_types_0_through_6);

    const auto resolution = descriptor_resolution_evidence(canonical_sha);
    assert(resolution.has_value());
    assert(resolution->indirect_transform_builder_va == 0x14005C630ULL);
    assert(resolution->inline_transform_builder_va == 0x14005C740ULL);
    assert(resolution->manager_entry_table_pointer_offset == 0x108U);
    assert(resolution->manager_descriptor_table_pointer_offset == 0x110U);
    assert(resolution->entry_stride_bytes == 4U);
    assert(resolution->descriptor_index_offset == 2U);
    assert(resolution->descriptor_stride_bytes == 0x50U);
    assert(resolution->transform_selector_offset == 1U);
    assert(resolution->inline_transform_stride_bytes == 0x40U);
    assert(resolution->both_builders_resolve_same_primitive_descriptor_table);
    assert(resolution->transform_source_is_separate_from_primitive_descriptor);

    const auto init_body =
        canonical_function_body(canonical_sha, DescriptorOwnershipBodyRole::manager_source_initializer);
    assert(init_body.has_value());
    assert(init_body->begin_va == 0x14005C260ULL);
    assert(init_body->end_va == 0x14005C318ULL);
    assert(init_body->size_bytes == 184U);
    assert(init_body->sha256 ==
           "9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7");

    const auto runtime_init =
        canonical_function_body(canonical_sha, DescriptorOwnershipBodyRole::runtime_object_initializer);
    assert(runtime_init.has_value());
    assert(runtime_init->begin_va == 0x14005C8D0ULL);
    assert(runtime_init->end_va == 0x14005C99DULL);
    assert(runtime_init->size_bytes == 205U);
    assert(runtime_init->sha256 ==
           "f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe");

    assert(!manager_descriptor_table_evidence(packed_sha).has_value());
    assert(!runtime_descriptor_ownership_evidence(packed_sha).has_value());
    assert(!descriptor_resolution_evidence(packed_sha).has_value());
    assert(!canonical_function_body(packed_sha, DescriptorOwnershipBodyRole::runtime_object_initializer)
                .has_value());

    return 0;
}

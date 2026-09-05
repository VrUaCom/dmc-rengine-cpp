#pragma once

#include <cstddef>

namespace dmc::rengine::formats::model_family {

// Evidence-backed document-shell subset shared by the recovered DMC3-HD
// SCM and MOD resources. Semantic decoding of +0x13/+0x14 remains
// adapter-specific; the offsets themselves and the common initialization
// path are executable-confirmed.
struct DocumentCoreAbi final {
    static constexpr std::size_t header_size = 0x40U;
    static constexpr std::size_t version_field = 0x04U;
    static constexpr std::size_t outer_count_field = 0x10U;
    static constexpr std::size_t node_domain_count_field = 0x11U;

    // Serialized texture-slot-domain count/mirror. The shared runtime manager
    // initializer 0x1402F9570 takes live texture-table authority from the
    // external companion instead of trusting this byte.
    static constexpr std::size_t texture_slot_count_field = 0x12U;

    // Runtime-carried, semantic name unresolved across the family.
    static constexpr std::size_t runtime_mode_byte_field = 0x13U;
    static constexpr std::size_t runtime_metadata_u32_field = 0x14U;

    static constexpr std::size_t node_domain_block_field = 0x20U;
    static constexpr std::size_t outer_table_offset = 0x40U;
};

static_assert(DocumentCoreAbi::header_size == 0x40U);
static_assert(DocumentCoreAbi::outer_count_field == 0x10U);
static_assert(DocumentCoreAbi::node_domain_count_field == 0x11U);
static_assert(DocumentCoreAbi::node_domain_block_field == 0x20U);
static_assert(DocumentCoreAbi::outer_table_offset == 0x40U);

} // namespace dmc::rengine::formats::model_family

#include "dmc_rengine/formats/mod.hpp"

namespace dmc::rengine::formats::mod {
namespace {

constexpr Header header_with_default_joint{
    .version = 1.0F,
    .outer_record_count = 1U,
    .transform_domain_count = 8U,
    .texture_slot_count = 2U,
    .runtime_mode_byte = 5U,
    .runtime_metadata_u32 = 0x12345678U,
    .document_offset = 0x40U,
};

static_assert(header_with_default_joint.default_joint_index() == 5U);
static_assert(header_with_default_joint.runtime_metadata_u32 == 0x12345678U);

} // namespace
} // namespace dmc::rengine::formats::mod

#include "dmc_rengine/analysis/mod/object_runtime.hpp"

#include <array>

namespace dmc::rengine::analysis::mod {
namespace {

constexpr auto source_fields = [] {
    std::array<std::byte, 0x40U> bytes{};
    // f32 12.5 == 0x41480000 little-endian at serialized object +0x18.
    bytes[0x18U] = std::byte{0x00U};
    bytes[0x19U] = std::byte{0x00U};
    bytes[0x1AU] = std::byte{0x48U};
    bytes[0x1BU] = std::byte{0x41U};
    // u32 0xDEADBEEF little-endian at serialized object +0x1C.
    bytes[0x1CU] = std::byte{0xEFU};
    bytes[0x1DU] = std::byte{0xBEU};
    bytes[0x1EU] = std::byte{0xADU};
    bytes[0x1FU] = std::byte{0xDEU};
    return decode_object_runtime_source_fields(bytes, 0U);
}();

static_assert(source_fields.has_value());
static_assert(source_fields->raw_parameter18_f32 == 12.5F);
static_assert(source_fields->raw_parameter1c_u32 == 0xDEADBEEFU);

constexpr std::uint32_t composite_source_flags =
    0x03000000U | // high mode 3 -> encoded index 2
    0x00040000U | // runtime bit4
    0x00020000U | // runtime bit9 + xyz 128 preset
    0x00010000U | // runtime bit7
    0x00004000U | // nearest texture filtering in shared material helper
    0x00001000U | // MOD-specific runtime bytes 05/06 -> 0x0C
    0x00000400U | // runtime bit13 + conditional parameter copy
    0x00000200U | // runtime bit12 + conditional parameter copy
    0x00000020U | // runtime bit10
    0x00000010U | // runtime bit11
    0x00000005U;  // low mode 5 -> runtime bit8 + forced control 0x80

constexpr auto projection = project_object_runtime(
    composite_source_flags,
    ObjectRuntimeSourceFields{12.5F, 0xDEADBEEFU});

static_assert(projection.runtime_flags == 0x0000BF93U);
static_assert(projection.low_mode_active);
static_assert(projection.low_mode == 5U);
static_assert(projection.forces_alpha_control_0x80);
static_assert(projection.high_mode_active);
static_assert(projection.high_mode_index == 2U);
static_assert(projection.requests_manager_global_bit21);
static_assert(projection.copies_conditional_render_parameters);
static_assert(projection.raw_parameter18_f32 == 12.5F);
static_assert(projection.raw_parameter1c_u32 == 0xDEADBEEFU);
static_assert(projection.seeds_xyz_128_preset);
static_assert(projection.nearest_texture_filter);
static_assert(projection.mod_runtime_byte05 == 0x0CU);
static_assert(projection.mod_runtime_byte06 == 0x0CU);

constexpr auto baseline = project_object_runtime(0U);
static_assert(baseline.runtime_flags == runtime_flag_baseline);
static_assert(!baseline.low_mode_active);
static_assert(!baseline.high_mode_active);
static_assert(!baseline.copies_conditional_render_parameters);
static_assert(!baseline.seeds_xyz_128_preset);
static_assert(!baseline.nearest_texture_filter);
static_assert(baseline.mod_runtime_byte05 == 4U);
static_assert(baseline.mod_runtime_byte06 == 4U);

} // namespace
} // namespace dmc::rengine::analysis::mod

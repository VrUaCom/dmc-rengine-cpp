#pragma once

#include <cstddef>

namespace dmc::rengine::formats::model_family {

// Evidence-backed 0x40 outer/object record subset shared by DMC3-HD SCM and
// MOD. The two format-specific runtime initializers (0x140302F10 for SCM and
// 0x1403029E0 for MOD/EFM) copy these fields into homologous runtime state.
struct ObjectCoreAbi final {
    static constexpr std::size_t record_size = 0x40U;

    static constexpr std::size_t child_mesh_count_field = 0x00U;

    // Shared alpha/control byte. Both SCM and MOD/EFM initializers copy +0x01
    // to runtime +0x07 and feed the same base <=0x80 / >0x80 control state.
    // SCM additionally contains narrow resource-specific compatibility fixes.
    static constexpr std::size_t alpha_control_field = 0x01U;

    static constexpr std::size_t aggregate_element_count_field = 0x02U;
    static constexpr std::size_t child_mesh_table_field = 0x08U;

    // Source flags are copied verbatim to runtime object +0x10/+0x14 in both
    // SCM and MOD/EFM object initializers. Individual bit semantics may still
    // be family/format specialized.
    static constexpr std::size_t source_flags_field = 0x10U;

    // Shared vec3 center + f32 radius bounding sphere.
    static constexpr std::size_t bounding_center_field = 0x30U;
    static constexpr std::size_t bounding_radius_field = 0x3CU;
};

static_assert(ObjectCoreAbi::record_size == 0x40U);
static_assert(ObjectCoreAbi::child_mesh_count_field == 0x00U);
static_assert(ObjectCoreAbi::aggregate_element_count_field == 0x02U);
static_assert(ObjectCoreAbi::child_mesh_table_field == 0x08U);
static_assert(ObjectCoreAbi::source_flags_field == 0x10U);
static_assert(ObjectCoreAbi::bounding_center_field == 0x30U);
static_assert(ObjectCoreAbi::bounding_radius_field == 0x3CU);

} // namespace dmc::rengine::formats::model_family

#pragma once

#include <cstddef>

namespace dmc::rengine::formats::model_family {

// Evidence-backed common serialized subset shared by the recovered DMC3-HD
// SCM mesh record and MOD inner-mesh record. Only offsets/stream shapes that
// match in both formats belong here; format-specific lanes stay in adapters.
struct MeshCoreAbi final {
    static constexpr std::size_t record_size = 0x50U;

    static constexpr std::size_t element_count_field = 0x00U;
    static constexpr std::size_t positions_field = 0x10U;
    static constexpr std::size_t normals_field = 0x18U;
    static constexpr std::size_t uv_field = 0x20U;
    static constexpr std::size_t topology_workspace_field = 0x40U;
    static constexpr std::size_t generated_topology_count_field = 0x48U;

    static constexpr std::size_t position_stride = 12U; // float3
    static constexpr std::size_t normal_stride = 12U;   // float3
    static constexpr std::size_t uv_stride = 4U;        // int16x2
    static constexpr float uv_fixed_scale = 4096.0F;
};

static_assert(MeshCoreAbi::record_size == 0x50U);
static_assert(MeshCoreAbi::positions_field == 0x10U);
static_assert(MeshCoreAbi::normals_field == 0x18U);
static_assert(MeshCoreAbi::uv_field == 0x20U);
static_assert(MeshCoreAbi::topology_workspace_field == 0x40U);
static_assert(MeshCoreAbi::generated_topology_count_field == 0x48U);

} // namespace dmc::rengine::formats::model_family

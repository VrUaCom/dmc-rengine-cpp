#pragma once

#include <cstddef>

namespace dmc::rengine::formats::model_family {

// Evidence-backed common serialized subset shared by the recovered DMC3-HD
// SCM mesh record and MOD inner-mesh record. The canonical runtime converges
// on generic material helper 0x1402F9890 after format-specific mesh builders,
// so texture-slot and legacy GS CLAMP fields are now part of the proven common
// mesh ABI as well as position/normal/UV/topology-workspace fields.
struct MeshCoreAbi final {
    static constexpr std::size_t record_size = 0x50U;

    static constexpr std::size_t element_count_field = 0x00U;
    static constexpr std::size_t texture_slot_field = 0x02U;

    // Legacy PS2 GS CLAMP REGION_REPEAT parameters. 0x1402F9890 reads these
    // four u16 values from the raw serialized mesh for both MOD/EFM and SCM
    // runtime paths and packs them into the same 64-bit descriptor state.
    static constexpr std::size_t gs_clamp_min_u_field = 0x04U;
    static constexpr std::size_t gs_clamp_max_u_field = 0x06U;
    static constexpr std::size_t gs_clamp_min_v_field = 0x08U;
    static constexpr std::size_t gs_clamp_max_v_field = 0x0AU;

    static constexpr std::size_t positions_field = 0x10U;
    static constexpr std::size_t normals_field = 0x18U;
    static constexpr std::size_t uv_field = 0x20U;
    static constexpr std::size_t topology_workspace_field = 0x40U;
    static constexpr std::size_t generated_topology_count_field = 0x48U;

    static constexpr std::size_t position_stride = 12U; // float3
    static constexpr std::size_t normal_stride = 12U;   // float3
    static constexpr std::size_t uv_stride = 4U;        // int16x2
    static constexpr std::size_t stream_alignment = 0x10U;
    static constexpr float uv_fixed_scale = 4096.0F;
};

static_assert(MeshCoreAbi::record_size == 0x50U);
static_assert(MeshCoreAbi::element_count_field == 0x00U);
static_assert(MeshCoreAbi::texture_slot_field == 0x02U);
static_assert(MeshCoreAbi::gs_clamp_min_u_field == 0x04U);
static_assert(MeshCoreAbi::gs_clamp_max_v_field == 0x0AU);
static_assert(MeshCoreAbi::positions_field == 0x10U);
static_assert(MeshCoreAbi::normals_field == 0x18U);
static_assert(MeshCoreAbi::uv_field == 0x20U);
static_assert(MeshCoreAbi::topology_workspace_field == 0x40U);
static_assert(MeshCoreAbi::generated_topology_count_field == 0x48U);
static_assert(MeshCoreAbi::stream_alignment == 0x10U);

} // namespace dmc::rengine::formats::model_family

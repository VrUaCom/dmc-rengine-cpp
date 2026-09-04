#include "dmc_rengine/formats/model_family.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"

#include <cassert>

int main() {
    namespace family = dmc::rengine::formats::model_family;

    constexpr auto scm = family::scm_profile();
    static_assert(scm.source_format == family::SourceFormat::scm);
    static_assert(family::has(scm.capabilities, family::Capability::geometry));
    static_assert(family::has(scm.capabilities, family::Capability::uv));
    static_assert(family::has(scm.capabilities, family::Capability::node_hierarchy));
    static_assert(family::has(scm.capabilities, family::Capability::alpha_control));
    static_assert(family::has(scm.capabilities, family::Capability::legacy_gs_sampler));
    static_assert(!family::has(scm.capabilities, family::Capability::skeletal_skinning));
    static_assert(family::has(scm.capabilities, family::Capability::experimental_authoring));
    static_assert(scm.max_serialized_skin_influences == 0U);
    static_assert(!scm.production_writer_authorized);

    constexpr auto mod = family::mod_profile();
    static_assert(mod.source_format == family::SourceFormat::mod);
    static_assert(family::has(mod.capabilities, family::Capability::geometry));
    static_assert(family::has(mod.capabilities, family::Capability::uv));
    static_assert(family::has(mod.capabilities, family::Capability::node_hierarchy));
    static_assert(family::has(mod.capabilities, family::Capability::skeletal_skinning));
    static_assert(!family::has(mod.capabilities, family::Capability::alpha_control));
    static_assert(!family::has(mod.capabilities, family::Capability::experimental_authoring));
    static_assert(mod.max_serialized_skin_influences == 3U);
    static_assert(!mod.production_writer_authorized);

    static_assert(family::MeshCoreAbi::record_size == 0x50U);
    static_assert(family::MeshCoreAbi::element_count_field == 0x00U);
    static_assert(family::MeshCoreAbi::positions_field == 0x10U);
    static_assert(family::MeshCoreAbi::normals_field == 0x18U);
    static_assert(family::MeshCoreAbi::uv_field == 0x20U);
    static_assert(family::MeshCoreAbi::topology_workspace_field == 0x40U);
    static_assert(family::MeshCoreAbi::generated_topology_count_field == 0x48U);
    static_assert(family::MeshCoreAbi::position_stride == 12U);
    static_assert(family::MeshCoreAbi::normal_stride == 12U);
    static_assert(family::MeshCoreAbi::uv_stride == 4U);
    static_assert(family::MeshCoreAbi::uv_fixed_scale == 4096.0F);

    assert(family::to_string(scm.source_format) == "scm");
    assert(family::to_string(mod.source_format) == "mod");
    return 0;
}

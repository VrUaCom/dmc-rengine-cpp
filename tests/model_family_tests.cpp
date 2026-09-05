#include "dmc_rengine/formats/model_document_core.hpp"
#include "dmc_rengine/formats/model_family.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"
#include "dmc_rengine/formats/model_node_domain_core.hpp"
#include "dmc_rengine/formats/model_object_core.hpp"

#include <cassert>

int main() {
    namespace family = dmc::rengine::formats::model_family;

    constexpr auto scm = family::scm_profile();
    static_assert(scm.source_format == family::SourceFormat::scm);
    static_assert(family::has(scm.capabilities, family::Capability::geometry));
    static_assert(family::has(scm.capabilities, family::Capability::uv));
    static_assert(family::has(scm.capabilities, family::Capability::node_hierarchy));
    static_assert(family::has(scm.capabilities, family::Capability::texture_binding));
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
    static_assert(family::has(mod.capabilities, family::Capability::texture_binding));
    static_assert(family::has(mod.capabilities, family::Capability::alpha_control));
    static_assert(family::has(mod.capabilities, family::Capability::legacy_gs_sampler));
    static_assert(family::has(mod.capabilities, family::Capability::skeletal_skinning));
    static_assert(!family::has(mod.capabilities, family::Capability::experimental_authoring));
    static_assert(mod.max_serialized_skin_influences == 3U);
    static_assert(!mod.production_writer_authorized);

    static_assert(family::DocumentCoreAbi::header_size == 0x40U);
    static_assert(family::DocumentCoreAbi::version_field == 0x04U);
    static_assert(family::DocumentCoreAbi::outer_count_field == 0x10U);
    static_assert(family::DocumentCoreAbi::node_domain_count_field == 0x11U);
    static_assert(family::DocumentCoreAbi::texture_slot_count_field == 0x12U);
    static_assert(family::DocumentCoreAbi::runtime_mode_byte_field == 0x13U);
    static_assert(family::DocumentCoreAbi::runtime_metadata_u32_field == 0x14U);
    static_assert(family::DocumentCoreAbi::node_domain_block_field == 0x20U);
    static_assert(family::DocumentCoreAbi::outer_table_offset == 0x40U);

    static_assert(family::ObjectCoreAbi::record_size == 0x40U);
    static_assert(family::ObjectCoreAbi::child_mesh_count_field == 0x00U);
    static_assert(family::ObjectCoreAbi::alpha_control_field == 0x01U);
    static_assert(family::ObjectCoreAbi::aggregate_element_count_field == 0x02U);
    static_assert(family::ObjectCoreAbi::child_mesh_table_field == 0x08U);
    static_assert(family::ObjectCoreAbi::source_flags_field == 0x10U);
    static_assert(family::ObjectCoreAbi::bounding_center_field == 0x30U);
    static_assert(family::ObjectCoreAbi::bounding_radius_field == 0x3CU);

    static_assert(family::MeshCoreAbi::record_size == 0x50U);
    static_assert(family::MeshCoreAbi::element_count_field == 0x00U);
    static_assert(family::MeshCoreAbi::texture_slot_field == 0x02U);
    static_assert(family::MeshCoreAbi::gs_clamp_min_u_field == 0x04U);
    static_assert(family::MeshCoreAbi::gs_clamp_max_u_field == 0x06U);
    static_assert(family::MeshCoreAbi::gs_clamp_min_v_field == 0x08U);
    static_assert(family::MeshCoreAbi::gs_clamp_max_v_field == 0x0AU);
    static_assert(family::MeshCoreAbi::positions_field == 0x10U);
    static_assert(family::MeshCoreAbi::normals_field == 0x18U);
    static_assert(family::MeshCoreAbi::uv_field == 0x20U);
    static_assert(family::MeshCoreAbi::topology_workspace_field == 0x40U);
    static_assert(family::MeshCoreAbi::generated_topology_count_field == 0x48U);
    static_assert(family::MeshCoreAbi::position_stride == 12U);
    static_assert(family::MeshCoreAbi::normal_stride == 12U);
    static_assert(family::MeshCoreAbi::uv_stride == 4U);
    static_assert(family::MeshCoreAbi::stream_alignment == 0x10U);
    static_assert(family::MeshCoreAbi::uv_fixed_scale == 4096.0F);

    static_assert(family::NodeDomainCoreAbi::parent_rel_field == 0x00U);
    static_assert(family::NodeDomainCoreAbi::order_rel_field == 0x04U);
    static_assert(family::NodeDomainCoreAbi::adapter_array_rel_field == 0x08U);
    static_assert(family::NodeDomainCoreAbi::transform_rel_field == 0x0CU);
    static_assert(family::NodeDomainCoreAbi::expected_parent_rel(24U) == 0x20U);
    static_assert(family::NodeDomainCoreAbi::expected_order_rel(24U) == 0x38U);
    static_assert(family::NodeDomainCoreAbi::expected_adapter_array_rel(24U) == 0x50U);
    static_assert(family::NodeDomainCoreAbi::expected_transform_rel(24U) == 0x70U);
    static_assert(family::NodeDomainCoreAbi::expected_order_rel(33U) == 0x44U);
    static_assert(family::NodeDomainCoreAbi::expected_adapter_array_rel(33U) == 0x68U);
    static_assert(family::NodeDomainCoreAbi::expected_transform_rel(33U) == 0x90U);

    static_assert(family::TransformCoreAbi::record_size == 0x20U);
    static_assert(family::TransformCoreAbi::translation_field == 0x00U);
    static_assert(family::TransformCoreAbi::translation_magnitude_field == 0x0CU);
    static_assert(family::TransformCoreAbi::rotation_xyz_radians_field == 0x10U);
    static_assert(family::TransformCoreAbi::reserved1c_field == 0x1CU);

    assert(family::to_string(scm.source_format) == "scm");
    assert(family::to_string(mod.source_format) == "mod");
    return 0;
}

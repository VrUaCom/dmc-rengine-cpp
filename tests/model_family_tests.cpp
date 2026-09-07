#include "dmc_rengine/analysis/mod/texture_binding.hpp"
#include "dmc_rengine/formats/model_document_core.hpp"
#include "dmc_rengine/formats/model_family.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"
#include "dmc_rengine/formats/model_node_domain_core.hpp"
#include "dmc_rengine/formats/model_object_core.hpp"
#include "dmc_rengine/formats/model_texture_companion.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

} // namespace

int main() {
    namespace family = dmc::rengine::formats::model_family;
    namespace mod_format = dmc::rengine::formats::mod;
    namespace mod_analysis = dmc::rengine::analysis::mod;

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

    static_assert(family::ModelTextureCompanionAbi::texture_count_field == 0x000U);
    static_assert(family::ModelTextureCompanionAbi::block_count_table == 0x004U);
    static_assert(family::ModelTextureCompanionAbi::payload_base == 0x800U);
    static_assert(family::ModelTextureCompanionAbi::payload_block_size == 0x800U);
    static_assert(family::ModelTextureCompanionAbi::tm2_magic_le == 0x00324D54U);

    {
        std::vector<std::byte> bytes(0x2000U, std::byte{0});
        put_u32(bytes, 0x000U, 2U);
        put_u32(bytes, 0x004U, 1U);
        put_u32(bytes, 0x008U, 2U);
        put_u32(bytes, 0x800U, family::ModelTextureCompanionAbi::tm2_magic_le);
        put_u32(bytes, 0x1000U, family::ModelTextureCompanionAbi::tm2_magic_le);

        const auto parsed = family::parse_texture_companion(bytes);
        assert(parsed.ok());
        assert(parsed.texture_count == 2U);
        assert(parsed.entries.size() == 2U);
        assert(parsed.entries[0].index == 0U);
        assert(parsed.entries[0].block_count == 1U);
        assert(parsed.entries[0].payload_offset == 0x800U);
        assert(parsed.entries[0].allocated_size == 0x800U);
        assert(parsed.entries[1].index == 1U);
        assert(parsed.entries[1].block_count == 2U);
        assert(parsed.entries[1].payload_offset == 0x1000U);
        assert(parsed.entries[1].allocated_size == 0x1000U);

        put_u32(bytes, 0x1000U, 0U);
        const auto bad_magic = family::parse_texture_companion(bytes);
        assert(!bad_magic.ok());
        assert(bad_magic.status ==
               family::TextureCompanionStatus::tm2_magic_mismatch);

        bytes.resize(0x1800U);
        put_u32(bytes, 0x1000U, family::ModelTextureCompanionAbi::tm2_magic_le);
        const auto truncated = family::parse_texture_companion(bytes);
        assert(!truncated.ok());
        assert(truncated.status ==
               family::TextureCompanionStatus::payload_out_of_bounds);
    }

    {
        mod_format::Document document;
        document.header.texture_slot_count = 3U; // serialized mirror only

        mod_format::OuterModel outer;
        mod_format::InnerMesh mesh0;
        mesh0.texture_slot = 0U;
        mod_format::InnerMesh mesh1;
        mesh1.texture_slot = 1U;
        outer.meshes.push_back(mesh0);
        outer.meshes.push_back(mesh1);
        document.outer_models.push_back(outer);

        family::TextureCompanionParseResult companion;
        companion.status = family::TextureCompanionStatus::ok;
        companion.texture_count = 2U;

        const auto mirror_mismatch =
            mod_analysis::analyze_texture_binding(document, companion);
        assert(mirror_mismatch.companion_valid);
        assert(mirror_mismatch.header_texture_slot_count == 3U);
        assert(mirror_mismatch.companion_texture_count == 2U);
        assert(!mirror_mismatch.header_mirror_matches_companion);
        assert(mirror_mismatch.mesh_count == 2U);
        assert(mirror_mismatch.out_of_range_meshes.empty());
        assert(mirror_mismatch.runtime_bindings_valid());

        document.header.texture_slot_count = 2U;
        const auto exact_mirror =
            mod_analysis::analyze_texture_binding(document, companion);
        assert(exact_mirror.header_mirror_matches_companion);
        assert(exact_mirror.runtime_bindings_valid());

        document.outer_models[0].meshes[1].texture_slot = 2U;
        const auto out_of_range =
            mod_analysis::analyze_texture_binding(document, companion);
        assert(!out_of_range.runtime_bindings_valid());
        assert(out_of_range.out_of_range_meshes.size() == 1U);
        assert(out_of_range.out_of_range_meshes[0].outer_index == 0U);
        assert(out_of_range.out_of_range_meshes[0].mesh_index == 1U);
        assert(out_of_range.out_of_range_meshes[0].texture_slot == 2U);

        companion.status = family::TextureCompanionStatus::tm2_magic_mismatch;
        const auto invalid_companion =
            mod_analysis::analyze_texture_binding(document, companion);
        assert(!invalid_companion.companion_valid);
        assert(!invalid_companion.runtime_bindings_valid());
        assert(invalid_companion.out_of_range_meshes.empty());
    }

    assert(family::to_string(scm.source_format) == "scm");
    assert(family::to_string(mod.source_format) == "mod");
    return 0;
}

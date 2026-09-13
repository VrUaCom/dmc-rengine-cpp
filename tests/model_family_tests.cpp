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

    // The framing every preserved specimen actually uses.
    //
    // This parser required "TM2\0" at payload offset zero, and the fixture
    // above is the only thing that has ever satisfied it: every .ptx in the
    // reference corpus and in the supplied stage sets carries a 0x70 texture
    // slot descriptor with the DDS image behind it, and the first four bytes
    // of the payload are zero. So the parser refused the entire corpus, and
    // analyze_texture_binding — whose whole job is to check mesh slots against
    // the companion — answered "no valid companion" for every real model.
    //
    // The synthetic fixture is exactly why that went unnoticed. A test that
    // builds the input its parser wants proves the parser reads that input and
    // nothing about whether the input exists.
    {
        constexpr auto descriptor =
            family::ModelTextureCompanionAbi::wrapped_descriptor_size;
        std::vector<std::byte> bytes(0x2000U, std::byte{0});
        put_u32(bytes, 0x000U, 2U);
        put_u32(bytes, 0x004U, 1U);
        put_u32(bytes, 0x008U, 2U);
        put_u32(
            bytes, 0x800U + descriptor,
            family::ModelTextureCompanionAbi::dds_magic_le);
        put_u32(
            bytes, 0x1000U + descriptor,
            family::ModelTextureCompanionAbi::dds_magic_le);

        const auto parsed = family::parse_texture_companion(bytes);
        assert(parsed.ok());
        assert(parsed.texture_count == 2U);
        assert(parsed.entries.size() == 2U);
        assert(parsed.framing ==
               family::TextureCompanionFraming::descriptor_wrapped_dds);
        // The allocation still starts where it always did; the image does not.
        // A consumer reading pixels from payload_offset would read descriptor
        // bytes and call the result a corrupt texture.
        assert(parsed.entries[0].payload_offset == 0x800U);
        assert(parsed.entries[0].image_offset() == 0x800U + descriptor);
        assert(parsed.entries[1].image_offset() == 0x1000U + descriptor);
        assert(parsed.entries[0].framing ==
               family::TextureCompanionFraming::descriptor_wrapped_dds);

        // Under the TM2 framing the image is the payload, so the accessor has
        // to stay an identity there rather than adding the descriptor blindly.
        std::vector<std::byte> tm2(0x1000U, std::byte{0});
        put_u32(tm2, 0x000U, 1U);
        put_u32(tm2, 0x004U, 1U);
        put_u32(tm2, 0x800U, family::ModelTextureCompanionAbi::tm2_magic_le);
        const auto tm2_parsed = family::parse_texture_companion(tm2);
        assert(tm2_parsed.ok());
        assert(tm2_parsed.framing ==
               family::TextureCompanionFraming::tm2_at_payload_start);
        assert(tm2_parsed.entries[0].image_offset() == 0x800U);

        // A companion is one runtime table materialized one way, so payloads
        // that are individually recognized but disagree are a refusal, not a
        // per-entry detail an unsuspecting consumer would average over.
        std::vector<std::byte> mixed(0x2000U, std::byte{0});
        put_u32(mixed, 0x000U, 2U);
        put_u32(mixed, 0x004U, 1U);
        put_u32(mixed, 0x008U, 2U);
        put_u32(mixed, 0x800U, family::ModelTextureCompanionAbi::tm2_magic_le);
        put_u32(
            mixed, 0x1000U + descriptor,
            family::ModelTextureCompanionAbi::dds_magic_le);
        const auto mixed_parsed = family::parse_texture_companion(mixed);
        assert(!mixed_parsed.ok());
        assert(mixed_parsed.status ==
               family::TextureCompanionStatus::mixed_framing);

        // The status word crosses the ABI to an operator, so it has to say the
        // condition rather than the history. `tm2_magic_mismatch` kept its
        // identifier so existing consumers compile, and it no longer means
        // "not TM2" — it means neither framing was recognized. A reader shown
        // "tm2-magic-mismatch" for a bundle whose payloads are DDS-framed
        // would go looking for the wrong problem.
        assert(family::to_string(family::TextureCompanionStatus::tm2_magic_mismatch) ==
               "no-recognized-payload-framing");
        assert(family::to_string(family::TextureCompanionStatus::mixed_framing) ==
               "mixed-framing");
        assert(family::to_string(family::TextureCompanionStatus::ok) == "ok");

        // And neither signature is still a refusal: widening the contract must
        // not turn it into "anything with a block table".
        std::vector<std::byte> neither(0x1000U, std::byte{0});
        put_u32(neither, 0x000U, 1U);
        put_u32(neither, 0x004U, 1U);
        put_u32(neither, 0x800U, 0xDEADBEEFU);
        const auto refused = family::parse_texture_companion(neither);
        assert(!refused.ok());
        assert(refused.status ==
               family::TextureCompanionStatus::tm2_magic_mismatch);
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

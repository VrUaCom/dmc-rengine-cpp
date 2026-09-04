#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"
#include "dmc_rengine/profiles/dmc3/scm_resource_bundle.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace {

template <class T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

std::vector<std::byte> fixture() {
    using namespace dmc::rengine::formats::scm;

    ObjectShape shape;
    shape.mesh_vertex_counts = {3U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 1U);
    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});

    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{2};
    put<std::uint32_t>(bytes, 0x14U, 300100U);
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset =
        static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(
        bytes, object_offset + 0x08U, object_layout.mesh_table_offset);
    put<float>(bytes, object_offset + 0x3CU, 2.0F);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset =
        static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    put<float>(
        bytes,
        static_cast<std::size_t>(mesh_layout.positions_offset),
        1.0F);
    put<float>(
        bytes,
        static_cast<std::size_t>(mesh_layout.positions_offset) + 16U,
        2.0F);
    put<float>(
        bytes,
        static_cast<std::size_t>(mesh_layout.positions_offset) + 32U,
        1.5F);

    const auto scene_offset =
        static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(
        bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(
        bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};
    return bytes;
}

std::vector<std::byte> wrapped_texture_fixture() {
    constexpr std::uint32_t width = 4U;
    constexpr std::uint32_t height = 4U;
    constexpr std::uint32_t mip_count = 1U;
    constexpr std::uint32_t payload_size = 8U;

    std::vector<std::byte> dds(128U + payload_size, std::byte{0});
    dds[0] = std::byte{'D'};
    dds[1] = std::byte{'D'};
    dds[2] = std::byte{'S'};
    dds[3] = std::byte{' '};
    put<std::uint32_t>(dds, 4U, 124U);
    put<std::uint32_t>(dds, 12U, height);
    put<std::uint32_t>(dds, 16U, width);
    put<std::uint32_t>(dds, 28U, mip_count);
    dds[84U] = std::byte{'D'};
    dds[85U] = std::byte{'X'};
    dds[86U] = std::byte{'T'};
    dds[87U] = std::byte{'1'};

    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    put<std::uint32_t>(descriptor, 0x08U, 0x20000U | (mip_count << 8U) | 0x86U);
    put<std::uint32_t>(descriptor, 0x0CU, 0xAAE4U);
    put<std::uint32_t>(descriptor, 0x10U, (height << 16U) | width);
    put<std::uint32_t>(descriptor, 0x14U, 1U);
    put<std::uint32_t>(descriptor, 0x18U, width * 2U);
    put<std::uint32_t>(descriptor, 0x20U, 0x40U);
    put<std::uint32_t>(descriptor, 0x38U, payload_size);
    put<std::uint32_t>(descriptor, 0x44U, (height << 16U) | width);
    put<std::uint32_t>(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(width)));
    put<std::uint32_t>(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(height)));
    put<std::uint32_t>(descriptor, 0x60U, 0U);
    put<std::uint32_t>(
        descriptor, 0x64U, static_cast<std::uint32_t>(dds.size()));
    put<std::uint32_t>(descriptor, 0x68U, 8U);

    std::vector<std::byte> result;
    result.reserve(descriptor.size() + dds.size());
    result.insert(result.end(), descriptor.begin(), descriptor.end());
    result.insert(result.end(), dds.begin(), dds.end());
    return result;
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto source = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    assert(parsed.ok());
    assert(parsed.document.objects[0].meshes[0].positions.size() == 3U);
    assert(parsed.document.objects[0].meshes[0].normals.size() == 3U);
    assert(parsed.document.objects[0].meshes[0].uvs.size() == 3U);
    assert(parsed.document.objects[0].meshes[0].colors_topology.size() == 3U);

    const auto preserved = Writer::write(
        parsed.document, WriteMode::preserve_layout);
    assert(preserved.ok());
    assert(preserved.bit_identical_to_source);
    assert(preserved.bytes == source);

    const auto rebuilt = Writer::write(
        parsed.document, WriteMode::canonical_rebuild);
    assert(rebuilt.ok());
    assert(rebuilt.bit_identical_to_source);
    assert(rebuilt.bytes == source);

    auto edited_document = parsed.document;
    const auto position_edit = set_vertex_position(
        edited_document, 0U, 0U, 0U, Vec3f{4.0F, 0.0F, 0.0F});
    assert(position_edit.ok());
    assert(position_edit.changed);

    const auto uv_edit = set_uv(
        edited_document, 0U, 0U, 0U, 0.5F, -0.25F);
    assert(uv_edit.ok());
    assert(uv_edit.changed);

    const auto texture_edit = set_texture_slot(
        edited_document, 0U, 0U, 1U);
    assert(texture_edit.ok());
    assert(texture_edit.changed);

    const auto alpha_edit = set_alpha_control(
        edited_document, 0U, 0x40U);
    assert(alpha_edit.ok());
    assert(alpha_edit.changed);

    const auto filter_edit = set_texture_filter_nearest(
        edited_document, 0U, true);
    assert(filter_edit.ok());
    assert(filter_edit.changed);

    const auto clamp_edit = set_region_repeat(
        edited_document,
        0U,
        0U,
        LegacyGsClampRegionRepeat{1U, 2U, 3U, 4U});
    assert(clamp_edit.ok());
    assert(clamp_edit.changed);

    const auto translation_edit = set_node_translation(
        edited_document, 0U, Vec3f{3.0F, 4.0F, 0.0F});
    assert(translation_edit.ok());
    assert(translation_edit.changed);
    assert(
        edited_document.scene_nodes.transform_by_node_index[0]
            .translation_magnitude == 5.0F);

    const auto rotation_edit = set_node_rotation(
        edited_document, 0U, Vec3f{0.1F, 0.2F, 0.3F});
    assert(rotation_edit.ok());
    assert(rotation_edit.changed);

    const auto edited = Writer::write(
        edited_document, WriteMode::preserve_layout);
    assert(edited.ok());
    assert(!edited.bit_identical_to_source);
    const auto edited_parse = Parser::parse(
        std::span<const std::byte>{edited.bytes});
    assert(edited_parse.ok());
    assert(edited_parse.document.objects[0].meshes[0].positions[0].x == 4.0F);
    assert(edited_parse.document.objects[0].meshes[0].uvs[0].u == 2048);
    assert(edited_parse.document.objects[0].meshes[0].uvs[0].v == -1024);
    assert(edited_parse.document.objects[0].meshes[0].texture_index == 1U);
    assert(edited_parse.document.objects[0].alpha_control == 0x40U);
    assert(
        (edited_parse.document.objects[0].flags &
         object_flag_nearest_texture_filter) != 0U);
    assert(
        edited_parse.document.objects[0].meshes[0]
            .gs_clamp_region_repeat.min_u == 1U);
    assert(edited_parse.document.objects[0].bounding_radius == 4.0F);
    assert(
        edited_parse.document.scene_nodes.transform_by_node_index[0]
            .translation_magnitude == 5.0F);

    auto invalid_edit_document = parsed.document;
    const auto invalid_uv = set_uv(
        invalid_edit_document, 0U, 0U, 0U, 100.0F, 0.0F);
    assert(!invalid_uv.ok());
    const auto invalid_texture = set_texture_slot(
        invalid_edit_document, 0U, 0U, 2U);
    assert(!invalid_texture.ok());
    const auto invalid_clamp = set_region_repeat(
        invalid_edit_document,
        0U,
        0U,
        LegacyGsClampRegionRepeat{0x400U, 0U, 0U, 0U});
    assert(!invalid_clamp.ok());

    auto resized_document = parsed.document;
    auto& mesh = resized_document.objects[0].meshes[0];
    mesh.positions.push_back(Vec3f{1.0F, 1.0F, 1.0F});
    mesh.normals.push_back(Vec3f{0.0F, 1.0F, 0.0F});
    mesh.uvs.push_back(SerializedUv{0, 0});
    mesh.colors_topology.push_back(ColorTopology{255U, 255U, 255U, 0U});

    const auto rejected_preserve = Writer::write(
        resized_document, WriteMode::preserve_layout);
    assert(!rejected_preserve.ok());

    const auto resized = Writer::write(
        resized_document, WriteMode::canonical_rebuild);
    assert(resized.ok());
    const auto resized_parse = Parser::parse(
        std::span<const std::byte>{resized.bytes});
    assert(resized_parse.ok());
    assert(resized_parse.document.objects[0].meshes[0].vertex_count == 4U);
    assert(resized_parse.document.objects[0].total_vertex_count == 4U);
    assert(resized.bytes.size() != source.size());

    const auto texture_source = wrapped_texture_fixture();
    const auto texture_parse = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{texture_source});
    assert(texture_parse.ok());
    assert(texture_parse.document.textures.size() == 1U);

    auto bundle_document = parsed.document;
    bundle_document.header.texture_slot_count = 1U;
    bundle_document.objects[0].meshes[0].texture_index = 0U;
    dmc3::ScmResourceBundle bundle{
        .scm = bundle_document,
        .texture_companion_source = texture_source,
    };
    const auto bundle_write = dmc3::ScmResourceBundleWriter::write(
        bundle, WriteMode::preserve_layout);
    assert(bundle_write.ok());
    assert(bundle_write.texture_companion_bytes == texture_source);
    const auto bundle_scm_parse = Parser::parse(
        std::span<const std::byte>{bundle_write.scm_bytes});
    assert(bundle_scm_parse.ok());
    assert(bundle_scm_parse.document.header.texture_slot_count == 1U);

    auto bad_count_bundle = bundle;
    bad_count_bundle.scm.header.texture_slot_count = 2U;
    const auto bad_count = dmc3::ScmResourceBundleWriter::write(
        bad_count_bundle, WriteMode::preserve_layout);
    assert(!bad_count.ok());
    assert(
        bad_count.status ==
        dmc3::ScmResourceBundleStatus::texture_count_mismatch);

    auto bad_index_bundle = bundle;
    bad_index_bundle.scm.objects[0].meshes[0].texture_index = 1U;
    const auto bad_index = dmc3::ScmResourceBundleWriter::write(
        bad_index_bundle, WriteMode::preserve_layout);
    assert(!bad_index.ok());
    assert(
        bad_index.status ==
        dmc3::ScmResourceBundleStatus::texture_index_out_of_range);

    return 0;
}

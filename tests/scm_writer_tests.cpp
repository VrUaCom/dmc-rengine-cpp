#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"

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
    bytes[0x12U] = std::byte{1};
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

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

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
    edited_document.objects[0].meshes[0].positions[0].x = 4.0F;
    const auto edited = Writer::write(
        edited_document, WriteMode::preserve_layout);
    assert(edited.ok());
    assert(!edited.bit_identical_to_source);
    const auto edited_parse = Parser::parse(
        std::span<const std::byte>{edited.bytes});
    assert(edited_parse.ok());
    assert(edited_parse.document.objects[0].meshes[0].positions[0].x == 4.0F);
    assert(edited_parse.document.objects[0].bounding_radius == 4.0F);

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

    return 0;
}

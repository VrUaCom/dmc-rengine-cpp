#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_reflow_edit.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
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
    put<float>(bytes, object_offset + 0x3CU, 4.0F);

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

    for (std::size_t index = 0U; index < 3U; ++index) {
        const auto p = static_cast<std::size_t>(mesh_layout.positions_offset) +
                       index * 12U;
        put<float>(bytes, p + 0U, static_cast<float>(index + 1U));
        put<float>(bytes, p + 4U, static_cast<float>(index));
        put<float>(bytes, p + 8U, 0.0F);

        const auto n = static_cast<std::size_t>(mesh_layout.normals_offset) +
                       index * 12U;
        put<float>(bytes, n + 0U, 0.0F);
        put<float>(bytes, n + 4U, 1.0F);
        put<float>(bytes, n + 8U, 0.0F);

        const auto color =
            static_cast<std::size_t>(mesh_layout.color_flags_offset) +
            index * 4U;
        bytes[color + 0U] = std::byte{0x10};
        bytes[color + 1U] = std::byte{0x20};
        bytes[color + 2U] = std::byte{0x30};
        bytes[color + 3U] = std::byte{0};
    }

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

std::size_t triangle_count(const dmc::rengine::formats::scm::Mesh& mesh) {
    using namespace dmc::rengine::formats::scm;
    std::vector<std::uint8_t> flags;
    flags.reserve(mesh.colors_topology.size());
    for (const auto& value : mesh.colors_topology) {
        flags.push_back(value.topology_flags);
    }
    const auto indices = generate_triangle_strip_indices(flags);
    std::size_t count = 0U;
    for (std::size_t index = 2U; index < indices.size(); ++index) {
        const auto a = indices[index - 2U];
        const auto b = indices[index - 1U];
        const auto c = indices[index];
        if (a != b && b != c && a != c) ++count;
    }
    return count;
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

    const auto source = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    assert(parsed.ok());
    assert(parsed.document.objects.size() == 1U);
    assert(parsed.document.objects[0].meshes.size() == 1U);
    assert(parsed.document.objects[0].meshes[0].vertex_count == 3U);
    assert(triangle_count(parsed.document.objects[0].meshes[0]) == 1U);

    auto document = parsed.document;
    const auto edit = append_break_vertex_copy(document, 0U, 0U, 2U);
    assert(edit.ok());
    assert(edit.changed);
    assert(document.objects[0].meshes[0].positions.size() == 4U);
    assert(
        document.objects[0].meshes[0].colors_topology.back().topology_flags ==
        triangle_break_bit);

    const auto preserve = Writer::write(document, WriteMode::preserve_layout);
    assert(!preserve.ok());

    const auto rebuilt = Writer::write(document, WriteMode::canonical_rebuild);
    assert(rebuilt.ok());
    assert(rebuilt.bytes.size() > source.size());
    assert(!rebuilt.bit_identical_to_source);

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{rebuilt.bytes});
    assert(reparsed.ok());
    const auto& mesh = reparsed.document.objects[0].meshes[0];
    assert(mesh.vertex_count == 4U);
    assert(reparsed.document.objects[0].total_vertex_count == 4U);
    assert(mesh.positions.back().x == parsed.document.objects[0].meshes[0].positions[2].x);
    assert(mesh.positions.back().y == parsed.document.objects[0].meshes[0].positions[2].y);
    assert(mesh.positions.back().z == parsed.document.objects[0].meshes[0].positions[2].z);
    assert(mesh.colors_topology.back().topology_flags == triangle_break_bit);
    assert(triangle_count(mesh) == 1U);

    auto invalid = parsed.document;
    const auto invalid_edit = append_break_vertex_copy(invalid, 0U, 0U, 3U);
    assert(!invalid_edit.ok());
    assert(!invalid_edit.changed);

    return 0;
}

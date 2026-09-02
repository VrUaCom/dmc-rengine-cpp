#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace {

template <typename T>
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
        static_cast<std::size_t>(layout.file_size),
        std::byte{0});

    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put<std::uint64_t>(
        bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(
        bytes, object_offset + 0x08U, object_layout.mesh_table_offset);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint16_t>(bytes, mesh_offset + 0x02U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x28U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    const auto scene_offset = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};
    return bytes;
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

    static_assert(header_size == 0x40U);
    static_assert(object_record_size == 0x40U);
    static_assert(mesh_record_size == 0x50U);
    static_assert(index_workspace_capacity_bytes(3U) == 16U);
    static_assert(index_workspace_capacity_bytes(10U) == 48U);

    ObjectShape shape;
    shape.mesh_vertex_counts = {3U, 4U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 3U);
    assert(layout.objects.size() == 1U);
    assert(layout.objects[0].meshes.size() == 2U);
    assert(layout.objects[0].meshes[1].record_offset ==
        layout.objects[0].meshes[0].record_offset + 0x50U);
    assert(layout.scene.parent_rel == 0x20U);
    assert(layout.scene.order_rel == 0x24U);
    assert(layout.scene.object_binding_rel == 0x28U);
    assert(layout.scene.transform_rel == 0x30U);

    const std::vector<std::uint8_t> topology{
        0U, 0U, 0U, 0U, triangle_break_bit, 0U, 0U};
    const auto indices = generate_triangle_strip_indices(topology);
    const std::vector<std::uint16_t> expected{
        0U, 1U, 2U, 3U, 3U, 3U, 3U, 4U, 5U, 6U};
    assert(indices == expected);

    const auto bytes = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{bytes});
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.header.object_count == 1U);
    assert(parsed.document.header.scene_node_count == 1U);
    assert(parsed.document.objects.size() == 1U);
    assert(parsed.document.objects[0].meshes.size() == 1U);
    assert(parsed.document.objects[0].meshes[0].vertex_count == 3U);
    assert(parsed.document.objects[0].meshes[0].observed_topology_flag_mask == 0U);
    assert(parsed.document.scene_nodes.parents[0] == -1);
    assert(parsed.document.scene_nodes.order[0] == 0U);
    assert(parsed.document.scene_nodes.object_bindings[0] == 0);

    auto bad_vertex_sum = bytes;
    put<std::uint16_t>(bad_vertex_sum, 0x42U, 4U);
    const auto bad_sum = Parser::parse(
        std::span<const std::byte>{bad_vertex_sum});
    assert(bad_sum.recognized);
    assert(!bad_sum.ok());

    auto bad_continuation = bytes;
    const auto mesh_offset = static_cast<std::size_t>(
        parsed.document.objects[0].meshes[0].record_offset);
    put<std::uint64_t>(bad_continuation, mesh_offset + 0x28U, 0x50U);
    const auto bad_cont = Parser::parse(
        std::span<const std::byte>{bad_continuation});
    assert(bad_cont.recognized);
    assert(!bad_cont.ok());

    std::vector<std::byte> other(header_size, std::byte{0});
    const auto unrecognized = Parser::parse(
        std::span<const std::byte>{other});
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    const std::vector<std::byte> truncated(0x10U, std::byte{0});
    const auto short_result = Parser::parse(
        std::span<const std::byte>{truncated});
    assert(!short_result.recognized);
    assert(!short_result.ok());
    assert(short_result.diagnostics[0].code == "scm.truncated-header");

    return 0;
}

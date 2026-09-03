#include "dmc_rengine/formats/scm_layout.hpp"

namespace dmc::rengine::formats::scm {
namespace {

constexpr std::uint64_t header_size = 0x40U;
constexpr std::uint64_t object_record_size = 0x40U;
constexpr std::uint64_t mesh_record_size = 0x50U;
constexpr std::uint64_t scene_transform_size = 0x20U;

[[nodiscard]] std::uint64_t add_stream(
    std::uint64_t cursor,
    std::uint16_t vertex_count,
    std::uint64_t stride) noexcept {
    return static_cast<std::uint64_t>(align16(
        static_cast<std::size_t>(
            cursor + static_cast<std::uint64_t>(vertex_count) * stride)));
}

} // namespace

SerializedLayout build_serialized_layout(
    std::span<const ObjectShape> objects,
    std::uint8_t scene_node_count) {
    SerializedLayout layout;
    layout.objects.reserve(objects.size());

    std::uint64_t cursor =
        header_size + static_cast<std::uint64_t>(objects.size()) * object_record_size;

    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        const auto& shape = objects[object_index];
        ObjectSerializedLayout object_layout;
        object_layout.record_offset =
            header_size + static_cast<std::uint64_t>(object_index) * object_record_size;
        object_layout.mesh_table_offset = cursor;
        object_layout.meshes.resize(shape.mesh_vertex_counts.size());

        for (std::size_t mesh_index = 0; mesh_index < object_layout.meshes.size(); ++mesh_index) {
            object_layout.meshes[mesh_index].record_offset =
                object_layout.mesh_table_offset +
                static_cast<std::uint64_t>(mesh_index) * mesh_record_size;
        }
        cursor = static_cast<std::uint64_t>(align16(
            static_cast<std::size_t>(
                object_layout.mesh_table_offset +
                static_cast<std::uint64_t>(object_layout.meshes.size()) * mesh_record_size)));

        for (std::size_t mesh_index = 0; mesh_index < object_layout.meshes.size(); ++mesh_index) {
            auto& mesh = object_layout.meshes[mesh_index];
            mesh.positions_offset = cursor;
            cursor = add_stream(cursor, shape.mesh_vertex_counts[mesh_index], 12U);
        }
        for (std::size_t mesh_index = 0; mesh_index < object_layout.meshes.size(); ++mesh_index) {
            auto& mesh = object_layout.meshes[mesh_index];
            mesh.normals_offset = cursor;
            cursor = add_stream(cursor, shape.mesh_vertex_counts[mesh_index], 12U);
        }
        for (std::size_t mesh_index = 0; mesh_index < object_layout.meshes.size(); ++mesh_index) {
            auto& mesh = object_layout.meshes[mesh_index];
            mesh.uv_offset = cursor;
            cursor = add_stream(cursor, shape.mesh_vertex_counts[mesh_index], 4U);
        }
        for (std::size_t mesh_index = 0; mesh_index < object_layout.meshes.size(); ++mesh_index) {
            auto& mesh = object_layout.meshes[mesh_index];
            mesh.color_flags_offset = cursor;
            cursor = add_stream(cursor, shape.mesh_vertex_counts[mesh_index], 4U);
        }

        layout.objects.push_back(std::move(object_layout));
    }

    layout.scene.block_offset = cursor;
    const auto node_count = static_cast<std::size_t>(scene_node_count);
    const auto aligned_node_bytes = align4(node_count);
    layout.scene.parent_rel = 0x20U;
    layout.scene.order_rel = static_cast<std::uint32_t>(0x20U + aligned_node_bytes);
    layout.scene.object_binding_rel = static_cast<std::uint32_t>(
        0x20U + 2U * aligned_node_bytes);
    layout.scene.transform_rel = static_cast<std::uint32_t>(align16(
        0x20U + 3U * aligned_node_bytes));
    const auto scene_end =
        layout.scene.block_offset + layout.scene.transform_rel +
        static_cast<std::uint64_t>(scene_node_count) * scene_transform_size;
    layout.scene.aligned_end = static_cast<std::uint64_t>(align16(
        static_cast<std::size_t>(scene_end)));

    cursor = layout.scene.aligned_end;
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        for (std::size_t mesh_index = 0;
             mesh_index < layout.objects[object_index].meshes.size();
             ++mesh_index) {
            auto& mesh = layout.objects[object_index].meshes[mesh_index];
            mesh.index_workspace_offset = cursor;
            mesh.index_workspace_capacity = index_workspace_capacity_bytes(
                objects[object_index].mesh_vertex_counts[mesh_index]);
            cursor += mesh.index_workspace_capacity;
        }
    }
    layout.file_size = cursor;
    return layout;
}

} // namespace dmc::rengine::formats::scm

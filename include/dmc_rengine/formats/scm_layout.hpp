#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::scm {

[[nodiscard]] constexpr std::size_t align4(std::size_t value) noexcept {
    return (value + 3U) & ~std::size_t{3U};
}

[[nodiscard]] constexpr std::size_t align16(std::size_t value) noexcept {
    return (value + 15U) & ~std::size_t{15U};
}

struct ObjectShape final {
    std::vector<std::uint16_t> mesh_vertex_counts;
};

struct MeshSerializedLayout final {
    std::uint64_t record_offset{};
    std::uint64_t positions_offset{};
    std::uint64_t normals_offset{};
    std::uint64_t uv_offset{};
    std::uint64_t color_flags_offset{};
    std::uint64_t index_workspace_offset{};
    std::uint64_t index_workspace_capacity{};
};

struct ObjectSerializedLayout final {
    std::uint64_t record_offset{};
    std::uint64_t mesh_table_offset{};
    std::vector<MeshSerializedLayout> meshes;
};

struct SceneSerializedLayout final {
    std::uint64_t block_offset{};
    std::uint32_t parent_rel{};
    std::uint32_t order_rel{};
    std::uint32_t object_binding_rel{};
    std::uint32_t transform_rel{};
    std::uint64_t aligned_end{};
};

struct SerializedLayout final {
    std::vector<ObjectSerializedLayout> objects;
    SceneSerializedLayout scene;
    std::uint64_t file_size{};
};

[[nodiscard]] constexpr std::size_t index_workspace_capacity_bytes(
    std::uint16_t vertex_count) noexcept {
    return vertex_count <= 2U
        ? 0U
        : align16(6U * static_cast<std::size_t>(vertex_count - 2U));
}

[[nodiscard]] SerializedLayout build_serialized_layout(
    std::span<const ObjectShape> objects,
    std::uint8_t scene_node_count);

} // namespace dmc::rengine::formats::scm

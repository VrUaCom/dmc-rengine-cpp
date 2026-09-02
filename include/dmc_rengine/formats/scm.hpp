#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::scm {

inline constexpr std::array<std::byte, 4> magic{
    std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{' '}};
inline constexpr std::size_t header_size = 0x40U;
inline constexpr std::size_t object_record_size = 0x40U;
inline constexpr std::size_t mesh_record_size = 0x50U;
inline constexpr std::size_t scene_block_header_size = 0x20U;
inline constexpr std::size_t scene_transform_size = 0x20U;
inline constexpr std::uint16_t index_workspace_sentinel = 0x1212U;

struct Vec3f final {
    float x{};
    float y{};
    float z{};
};

struct Header final {
    float version{};
    std::uint64_t reserved08{};
    std::uint8_t object_count{};
    std::uint8_t scene_node_count{};
    std::uint8_t texture_slot_count{};
    std::uint8_t reserved13{};
    std::uint32_t unresolved_id14{};
    std::uint64_t reserved18{};
    std::uint64_t scene_node_block_offset{};
    std::uint64_t reserved28{};
    std::uint64_t reserved30{};
    std::uint64_t reserved38{};
};

struct Mesh final {
    std::uint64_t record_offset{};
    std::uint16_t vertex_count{};
    std::uint16_t texture_index{};
    std::uint32_t reserved04{};
    std::uint64_t reserved08{};
    std::uint64_t positions_offset{};
    std::uint64_t normals_offset{};
    std::uint64_t uv_offset{};
    std::uint64_t continuation_span{};
    std::uint64_t reserved30{};
    std::uint64_t color_flags_offset{};
    std::uint64_t index_workspace_relative_offset{};
    std::uint32_t generated_index_count{};
    std::uint32_t reserved4c{};
    std::uint64_t index_workspace_offset{};
    std::uint64_t index_workspace_capacity{};
    std::uint8_t observed_topology_flag_mask{};
};

struct Object final {
    std::uint64_t record_offset{};
    std::uint8_t mesh_count{};
    std::uint8_t unresolved01{};
    std::uint16_t total_vertex_count{};
    std::uint32_t reserved04{};
    std::uint64_t mesh_table_offset{};
    std::uint32_t flags{};
    Vec3f bounding_center{};
    float bounding_radius{};
    std::vector<Mesh> meshes;
};

struct SceneTransform final {
    Vec3f translation{};
    float translation_magnitude{};
    Vec3f rotation_candidate{};
    float reserved1c{};
};

struct SceneNodeBlock final {
    std::uint64_t offset{};
    std::uint32_t parent_rel{};
    std::uint32_t order_rel{};
    std::uint32_t object_binding_rel{};
    std::uint32_t transform_rel{};
    std::vector<std::int8_t> parents;
    std::vector<std::uint8_t> order;
    std::vector<std::int8_t> object_bindings;
    std::vector<SceneTransform> transforms;
};

struct Document final {
    Header header;
    std::vector<Object> objects;
    SceneNodeBlock scene_nodes;
};

struct ParseResult final {
    bool recognized{false};
    Document document;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Parser final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::scm

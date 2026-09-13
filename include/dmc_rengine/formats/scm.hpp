#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/formats/scm_render.hpp"
#include "dmc_rengine/formats/scm_resource_code.hpp"

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

struct Vec3f final { float x{}; float y{}; float z{}; };
struct SerializedUv final { std::int16_t u{}; std::int16_t v{}; };
struct ColorTopology final { std::uint8_t r{}; std::uint8_t g{}; std::uint8_t b{}; std::uint8_t topology_flags{}; };

struct Header final {
    float version{};
    std::uint64_t reserved08{};
    std::uint8_t object_count{};
    std::uint8_t scene_node_count{};
    std::uint8_t texture_slot_count{};

    // Legacy storage name retained for authoring compatibility. Canonical EXE
    // evidence proves +0x13 -> manager+0xFA and CDrawSCM uses it as an index
    // into the scene-node world-matrix array at manager+0x188 (0x40 stride),
    // consuming selected matrix lane +0x30 as a spatial draw-state input.
    // Evidence-safe semantic name: draw_reference_node_index.
    std::uint8_t reserved13{};

    // Decimal structural decomposition is corpus-confirmed. Fresh retail st002
    // extends the observed family_class domain to 8. The exact high-level role
    // of the runtime copy at manager+0xE4 remains unproven.
    LegacyResourceCode resource_code{};

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
    LegacyGsClampRegionRepeat gs_clamp_region_repeat{};

    // Corpus-zero preservation lanes. Canonical deep census follows primary
    // materialization, post-init, common material/follow-up and SCM-specialized
    // allocation without recovering provenance-clean reads of these fields.
    std::uint32_t reserved0c{};
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
    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;
    std::vector<SerializedUv> uvs;
    std::vector<ColorTopology> colors_topology;
};

struct Object final {
    std::uint64_t record_offset{};
    std::uint8_t mesh_count{};
    std::uint8_t alpha_control{};
    std::uint16_t total_vertex_count{};
    std::uint32_t reserved04{};
    std::uint64_t mesh_table_offset{};

    // Runtime-consumed source flags. Proven bits have typed projections;
    // 0x00200000 remains real runtime-carried state with expanded whole-image
    // negative provenance and must be preserved exactly.
    std::uint32_t flags{};
    std::array<std::byte, 0x1CU> reserved14_2f{};
    Vec3f bounding_center{};
    float bounding_radius{};
    std::vector<Mesh> meshes;
};

struct SceneTransform final {
    Vec3f translation{};
    float translation_magnitude{};
    Vec3f rotation_xyz_radians{};
    float reserved1c{};
};

struct SceneNodeBlock final {
    std::uint64_t offset{};
    std::uint32_t parent_rel{};
    std::uint32_t order_rel{};
    std::uint32_t object_binding_rel{};
    std::uint32_t transform_rel{};
    std::array<std::byte, 0x10U> reserved10_1f{};
    std::vector<std::int8_t> parent_by_order_position;
    std::vector<std::uint8_t> node_at_order_position;
    std::vector<std::int8_t> object_binding_by_node_index;
    std::vector<SceneTransform> transform_by_node_index;
};

struct Document final {
    Header header;
    std::vector<Object> objects;
    SceneNodeBlock scene_nodes;
    std::vector<std::byte> source_bytes;
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

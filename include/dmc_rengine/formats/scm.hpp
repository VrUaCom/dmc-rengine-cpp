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

struct Vec3f final {
    float x{};
    float y{};
    float z{};
};

// Exact serialized UV representation. The canonical DMC3-HD path interprets
// each component with scale 1/4096. Keeping the signed 16-bit source values in
// the IR makes no-edit authoring bit preserving and avoids imposing a frontend
// coordinate convention on the binary model.
struct SerializedUv final {
    std::int16_t u{};
    std::int16_t v{};
};

struct ColorTopology final {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t topology_flags{};
};

struct Header final {
    float version{};

    // Terminal preservation fields: zero on the bounded retail SCM corpus.
    // The canonical model-manager source-pointer census finds no typed runtime
    // effect for +0x08, +0x18 or +0x28..+0x3F. The `reserved*` member names are
    // physical compatibility names only; writers must preserve source bytes.
    std::uint64_t reserved08{};
    std::uint8_t object_count{};
    std::uint8_t scene_node_count{};

    // Serialized consistency/mirror value. The canonical main load path takes
    // runtime texture-table authority from the external texture companion.
    std::uint8_t texture_slot_count{};

    // +0x13 is copied to manager +0xFA and is zero on the bounded corpus. No
    // higher-level semantic is evidenced; terminal status is
    // PRESERVED_UNDECODED rather than RESEARCH_REQUIRED.
    std::uint8_t reserved13{};

    // Runtime-carried legacy identity/provenance code from serialized +0x14.
    // The decimal component decomposition is corpus-confirmed. Official source
    // names for family classes 3/4 are unavailable, so neutral structural names
    // are the terminal evidence-safe contract.
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

    // EXE-confirmed index into the runtime table derived from the external
    // texture companion.
    std::uint16_t texture_index{};

    // Legacy PS2 GS CLAMP REGION_REPEAT state serialized at +0x04..+0x0B.
    // Stock HD SCM corpus uses zero, but canonical executable support is live.
    LegacyGsClampRegionRepeat gs_clamp_region_repeat{};

    // +0x0C is zero on the bounded corpus and skipped by the confirmed SCM
    // normalizer/material path. It remains a source-preserved terminal
    // PRESERVED_UNDECODED u32; `reserved0c` is a physical compatibility name.
    std::uint32_t reserved0c{};

    std::uint64_t positions_offset{};
    std::uint64_t normals_offset{};
    std::uint64_t uv_offset{};
    std::uint64_t continuation_span{};

    // +0x30 is zero on the canonical corpus and has no promoted serialized
    // semantic. Preserve exactly; do not normalize merely because it is zero.
    std::uint64_t reserved30{};

    std::uint64_t color_flags_offset{};
    std::uint64_t index_workspace_relative_offset{};
    std::uint32_t generated_index_count{};

    // Trailing +0x4C is zero on the canonical corpus and remains terminal
    // preservation-only state.
    std::uint32_t reserved4c{};

    std::uint64_t index_workspace_offset{};
    std::uint64_t index_workspace_capacity{};
    std::uint8_t observed_topology_flag_mask{};

    // Materialized authoring payloads. Their lengths must agree exactly with
    // vertex_count in preserve-layout mode. Canonical rebuild derives the
    // serialized vertex count from these vectors instead of trusting stale
    // dependent metadata.
    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;
    std::vector<SerializedUv> uvs;
    std::vector<ColorTopology> colors_topology;
};

struct Object final {
    std::uint64_t record_offset{};
    std::uint8_t mesh_count{};

    // EXE-confirmed alpha-control byte. Common values <=0x80 are normalized
    // to MDL_PARTS_COLOR_PKT.alpha.w as value/255. Values >0x80 act as
    // control codes and force alpha.w=1 while remaining available to runtime.
    // The canonical EXE also contains two narrow hard-coded C4/EA corrections.
    std::uint8_t alpha_control{};

    std::uint16_t total_vertex_count{};

    // Corpus-zero preservation-only field. The member name describes its
    // physical location, not permission to discard or normalize it.
    std::uint32_t reserved04{};

    std::uint64_t mesh_table_offset{};

    // Runtime-consumed source flags. Confirmed bits are exposed by dedicated
    // render/runtime helpers; terminal undecoded bits (notably 0x00200000) must
    // remain source-preserved.
    std::uint32_t flags{};

    // Bytes +0x14..+0x2F are zero in the bounded retail corpus and have no
    // promoted high-level semantic. They are terminal PRESERVED_UNDECODED state
    // and are represented explicitly so a writer never silently drops them.
    std::array<std::byte, 0x1CU> reserved14_2f{};

    Vec3f bounding_center{};
    float bounding_radius{};
    std::vector<Mesh> meshes;
};

struct SceneTransform final {
    Vec3f translation{};

    // Corpus-confirmed precomputed length(translation). The local matrix
    // builder deliberately ignores this fourth lane when constructing
    // homogeneous translation W.
    float translation_magnitude{};

    // EXE-confirmed through the SCM-specific initializer 0x1402FA360 ->
    // 0x140330450: serialized +0x10/+0x14/+0x18 are X/Y/Z Euler angles in
    // radians. The game applies X, then Y, then Z, yielding Rz * Ry * Rx.
    Vec3f rotation_xyz_radians{};

    // +0x1C is zero on the canonical SCM corpus and is not consumed by the
    // confirmed SCM local-matrix construction. It is terminal preservation-only
    // state, not an open semantic requirement.
    float reserved1c{};
};

struct SceneNodeBlock final {
    std::uint64_t offset{};
    std::uint32_t parent_rel{};
    std::uint32_t order_rel{};
    std::uint32_t object_binding_rel{};
    std::uint32_t transform_rel{};

    // Serialized scene-header +0x10..+0x1F is terminal preservation-only state.
    // The bounded corpus is zero; canonical model planner reads +0x10 once into
    // a dead local and +0x11..+0x1F has no provenance-confirmed model consumer.
    // Preserve the complete shell exactly rather than treating it as padding.
    std::array<std::byte, 0x10U> reserved10_1f{};

    // EXE-confirmed indexing contract:
    //   position i in evaluation order -> node_at_order_position[i]
    //   position i in evaluation order -> parent_by_order_position[i]
    // Object bindings and transforms are indexed by scene-node index itself.
    std::vector<std::int8_t> parent_by_order_position;
    std::vector<std::uint8_t> node_at_order_position;
    std::vector<std::int8_t> object_binding_by_node_index;
    std::vector<SceneTransform> transform_by_node_index;
};

struct Document final {
    Header header;
    std::vector<Object> objects;
    SceneNodeBlock scene_nodes;

    // Original recognized byte image. This is the preservation authority for
    // same-layout authoring: unknown/preservation-only padding/workspace bytes
    // survive unchanged. Canonical rebuild has its own fail-closed policy.
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

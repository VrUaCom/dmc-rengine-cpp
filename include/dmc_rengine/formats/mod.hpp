#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"
#include "dmc_rengine/formats/model_object_core.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::mod {

inline constexpr std::size_t header_size = 0x40U;
inline constexpr std::size_t outer_record_size =
    model_family::ObjectCoreAbi::record_size;
inline constexpr std::size_t inner_record_size =
    model_family::MeshCoreAbi::record_size;

struct Vec3f final {
    float x{};
    float y{};
    float z{};
};

struct SerializedUv final {
    std::int16_t u{};
    std::int16_t v{};
};

struct BlendIndices final {
    std::array<std::uint8_t, 4> lanes{};
};

struct LegacyGsClampRegionRepeat final {
    std::uint16_t min_u{};
    std::uint16_t max_u{};
    std::uint16_t min_v{};
    std::uint16_t max_v{};
};

struct InnerMesh final {
    std::uint64_t record_offset{};
    std::uint16_t element_count{};

    // EXE-confirmed common MOD/EFM/SCM material payload consumed by
    // 0x1402F9890 after the format-specific mesh builders.
    std::uint16_t texture_slot{};
    LegacyGsClampRegionRepeat gs_clamp_region_repeat{};

    std::uint64_t positions_offset{};
    std::uint64_t normals_offset{};
    std::uint64_t uv_offset{};
    std::uint64_t blend_indices_offset{};
    std::uint64_t control_offset{};
    std::uint64_t reserved38{};
    std::uint64_t generated_workspace_relative_offset{};
    std::uint32_t generated_topology_count{};
    std::uint32_t reserved4c{};
    std::uint64_t generated_workspace_offset{};

    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;
    std::vector<SerializedUv> uvs;
    std::vector<BlendIndices> blend_indices;
    std::vector<std::uint16_t> control_words;
    std::vector<SkinDecodeResult> skin;

    std::size_t skin_decode_failures{};
    std::size_t reserved_blend_lane_nonzero{};
};

struct OuterModel final {
    std::uint64_t record_offset{};
    std::uint8_t inner_record_count{};

    // Raw object state copied by canonical MOD/EFM initializer 0x1403029E0.
    // Keep this value format-local: SCM has additional narrow compatibility
    // corrections that are not promoted into the MOD parser by this contract.
    std::uint8_t alpha_control{};

    std::uint16_t aggregate_element_count{};
    std::uint64_t inner_table_offset{};

    // Serialized source flags are copied verbatim into the MOD runtime object's
    // baseline and mutable effective flag fields. Per-bit semantics remain
    // independently evidence-gated.
    std::uint32_t source_flags{};

    // Shared serialized object bounding sphere consumed by the runtime object
    // initializer as vec3 center + f32 radius.
    Vec3f bounding_center{};
    float bounding_radius{};

    std::vector<InnerMesh> meshes;
};

struct Header final {
    float version{};
    std::uint8_t outer_record_count{};
    std::uint8_t transform_domain_count{};
    std::uint64_t document_offset{};
};

struct Document final {
    Header header;
    std::vector<OuterModel> outer_models;
    transform_domain::ParseResult transform_domain;

    // Byte-preservation authority. The structural reader is read-only, but
    // retaining the recognized source image keeps future authoring work from
    // silently discarding unresolved fields.
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

} // namespace dmc::rengine::formats::mod
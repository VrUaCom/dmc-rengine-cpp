#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::mod {

inline constexpr std::size_t header_size = 0x40U;
inline constexpr std::size_t outer_record_size = 0x40U;
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

struct InnerMesh final {
    std::uint64_t record_offset{};
    std::uint16_t element_count{};

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
    std::uint16_t aggregate_element_count{};
    std::uint64_t inner_table_offset{};
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

#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod/version.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/formats/model_document_core.hpp"
#include "dmc_rengine/formats/model_mesh_core.hpp"
#include "dmc_rengine/formats/model_object_core.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::mod {

inline constexpr std::size_t header_size =
    model_family::DocumentCoreAbi::header_size;
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
    // Retail em000 confirms the recovered structural grammar at versions
    // 0.82, 0.84, 1.00 and 1.01. See formats/mod/version.hpp. Other versions
    // remain readable when structurally valid, but are reported as unconfirmed.
    float version{};
    std::uint8_t outer_record_count{};
    std::uint8_t transform_domain_count{};

    // Serialized texture-slot-domain count/mirror. The canonical manager takes
    // live texture-table authority from the external companion, so this value
    // is retained for comparison rather than treated as runtime truth.
    std::uint8_t texture_slot_count{};

    // Serialized +0x13 is copied to manager +0xFA by 0x1402F9570. The MOD
    // motion/control parser uses that value as the fallback for the `JntNo`
    // field when a parsed joint selector is out of range; 0x1402FD040 also
    // indexes currentWorld[] with it. Keep the raw storage name for source/API
    // compatibility while exposing the proven MOD-specific semantic accessor.
    std::uint8_t runtime_mode_byte{};

    // +0x14 is a raw runtime-carried u32 copied to manager +0xE4. Expanded
    // em000/pl000/id100 corpus evidence rejects the earlier universal decimal
    // component interpretation: values such as 217 and 1000000 do not support
    // a stable MOD-wide family/model/sub-index semantic partition. No typed
    // downstream manager consumer currently promotes a higher-level name.
    // Preserve the source value exactly; do not derive it from resource names
    // and do not transfer SCM LegacyResourceCode semantics into MOD.
    std::uint32_t runtime_metadata_u32{};

    [[nodiscard]] constexpr std::uint8_t default_joint_index() const noexcept {
        return runtime_mode_byte;
    }

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

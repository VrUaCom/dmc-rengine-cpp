#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_primitive_shape_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class ParsedShapeKind : std::uint8_t {
    sphere = 0,
    box = 1,
    cylinder = 2,
    capsule = 3,
};

enum class PrimitiveBodyRole : std::uint8_t {
    shape_parser_cluster,
    parsed_shape_converter,
    runtime_constructor_dispatcher,
    runtime_sphere_or_swept_sphere_constructor,
    runtime_box_constructor,
    runtime_capsule_constructor,
    runtime_cylinder_constructor,
    runtime_three_vertex_face_constructor,
};

struct CanonicalFunctionBodyEvidence final {
    PrimitiveBodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct ParsedShapeMappingEvidence final {
    ParsedShapeKind parsed_kind{};
    std::string_view token{};
    std::uint8_t parser_enum_value{};
    std::uint8_t internal_descriptor_type{};
    std::uint8_t primary_runtime_type{};
};

struct ParserToConverterFlowEvidence final {
    std::uint64_t parser_va{};
    std::uint64_t builder_va{};
    std::uint64_t converter_va{};
    std::int32_t parser_frame_shape_offset{};
    std::uint32_t builder_shape_member_offset{};
    bool same_parsed_shape_storage_proven{};
};

struct RuntimeType4Evidence final {
    std::uint8_t runtime_type{};
    std::uint8_t parsed_capsule_descriptor_type{};
    std::uint8_t swept_sphere_descriptor_type{};
    std::uint32_t first_endpoint_offset{};
    std::uint32_t second_endpoint_offset{};
    std::uint32_t radius_offset{};
    bool parsed_capsule_builds_runtime_type4{};
    bool moving_sphere_may_promote_to_runtime_type4{};
    bool sphere_promotion_threshold_is_two_times_radius{};
};

struct RuntimeType5StructuralEvidence final {
    std::uint8_t runtime_type{};
    std::uint8_t internal_descriptor_type{};
    std::uint32_t first_vertex_offset{};
    std::uint32_t second_vertex_offset{};
    std::uint32_t third_vertex_offset{};
    std::uint32_t auxiliary_vector_offset{};
    std::uint32_t vertex_count{};
    bool transforms_three_vertices{};
    bool normalizes_auxiliary_vector{};
    bool three_vertex_face_representation_confirmed{};
    bool source_text_shape_name_confirmed{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, PrimitiveBodyRole role) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_bodies) {
            if (descriptor.role == role) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<ParsedShapeMappingEvidence>
    parsed_shape(std::string_view sha256, ParsedShapeKind kind) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_shape_mappings) {
            if (descriptor.parsed_kind == kind) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<ParserToConverterFlowEvidence>
    parser_to_converter_flow(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_parser_to_converter_flow;
    }

    [[nodiscard]] static std::optional<RuntimeType4Evidence>
    runtime_type4(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_runtime_type4;
    }

    [[nodiscard]] static std::optional<RuntimeType5StructuralEvidence>
    runtime_type5(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_runtime_type5;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 8> k_bodies{{
        {PrimitiveBodyRole::shape_parser_cluster,
         0x140247AD0ULL,
         0x1402481E0ULL,
         1808U,
         "1bd8a8e369105512166e5f2557d274c86e3801b56631f0d4f7022983adfda6e2"},
        {PrimitiveBodyRole::parsed_shape_converter,
         0x1402481E0ULL,
         0x1402482C5ULL,
         229U,
         "ed50d780aa2bd2868b783c4924edcac6c2af9b4269225ce92b7c778535d7c58a"},
        {PrimitiveBodyRole::runtime_constructor_dispatcher,
         0x1402CC530ULL,
         0x1402CC5EFULL,
         191U,
         "e8e0065e0aeb76b6839d0506e09623b48091d853bd9ccee3127bd0bec7e19364"},
        {PrimitiveBodyRole::runtime_sphere_or_swept_sphere_constructor,
         0x1402CCB50ULL,
         0x1402CCCB0ULL,
         352U,
         "6c8735293b9040d4b44405b45a64dabebbcbe3be0bf873a2159f79747493c189"},
        {PrimitiveBodyRole::runtime_box_constructor,
         0x1402CC610ULL,
         0x1402CC884ULL,
         628U,
         "d5d6e4d714626cafeaee21697d856887c4d6505c092c3498f14e82a838eea821"},
        {PrimitiveBodyRole::runtime_capsule_constructor,
         0x1402CC890ULL,
         0x1402CC994ULL,
         260U,
         "61b2966fe7ff3766ec821f3e5704e0e1d11568e796b22513c27b990c79981bd3"},
        {PrimitiveBodyRole::runtime_cylinder_constructor,
         0x1402CC9A0ULL,
         0x1402CCA26ULL,
         134U,
         "d53e4ecb5e91dfc0fedcf8c77648cb6c9074646e8853ee8f26e9a2b0add056a6"},
        {PrimitiveBodyRole::runtime_three_vertex_face_constructor,
         0x1402CCCB0ULL,
         0x1402CCD5BULL,
         171U,
         "766bf79c39056c7baaa6bb1cc097fab863651f7b5350edd2b07ff0509f432a2e"},
    }};

    inline static constexpr std::array<ParsedShapeMappingEvidence, 4> k_shape_mappings{{
        {ParsedShapeKind::sphere, "sphere", 0U, 2U, 2U},
        {ParsedShapeKind::box, "box", 1U, 3U, 3U},
        {ParsedShapeKind::cylinder, "cylinder", 2U, 6U, 6U},
        {ParsedShapeKind::capsule, "capsule", 3U, 4U, 4U},
    }};

    inline static constexpr ParserToConverterFlowEvidence k_parser_to_converter_flow{
        0x140247AD0ULL,
        0x140249710ULL,
        0x1402481E0ULL,
        -0x38,
        0x98U,
        true,
    };

    inline static constexpr RuntimeType4Evidence k_runtime_type4{
        4U,
        4U,
        2U,
        0x130U,
        0x140U,
        0x150U,
        true,
        true,
        true,
    };

    inline static constexpr RuntimeType5StructuralEvidence k_runtime_type5{
        5U,
        5U,
        0x130U,
        0x140U,
        0x150U,
        0x160U,
        3U,
        true,
        true,
        true,
        false,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, PrimitiveBodyRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<ParsedShapeMappingEvidence>
parsed_shape_mapping(std::string_view sha256, ParsedShapeKind kind) noexcept {
    return detail::EvidenceStore::parsed_shape(sha256, kind);
}

[[nodiscard]] inline std::optional<ParserToConverterFlowEvidence>
parser_to_converter_flow(std::string_view sha256) noexcept {
    return detail::EvidenceStore::parser_to_converter_flow(sha256);
}

[[nodiscard]] inline std::optional<RuntimeType4Evidence>
runtime_type4_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::runtime_type4(sha256);
}

[[nodiscard]] inline std::optional<RuntimeType5StructuralEvidence>
runtime_type5_structural_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::runtime_type5(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_primitive_shape_evidence

#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_contact_normal_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class ContactNormalBodyRole : std::uint8_t {
    static_facing_dot3,
    normalize_xyz_direction,
    transform_direction_3x3,
    reflection_dot3,
    reflection_xyz_scale_preserve_w,
    reflection_consumer,
};

struct CanonicalFunctionBodyEvidence final {
    ContactNormalBodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct CommonSurfaceNormalEvidence final {
    std::uint32_t metadata_offset{};
    std::uint32_t component_count{};
    std::uint32_t static_raw_record_offset{};
    std::uint32_t dynamic_metadata_output_offset{};
    bool static_facing_dot_product_confirmed{};
    bool dynamic_sphere_radial_normal_confirmed{};
    bool dynamic_three_vertex_face_normal_confirmed{};
    bool dynamic_cylinder_cap_and_radial_normal_confirmed{};
    bool reflection_consumer_confirmed{};
    bool common_collision_surface_normal_semantics_confirmed{};
    bool exact_orientation_or_handedness_confirmed{};
};

struct StaticSurfaceNormalEvidence final {
    std::uint64_t static_query_va{};
    std::uint32_t raw_record_normal_offset{};
    std::uint64_t direction_normalizer_va{};
    std::uint64_t facing_dot3_va{};
    bool query_direction_is_normalized{};
    bool record_vector_is_used_as_facing_normal{};
};

struct DynamicSurfaceNormalEvidence final {
    std::uint64_t dynamic_query_va{};
    std::uint32_t metadata_normal_offset{};
    std::uint32_t sphere_runtime_type{};
    std::uint32_t three_vertex_face_runtime_type{};
    std::uint32_t cylinder_runtime_type{};
    std::uint32_t three_vertex_face_object_normal_offset{};
    bool sphere_uses_normalized_contact_minus_center{};
    bool face_uses_pre_normalized_object_direction{};
    bool cylinder_uses_axial_or_normalized_radial_direction{};
};

struct ReflectionConsumerEvidence final {
    std::uint64_t consumer_va{};
    std::uint64_t query_callsite_va{};
    std::uint32_t metadata_normal_offset{};
    std::uint64_t dot3_helper_va{};
    std::uint64_t xyz_scale_preserve_w_helper_va{};
    bool computes_v_minus_two_dot_v_n_times_n{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, ContactNormalBodyRole role) noexcept {
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

    [[nodiscard]] static std::optional<CommonSurfaceNormalEvidence>
    common_surface_normal(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_common_surface_normal;
    }

    [[nodiscard]] static std::optional<StaticSurfaceNormalEvidence>
    static_surface_normal(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_static_surface_normal;
    }

    [[nodiscard]] static std::optional<DynamicSurfaceNormalEvidence>
    dynamic_surface_normal(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_dynamic_surface_normal;
    }

    [[nodiscard]] static std::optional<ReflectionConsumerEvidence>
    reflection_consumer(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_reflection_consumer;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 6> k_bodies{{
        {ContactNormalBodyRole::static_facing_dot3,
         0x14032E5C0ULL,
         0x14032E5E5ULL,
         37U,
         "d9dba88696b4f17f15617eb2dd1b2e39713da0d0225f4e2130e0efc0931c0779"},
        {ContactNormalBodyRole::normalize_xyz_direction,
         0x140330390ULL,
         0x14033044AULL,
         186U,
         "568543ebcb3a7ea60b09e65c54da9f9364417d3679f5e9101a80722472f1a9d4"},
        {ContactNormalBodyRole::transform_direction_3x3,
         0x14032DC70ULL,
         0x14032DCF6ULL,
         134U,
         "46a253c0b7824d5ac26104947597961ff4d1399cabbd069b732ad43bab29fe1d"},
        {ContactNormalBodyRole::reflection_dot3,
         0x140030D30ULL,
         0x140030DB1ULL,
         129U,
         "7a1a584ecdd42d4c0af7c9f5bfdb02a47d32facd4d0c75efd777e11cac5cdae8"},
        {ContactNormalBodyRole::reflection_xyz_scale_preserve_w,
         0x1400311E0ULL,
         0x1400311FEULL,
         30U,
         "c4a58e73cfb2af640f32fad70956b98f58038c4d28b23c62ea74a6f239300e40"},
        {ContactNormalBodyRole::reflection_consumer,
         0x1402C64F0ULL,
         0x1402C6805ULL,
         789U,
         "86b77b7431411f7b2ed1c6f7b1109aebd36c305e2e8684c6d6897f915b76e7c4"},
    }};

    inline static constexpr CommonSurfaceNormalEvidence k_common_surface_normal{
        0x28U,
        3U,
        0x28U,
        0x28U,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
    };

    inline static constexpr StaticSurfaceNormalEvidence k_static_surface_normal{
        0x14005E880ULL,
        0x28U,
        0x140330390ULL,
        0x14032E5C0ULL,
        true,
        true,
    };

    inline static constexpr DynamicSurfaceNormalEvidence k_dynamic_surface_normal{
        0x14005BCF0ULL,
        0x28U,
        2U,
        5U,
        6U,
        0x160U,
        true,
        true,
        true,
    };

    inline static constexpr ReflectionConsumerEvidence k_reflection_consumer{
        0x1402C64F0ULL,
        0x1402C65B2ULL,
        0x28U,
        0x140030D30ULL,
        0x1400311E0ULL,
        true,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, ContactNormalBodyRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<CommonSurfaceNormalEvidence>
common_surface_normal_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::common_surface_normal(sha256);
}

[[nodiscard]] inline std::optional<StaticSurfaceNormalEvidence>
static_surface_normal_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::static_surface_normal(sha256);
}

[[nodiscard]] inline std::optional<DynamicSurfaceNormalEvidence>
dynamic_surface_normal_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::dynamic_surface_normal(sha256);
}

[[nodiscard]] inline std::optional<ReflectionConsumerEvidence>
reflection_consumer_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::reflection_consumer(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_contact_normal_evidence

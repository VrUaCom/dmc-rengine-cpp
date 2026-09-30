#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_primitive_type01_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class Primitive01BodyRole : std::uint8_t {
    runtime_type0_constructor,
    runtime_type1_constructor,
    xyz_endpoint_distance,
};

struct CanonicalFunctionBodyEvidence final {
    Primitive01BodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct RuntimeType0Evidence final {
    std::uint8_t runtime_type{};
    std::uint32_t descriptor_point_offset{};
    std::uint32_t runtime_point_offset{};
    bool transforms_one_position{};
    bool one_point_representation_confirmed{};
    bool source_token_confirmed{};
};

struct RuntimeType1Evidence final {
    std::uint8_t runtime_type{};
    std::uint32_t descriptor_first_endpoint_offset{};
    std::uint32_t descriptor_second_endpoint_offset{};
    std::uint32_t runtime_first_endpoint_offset{};
    std::uint32_t runtime_second_endpoint_offset{};
    std::uint32_t runtime_midpoint_offset{};
    std::uint32_t runtime_endpoint_distance_offset{};
    bool transforms_two_endpoints{};
    bool computes_midpoint{};
    bool computes_xyz_endpoint_distance{};
    bool contact_update_may_replace_first_endpoint{};
    bool two_endpoint_segment_representation_confirmed{};
    bool source_token_confirmed{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, Primitive01BodyRole role) noexcept {
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

    [[nodiscard]] static std::optional<RuntimeType0Evidence>
    type0(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_type0;
    }

    [[nodiscard]] static std::optional<RuntimeType1Evidence>
    type1(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_type1;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 3> k_bodies{{
        {Primitive01BodyRole::runtime_type0_constructor,
         0x1402CCA30ULL,
         0x1402CCA64ULL,
         52U,
         "8033e51cb704386397c4ee988304b55dbffd538e0710514eaedc785da26ba132"},
        {Primitive01BodyRole::runtime_type1_constructor,
         0x1402CCA70ULL,
         0x1402CCB46ULL,
         214U,
         "c06cf1554f9b98b3d012ab0f83b3d2ae85d1a0b0c7a49ea69c573cd4820218fd"},
        {Primitive01BodyRole::xyz_endpoint_distance,
         0x14032E5F0ULL,
         0x14032E62DULL,
         61U,
         "7ca8c6b01c910665e3f70490b9339010ab6a17a5193779f29f0863278520f7d1"},
    }};

    inline static constexpr RuntimeType0Evidence k_type0{
        0U,
        0x10U,
        0x130U,
        true,
        true,
        false,
    };

    inline static constexpr RuntimeType1Evidence k_type1{
        1U,
        0x10U,
        0x20U,
        0x130U,
        0x140U,
        0xE0U,
        0x18U,
        true,
        true,
        true,
        true,
        true,
        false,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, Primitive01BodyRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<RuntimeType0Evidence>
runtime_type0_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::type0(sha256);
}

[[nodiscard]] inline std::optional<RuntimeType1Evidence>
runtime_type1_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::type1(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_primitive_type01_evidence

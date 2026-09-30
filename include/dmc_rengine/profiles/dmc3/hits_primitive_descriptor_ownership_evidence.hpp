#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_primitive_descriptor_ownership_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class DescriptorOwnershipBodyRole : std::uint8_t {
    manager_source_initializer,
    runtime_object_builder_indirect_transform,
    runtime_object_builder_inline_transform,
    runtime_object_initializer,
};

struct CanonicalFunctionBodyEvidence final {
    DescriptorOwnershipBodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct ManagerDescriptorTableEvidence final {
    std::uint64_t manager_source_initializer_va{};
    std::uint8_t entry_table_arg_index{};
    std::uint8_t descriptor_table_arg_index{};
    std::uint32_t manager_entry_table_pointer_offset{};
    std::uint32_t manager_descriptor_table_pointer_offset{};
    std::uint32_t entry_stride_bytes{};
    std::uint32_t entry_flags_offset{};
    std::uint32_t entry_transform_selector_offset{};
    std::uint32_t entry_descriptor_index_offset{};
    std::uint32_t entry_descriptor_index_width_bytes{};
    std::uint32_t descriptor_stride_bytes{};
};

struct RuntimeDescriptorOwnershipEvidence final {
    std::uint64_t runtime_object_initializer_va{};
    std::uint8_t entry_arg_index{};
    std::uint8_t primitive_descriptor_arg_index{};
    std::uint8_t runtime_object_arg_index{};
    std::uint8_t transform_pointer_arg_index{};
    std::uint32_t runtime_entry_pointer_offset{};
    std::uint32_t runtime_primitive_descriptor_pointer_offset{};
    std::uint32_t runtime_transform_pointer_offset{};
    std::uint64_t constructor_dispatcher_va{};
    bool constructor_reads_transform_from_runtime_offset_0x20{};
    bool constructor_reads_descriptor_from_runtime_offset_0x118{};
    bool descriptor_first_byte_dispatches_types_0_through_6{};
};

struct DescriptorResolutionEvidence final {
    std::uint64_t indirect_transform_builder_va{};
    std::uint64_t inline_transform_builder_va{};
    std::uint32_t manager_entry_table_pointer_offset{};
    std::uint32_t manager_descriptor_table_pointer_offset{};
    std::uint32_t entry_stride_bytes{};
    std::uint32_t descriptor_index_offset{};
    std::uint32_t descriptor_stride_bytes{};
    std::uint32_t transform_selector_offset{};
    std::uint32_t inline_transform_stride_bytes{};
    bool both_builders_resolve_same_primitive_descriptor_table{};
    bool transform_source_is_separate_from_primitive_descriptor{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, DescriptorOwnershipBodyRole role) noexcept {
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

    [[nodiscard]] static std::optional<ManagerDescriptorTableEvidence>
    manager_table(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_manager_table;
    }

    [[nodiscard]] static std::optional<RuntimeDescriptorOwnershipEvidence>
    runtime_ownership(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_runtime_ownership;
    }

    [[nodiscard]] static std::optional<DescriptorResolutionEvidence>
    descriptor_resolution(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_descriptor_resolution;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 4> k_bodies{{
        {DescriptorOwnershipBodyRole::manager_source_initializer,
         0x14005C260ULL,
         0x14005C318ULL,
         184U,
         "9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7"},
        {DescriptorOwnershipBodyRole::runtime_object_builder_indirect_transform,
         0x14005C630ULL,
         0x14005C731ULL,
         257U,
         "97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a"},
        {DescriptorOwnershipBodyRole::runtime_object_builder_inline_transform,
         0x14005C740ULL,
         0x14005C83DULL,
         253U,
         "0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6"},
        {DescriptorOwnershipBodyRole::runtime_object_initializer,
         0x14005C8D0ULL,
         0x14005C99DULL,
         205U,
         "f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe"},
    }};

    inline static constexpr ManagerDescriptorTableEvidence k_manager_table{
        0x14005C260ULL,
        2U,
        3U,
        0x108U,
        0x110U,
        4U,
        0U,
        1U,
        2U,
        2U,
        0x50U,
    };

    inline static constexpr RuntimeDescriptorOwnershipEvidence k_runtime_ownership{
        0x14005C8D0ULL,
        2U,
        3U,
        4U,
        5U,
        0x110U,
        0x118U,
        0x20U,
        0x1402CC530ULL,
        true,
        true,
        true,
    };

    inline static constexpr DescriptorResolutionEvidence k_descriptor_resolution{
        0x14005C630ULL,
        0x14005C740ULL,
        0x108U,
        0x110U,
        4U,
        2U,
        0x50U,
        1U,
        0x40U,
        true,
        true,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, DescriptorOwnershipBodyRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<ManagerDescriptorTableEvidence>
manager_descriptor_table_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::manager_table(sha256);
}

[[nodiscard]] inline std::optional<RuntimeDescriptorOwnershipEvidence>
runtime_descriptor_ownership_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::runtime_ownership(sha256);
}

[[nodiscard]] inline std::optional<DescriptorResolutionEvidence>
descriptor_resolution_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::descriptor_resolution(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_primitive_descriptor_ownership_evidence

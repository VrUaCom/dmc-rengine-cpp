#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_dynamic_update_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class DynamicUpdateBodyRole : std::uint8_t {
    pair_resolver,
    category_compatibility,
    dispatcher,
    post_update_step,
};

struct CanonicalFunctionBodyEvidence final {
    DynamicUpdateBodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct PairResolverAbiEvidence final {
    std::uint64_t function_va{};
    std::uint8_t manager_arg_index{};
    std::uint8_t source_object_arg_index{};
    std::uint8_t candidate_list_head_arg_index{};
    std::uint8_t source_category_arg_index{};
    std::uint8_t target_category_arg_index{};
    std::uint32_t linked_next_offset{};
    std::uint32_t candidate_type_offset{};
    std::uint32_t required_candidate_type{};
    std::uint32_t owner_pointer_offset{};
    std::uint32_t reciprocal_category_bits_offset{};
    bool rejects_self_pair{};
    bool rejects_shared_owner{};
    bool calls_category_compatibility_filter{};
    bool applies_contact_displacement{};
    bool sets_reciprocal_category_bits{};
};

struct CategoryMaskBridge final {
    std::uint32_t category_id{};
    std::uint16_t raw_reject_mask{};
};

struct CategoryCompatibilityEvidence final {
    std::uint64_t function_va{};
    std::uint32_t special_category_id{};
    std::uint32_t raw_flags_offset{};
    std::uint32_t raw_flags_width_bytes{};
    bool neither_special_category_is_allowed{};
    bool both_special_category_is_allowed{};
    bool one_special_category_uses_other_category_mask{};
};

struct DispatcherBindingEvidence final {
    std::uint32_t activation_flag{};
    std::uint32_t manager_list_offset{};
    std::uint32_t target_category_id{};
};

struct DynamicUpdateDispatcherEvidence final {
    std::uint64_t function_va{};
    std::uint64_t pair_resolver_va{};
    std::uint64_t post_update_step_va{};
    bool iterates_source_object_list{};
    bool dispatches_enabled_target_categories{};
    bool runs_post_update_step_after_pair_dispatch{};
};

struct PostUpdateStepEvidence final {
    std::uint64_t function_va{};
    std::uint32_t result_category_bits_offset{};
    std::uint32_t dynamic_contact_result_bit{};
    bool uses_category_to_reject_mask_bridge{};
    bool invokes_specialized_hits_queries{};
    bool may_apply_position_correction{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, DynamicUpdateBodyRole role) noexcept {
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

    [[nodiscard]] static std::optional<PairResolverAbiEvidence>
    pair_resolver(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_pair_resolver;
    }

    [[nodiscard]] static std::optional<CategoryCompatibilityEvidence>
    category_compatibility(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_category_compatibility;
    }

    [[nodiscard]] static std::optional<CategoryMaskBridge>
    category_mask(std::string_view sha256, std::uint32_t category_id) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& binding : k_category_masks) {
            if (binding.category_id == category_id) {
                return binding;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<DispatcherBindingEvidence>
    dispatcher_binding(std::string_view sha256, std::size_t index) noexcept {
        if (!matches_canonical_target(sha256) || index >= k_dispatcher_bindings.size()) {
            return std::nullopt;
        }
        return k_dispatcher_bindings[index];
    }

    [[nodiscard]] static std::optional<DynamicUpdateDispatcherEvidence>
    dispatcher(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_dispatcher;
    }

    [[nodiscard]] static std::optional<PostUpdateStepEvidence>
    post_update(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_post_update;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 4> k_bodies{{
        {DynamicUpdateBodyRole::pair_resolver,
         0x14005B460ULL,
         0x14005B6E6ULL,
         646U,
         "cba77cf4bc20dedfbd590991012fa471e6b5ae02b9cec57c66fce69333a67972"},
        {DynamicUpdateBodyRole::category_compatibility,
         0x14005B6F0ULL,
         0x14005B7A8ULL,
         184U,
         "c6eaaea738b317c9d92b3e06ff4ba47ec2d39610cb16a19dc21a96651024ae34"},
        {DynamicUpdateBodyRole::dispatcher,
         0x14005B7B0ULL,
         0x14005B8DDULL,
         301U,
         "ea1025e96121a3b117f53412d5d3e6439d81e97a23928f1417793e37ba6efbf3"},
        {DynamicUpdateBodyRole::post_update_step,
         0x14005B8E0ULL,
         0x14005BA64ULL,
         388U,
         "1d149357ffa3fe5024c0a6264e356f476c14c6366d89b38bb50a1a3f18e2f477"},
    }};

    inline static constexpr PairResolverAbiEvidence k_pair_resolver{
        0x14005B460ULL,
        1U,
        2U,
        3U,
        4U,
        5U,
        0x328U,
        0x00U,
        2U,
        0xD0U,
        0x10U,
        true,
        true,
        true,
        true,
        true,
    };

    inline static constexpr CategoryCompatibilityEvidence k_category_compatibility{
        0x14005B6F0ULL,
        0x0EU,
        0xDAU,
        2U,
        true,
        true,
        true,
    };

    inline static constexpr std::array<CategoryMaskBridge, 4> k_category_masks{{
        {0x02U, 0x0040U},
        {0x05U, 0x0002U},
        {0x08U, 0x0010U},
        {0x0BU, 0x0020U},
    }};

    inline static constexpr std::array<DispatcherBindingEvidence, 6> k_dispatcher_bindings{{
        {0x00001000U, 0x10U, 0x02U},
        {0x00002000U, 0x28U, 0x05U},
        {0x00004000U, 0x40U, 0x08U},
        {0x00008000U, 0x58U, 0x0BU},
        {0x00010000U, 0x70U, 0x0EU},
        {0x00020000U, 0x88U, 0x11U},
    }};

    inline static constexpr DynamicUpdateDispatcherEvidence k_dispatcher{
        0x14005B7B0ULL,
        0x14005B460ULL,
        0x14005B8E0ULL,
        true,
        true,
        true,
    };

    inline static constexpr PostUpdateStepEvidence k_post_update{
        0x14005B8E0ULL,
        0x10U,
        0x00020000U,
        true,
        true,
        true,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, DynamicUpdateBodyRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<PairResolverAbiEvidence>
pair_resolver_abi(std::string_view sha256) noexcept {
    return detail::EvidenceStore::pair_resolver(sha256);
}

[[nodiscard]] inline std::optional<CategoryCompatibilityEvidence>
category_compatibility_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::category_compatibility(sha256);
}

[[nodiscard]] inline std::optional<CategoryMaskBridge>
category_mask_bridge(std::string_view sha256, std::uint32_t category_id) noexcept {
    return detail::EvidenceStore::category_mask(sha256, category_id);
}

[[nodiscard]] inline std::optional<DispatcherBindingEvidence>
dispatcher_binding(std::string_view sha256, std::size_t index) noexcept {
    return detail::EvidenceStore::dispatcher_binding(sha256, index);
}

[[nodiscard]] inline std::optional<DynamicUpdateDispatcherEvidence>
dispatcher_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::dispatcher(sha256);
}

[[nodiscard]] inline std::optional<PostUpdateStepEvidence>
post_update_step_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::post_update(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_dynamic_update_evidence

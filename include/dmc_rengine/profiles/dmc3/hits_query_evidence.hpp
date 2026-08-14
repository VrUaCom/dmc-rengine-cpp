#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_evidence {

// DMC3 HD canonical-build profile evidence only. These descriptors are not a
// universal HITS ABI and intentionally do not model one monolithic original-
// game CollisionResult.
[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class QueryVariant : std::uint8_t {
    vertical_probe_05e460,
    generic_combined_05e7a0,
    inout_correction_05ebe0,
    bool_observed_05ee40,
    inout_correction_extended_05f070,
    local_cell_05fd10,
    source_selectable_segment_05fec0,
    positional_correction_0601e0,
    local_list_060790,
};

enum class SpecializedAbiKind : std::uint8_t {
    unresolved,
    bool_observed_in_al,
    mutable_vec4_inout,
    mutable_vec4_inout_plus_aux,
    combined_point_query,
};

struct QueryEvidenceDescriptor final {
    QueryVariant variant{};
    std::uint64_t function_va{};
    std::uint32_t direct_static_caller_count{};
    SpecializedAbiKind abi_kind{SpecializedAbiKind::unresolved};
    bool success_observed_in_al{};
    bool mutable_16_byte_inout_confirmed{};
    bool object_f0_argument_confirmed{};
    std::optional<std::uint32_t> independently_rejected_raw_flag_bit;
};

struct DynamicCategoryBinding final {
    std::uint32_t activation_flag{};
    std::uint32_t manager_field_offset{};
    std::uint32_t category_id{};
    std::uint16_t dispatcher_static_hits_reject_mask{};
};

enum class WrapperSourceIndex : std::uint8_t {
    stage_member_3 = 0,
    stage_member_6 = 1,
    external_global_source = 2,
};

struct WrapperSourceEvidence final {
    WrapperSourceIndex source{};
    bool structurally_present{};
    bool stage_pac_backed_confirmed{};
    bool direct_static_selection_confirmed{};
};

struct SourceSelectorCallsiteEvidence final {
    std::uint64_t callsite_va{};
    WrapperSourceIndex requested_source{};
};

struct TemporarySource1QueryPathEvidence final {
    std::uint64_t select_callsite_va{};
    QueryVariant query_variant{};
    std::uint64_t restore_callsite_va{};
};

enum class CanonicalBodyRole : std::uint8_t {
    combined_query_wrapper,
    static_hits_pass,
    dynamic_category_pass,
};

struct CanonicalFunctionBodyEvidence final {
    CanonicalBodyRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct CombinedQueryAbiEvidence final {
    std::uint64_t function_va{};
    std::uint8_t argument_count{};
    std::uint8_t runtime_wrapper_arg_index{};
    std::uint8_t reference_point_16_byte_arg_index{};
    std::uint8_t working_point_16_byte_arg_index{};
    std::uint8_t output_point_16_byte_arg_index{};
    std::uint8_t optional_hit_metadata_arg_index{};
    std::uint8_t raw_reject_mask_arg_index{};
    bool returns_any_hit_in_al{};
    bool working_and_output_may_alias{};
    bool total_miss_copies_working_point_to_output{};
    bool downstream_passes_share_progressive_working_point{};
};

enum class CombinedQueryPassKind : std::uint8_t {
    static_hits,
    dynamic_category,
};

struct CombinedQueryPassEvidence final {
    std::uint8_t order{};
    CombinedQueryPassKind kind{};
    std::uint64_t helper_va{};
    std::optional<std::uint32_t> dynamic_category_id;
};

struct CombinedHitMetadataEvidence final {
    std::uint32_t size_bytes{};
    bool static_hit_copies_complete_record{};
    bool dynamic_hit_writes_compatible_partial_record{};
    std::uint32_t dynamic_identity_source_offset{};
    std::uint32_t dynamic_identity_output_offset{};
    std::uint32_t dynamic_vector_output_offset{};
    std::uint32_t dynamic_vector_component_count{};
};

struct DynamicCategoryQueryEvidence final {
    std::uint64_t function_va{};
    std::uint8_t category_index_arg_index{};
    std::uint8_t raw_reject_mask_arg_index{};
    std::uint8_t mode_byte_arg_index{};
    std::uint32_t linked_next_offset{};
    std::uint32_t primitive_type_offset{};
    std::uint32_t raw_flags_offset{};
    std::uint32_t raw_flags_width_bytes{};
    std::uint32_t identity_source_offset{};
    std::uint32_t minimum_observed_primitive_type{};
    std::uint32_t maximum_observed_primitive_type{};
    bool category_indexes_pointer_table_directly{};
    bool accepted_hit_updates_working_point{};
    bool accepted_hit_writes_output_point{};
};

enum class RuntimeHelperObservedRole : std::uint8_t {
    hits_runtime_initializer,
    hits_runtime_teardown,
    source_binder,
    source_selector,
    candidate_cell_collector,
    cell_list_resolver,
    record_resolver,
    plane_evaluator,
    normal_classifier,
};

struct RuntimeHelperEvidence final {
    RuntimeHelperObservedRole observed_role{};
    std::uint64_t function_va{};
    std::uint32_t static_call_count{};
};

namespace detail {

// The backing tables are private class members rather than namespace-level
// constants. "detail" is a naming convention, not C++ access control; keeping
// the tables private prevents callers from bypassing the exact-build SHA gate
// through a directly addressable k_* table symbol.
class EvidenceStore final {
public:
    [[nodiscard]] static std::size_t query_count(std::string_view sha256) noexcept {
        return matches_canonical_target(sha256) ? k_query_evidence.size() : 0U;
    }

    [[nodiscard]] static std::optional<QueryEvidenceDescriptor>
    query(std::string_view sha256, QueryVariant variant) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_query_evidence) {
            if (descriptor.variant == variant) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<DynamicCategoryBinding>
    dynamic_category(std::string_view sha256, std::uint32_t category_id) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& binding : k_dynamic_category_bindings) {
            if (binding.category_id == category_id) {
                return binding;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<DynamicCategoryBinding>
    dynamic_category_from_activation(std::string_view sha256,
                                     std::uint32_t activation_flag) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& binding : k_dynamic_category_bindings) {
            if (binding.activation_flag == activation_flag) {
                return binding;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<WrapperSourceEvidence>
    wrapper_source(std::string_view sha256, WrapperSourceIndex source) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_wrapper_sources) {
            if (descriptor.source == source) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<SourceSelectorCallsiteEvidence>
    direct_selector_callsite(std::string_view sha256, std::size_t index) noexcept {
        if (!matches_canonical_target(sha256) || index >= k_direct_source_selector_callsites.size()) {
            return std::nullopt;
        }
        return k_direct_source_selector_callsites[index];
    }

    [[nodiscard]] static std::optional<TemporarySource1QueryPathEvidence>
    temporary_source1_path(std::string_view sha256, std::size_t index) noexcept {
        if (!matches_canonical_target(sha256) || index >= k_temporary_source1_query_paths.size()) {
            return std::nullopt;
        }
        return k_temporary_source1_query_paths[index];
    }

    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    canonical_body(std::string_view sha256, CanonicalBodyRole role) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_canonical_function_bodies) {
            if (descriptor.role == role) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<CombinedQueryAbiEvidence>
    combined_query_abi(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_combined_query_abi;
    }

    [[nodiscard]] static std::optional<CombinedQueryPassEvidence>
    combined_query_pass(std::string_view sha256, std::size_t index) noexcept {
        if (!matches_canonical_target(sha256) || index >= k_combined_query_passes.size()) {
            return std::nullopt;
        }
        return k_combined_query_passes[index];
    }

    [[nodiscard]] static std::optional<CombinedHitMetadataEvidence>
    combined_hit_metadata(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_combined_hit_metadata;
    }

    [[nodiscard]] static std::optional<DynamicCategoryQueryEvidence>
    dynamic_category_query(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_dynamic_category_query;
    }

    [[nodiscard]] static std::optional<RuntimeHelperEvidence>
    runtime_helper(std::string_view sha256, RuntimeHelperObservedRole observed_role) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_runtime_helper_evidence) {
            if (descriptor.observed_role == observed_role) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

private:
    inline static constexpr std::array<QueryEvidenceDescriptor, 9> k_query_evidence{{
        {QueryVariant::vertical_probe_05e460, 0x14005E460ULL, 1U,
         SpecializedAbiKind::unresolved, false, false, false, std::nullopt},
        {QueryVariant::generic_combined_05e7a0, 0x14005E7A0ULL, 51U,
         SpecializedAbiKind::combined_point_query, true, false, false, std::nullopt},
        {QueryVariant::inout_correction_05ebe0, 0x14005EBE0ULL, 1U,
         SpecializedAbiKind::mutable_vec4_inout, true, true, false, std::nullopt},
        {QueryVariant::bool_observed_05ee40, 0x14005EE40ULL, 1U,
         SpecializedAbiKind::bool_observed_in_al, true, false, false, std::nullopt},
        {QueryVariant::inout_correction_extended_05f070, 0x14005F070ULL, 1U,
         SpecializedAbiKind::mutable_vec4_inout_plus_aux, true, true, true, std::nullopt},
        {QueryVariant::local_cell_05fd10, 0x14005FD10ULL, 1U,
         SpecializedAbiKind::unresolved, false, false, false, std::nullopt},
        {QueryVariant::source_selectable_segment_05fec0, 0x14005FEC0ULL, 1U,
         SpecializedAbiKind::unresolved, false, false, false, 0x00080000U},
        {QueryVariant::positional_correction_0601e0, 0x1400601E0ULL, 5U,
         SpecializedAbiKind::unresolved, false, false, false, std::nullopt},
        {QueryVariant::local_list_060790, 0x140060790ULL, 1U,
         SpecializedAbiKind::bool_observed_in_al, true, false, false, std::nullopt},
    }};

    inline static constexpr std::array<DynamicCategoryBinding, 6> k_dynamic_category_bindings{{
        {0x00001000U, 0x10U, 0x02U, 0x0040U},
        {0x00002000U, 0x28U, 0x05U, 0x0002U},
        {0x00004000U, 0x40U, 0x08U, 0x0010U},
        {0x00008000U, 0x58U, 0x0BU, 0x0020U},
        {0x00010000U, 0x70U, 0x0EU, 0x0000U},
        {0x00020000U, 0x88U, 0x11U, 0x0000U},
    }};

    inline static constexpr std::array<WrapperSourceEvidence, 3> k_wrapper_sources{{
        {WrapperSourceIndex::stage_member_3, true, true, true},
        {WrapperSourceIndex::stage_member_6, true, true, true},
        {WrapperSourceIndex::external_global_source, true, false, false},
    }};

    inline static constexpr std::array<SourceSelectorCallsiteEvidence, 4>
        k_direct_source_selector_callsites{{
            {0x140056832ULL, WrapperSourceIndex::stage_member_6},
            {0x14005686EULL, WrapperSourceIndex::stage_member_3},
            {0x1400568F0ULL, WrapperSourceIndex::stage_member_6},
            {0x140056936ULL, WrapperSourceIndex::stage_member_3},
        }};

    inline static constexpr std::array<TemporarySource1QueryPathEvidence, 2>
        k_temporary_source1_query_paths{{
            {0x140056832ULL, QueryVariant::positional_correction_0601e0, 0x14005686EULL},
            {0x1400568F0ULL, QueryVariant::source_selectable_segment_05fec0, 0x140056936ULL},
        }};

    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 3>
        k_canonical_function_bodies{{
            {CanonicalBodyRole::combined_query_wrapper, 0x14005E7A0ULL, 0x14005E880ULL, 224U,
             "3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f"},
            {CanonicalBodyRole::static_hits_pass, 0x14005E880ULL, 0x14005EB95ULL, 789U,
             "b3fdeac674795752492e1eca9e7a9d21837552aa6cf90f0a02a14e3546136e8a"},
            {CanonicalBodyRole::dynamic_category_pass, 0x14005BCF0ULL, 0x14005C0D6ULL, 998U,
             "e7e1c1a56425e0a3c5ef5a5a4fff9105d5e073effeae6fcb268829ec3b451d02"},
        }};

    inline static constexpr CombinedQueryAbiEvidence k_combined_query_abi{
        0x14005E7A0ULL,
        6U,
        1U,
        2U,
        3U,
        4U,
        5U,
        6U,
        true,
        true,
        true,
        true,
    };

    inline static constexpr std::array<CombinedQueryPassEvidence, 3> k_combined_query_passes{{
        {0U, CombinedQueryPassKind::static_hits, 0x14005E880ULL, std::nullopt},
        {1U, CombinedQueryPassKind::dynamic_category, 0x14005BCF0ULL, 0x0EU},
        {2U, CombinedQueryPassKind::dynamic_category, 0x14005BCF0ULL, 0x11U},
    }};

    inline static constexpr CombinedHitMetadataEvidence k_combined_hit_metadata{
        0x38U,
        true,
        true,
        0xD8U,
        0x00U,
        0x28U,
        3U,
    };

    inline static constexpr DynamicCategoryQueryEvidence k_dynamic_category_query{
        0x14005BCF0ULL,
        6U,
        7U,
        8U,
        0x328U,
        0x120U,
        0xDAU,
        2U,
        0xD8U,
        2U,
        6U,
        true,
        true,
        true,
    };

    inline static constexpr std::array<RuntimeHelperEvidence, 9> k_runtime_helper_evidence{{
        {RuntimeHelperObservedRole::hits_runtime_initializer, 0x1402D3060ULL, 2U},
        {RuntimeHelperObservedRole::hits_runtime_teardown, 0x1402D29C0ULL, 2U},
        {RuntimeHelperObservedRole::source_binder, 0x14005EBA0ULL, 1U},
        {RuntimeHelperObservedRole::source_selector, 0x14005EBC0ULL, 4U},
        {RuntimeHelperObservedRole::candidate_cell_collector, 0x1402D2A10ULL, 7U},
        {RuntimeHelperObservedRole::cell_list_resolver, 0x1402D29F0ULL, 13U},
        {RuntimeHelperObservedRole::record_resolver, 0x1402D3050ULL, 13U},
        {RuntimeHelperObservedRole::plane_evaluator, 0x1402CF820ULL, 6U},
        {RuntimeHelperObservedRole::normal_classifier, 0x1402CCF20ULL, 6U},
    }};
};

} // namespace detail

[[nodiscard]] inline std::size_t query_evidence_count(std::string_view sha256) noexcept {
    return detail::EvidenceStore::query_count(sha256);
}

[[nodiscard]] inline std::optional<QueryEvidenceDescriptor>
query_evidence(std::string_view sha256, QueryVariant variant) noexcept {
    return detail::EvidenceStore::query(sha256, variant);
}

[[nodiscard]] inline std::optional<DynamicCategoryBinding>
dynamic_category_binding(std::string_view sha256, std::uint32_t category_id) noexcept {
    return detail::EvidenceStore::dynamic_category(sha256, category_id);
}

[[nodiscard]] inline std::optional<DynamicCategoryBinding>
dynamic_category_binding_from_activation(std::string_view sha256,
                                         std::uint32_t activation_flag) noexcept {
    return detail::EvidenceStore::dynamic_category_from_activation(sha256, activation_flag);
}

[[nodiscard]] inline std::optional<WrapperSourceEvidence>
wrapper_source_evidence(std::string_view sha256, WrapperSourceIndex source) noexcept {
    return detail::EvidenceStore::wrapper_source(sha256, source);
}

[[nodiscard]] inline std::optional<SourceSelectorCallsiteEvidence>
direct_source_selector_callsite(std::string_view sha256, std::size_t index) noexcept {
    return detail::EvidenceStore::direct_selector_callsite(sha256, index);
}

[[nodiscard]] inline std::optional<TemporarySource1QueryPathEvidence>
temporary_source1_query_path(std::string_view sha256, std::size_t index) noexcept {
    return detail::EvidenceStore::temporary_source1_path(sha256, index);
}

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, CanonicalBodyRole role) noexcept {
    return detail::EvidenceStore::canonical_body(sha256, role);
}

[[nodiscard]] inline std::optional<CombinedQueryAbiEvidence>
combined_query_abi_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::combined_query_abi(sha256);
}

[[nodiscard]] inline std::optional<CombinedQueryPassEvidence>
combined_query_pass_evidence(std::string_view sha256, std::size_t index) noexcept {
    return detail::EvidenceStore::combined_query_pass(sha256, index);
}

[[nodiscard]] inline std::optional<CombinedHitMetadataEvidence>
combined_hit_metadata_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::combined_hit_metadata(sha256);
}

[[nodiscard]] inline std::optional<DynamicCategoryQueryEvidence>
dynamic_category_query_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::dynamic_category_query(sha256);
}

[[nodiscard]] inline std::optional<RuntimeHelperEvidence>
runtime_helper_evidence(std::string_view sha256,
                        RuntimeHelperObservedRole observed_role) noexcept {
    return detail::EvidenceStore::runtime_helper(sha256, observed_role);
}

} // namespace dmc::rengine::profiles::dmc3::hits_evidence

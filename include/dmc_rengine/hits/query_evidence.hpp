#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace dmc::rengine::hits::evidence {

// Evidence-only identities for the original DMC3 HD query family.
// These descriptors intentionally do not model or imply one universal
// original-game CollisionResult structure.
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
};

struct QueryEvidenceDescriptor final {
    QueryVariant variant{};
    std::uint64_t function_va{};
    SpecializedAbiKind abi_kind{SpecializedAbiKind::unresolved};
    bool success_observed_in_al{};
    bool mutable_16_byte_inout_confirmed{};
    bool object_f0_argument_confirmed{};
};

inline constexpr std::array<QueryEvidenceDescriptor, 9> k_query_evidence{{
    {QueryVariant::vertical_probe_05e460, 0x14005E460ULL,
     SpecializedAbiKind::unresolved, false, false, false},
    {QueryVariant::generic_combined_05e7a0, 0x14005E7A0ULL,
     SpecializedAbiKind::unresolved, false, false, false},
    {QueryVariant::inout_correction_05ebe0, 0x14005EBE0ULL,
     SpecializedAbiKind::mutable_vec4_inout, true, true, false},
    {QueryVariant::bool_observed_05ee40, 0x14005EE40ULL,
     SpecializedAbiKind::bool_observed_in_al, true, false, false},
    {QueryVariant::inout_correction_extended_05f070, 0x14005F070ULL,
     SpecializedAbiKind::mutable_vec4_inout_plus_aux, true, true, true},
    {QueryVariant::local_cell_05fd10, 0x14005FD10ULL,
     SpecializedAbiKind::unresolved, false, false, false},
    {QueryVariant::source_selectable_segment_05fec0, 0x14005FEC0ULL,
     SpecializedAbiKind::unresolved, false, false, false},
    {QueryVariant::positional_correction_0601e0, 0x1400601E0ULL,
     SpecializedAbiKind::unresolved, false, false, false},
    {QueryVariant::local_list_060790, 0x140060790ULL,
     SpecializedAbiKind::bool_observed_in_al, true, false, false},
}};

[[nodiscard]] constexpr std::optional<QueryEvidenceDescriptor>
query_evidence(QueryVariant variant) noexcept {
    for (const auto& descriptor : k_query_evidence) {
        if (descriptor.variant == variant) {
            return descriptor;
        }
    }
    return std::nullopt;
}

struct DynamicCategoryBinding final {
    std::uint32_t activation_flag{};
    std::uint32_t manager_field_offset{};
    std::uint32_t category_id{};
    std::uint16_t static_hits_reject_mask{};
};

inline constexpr std::array<DynamicCategoryBinding, 6> k_dynamic_category_bindings{{
    {0x00001000U, 0x10U, 0x02U, 0x0040U},
    {0x00002000U, 0x28U, 0x05U, 0x0002U},
    {0x00004000U, 0x40U, 0x08U, 0x0010U},
    {0x00008000U, 0x58U, 0x0BU, 0x0020U},
    {0x00010000U, 0x70U, 0x0EU, 0x0000U},
    {0x00020000U, 0x88U, 0x11U, 0x0000U},
}};

[[nodiscard]] constexpr std::optional<DynamicCategoryBinding>
dynamic_category_binding(std::uint32_t category_id) noexcept {
    for (const auto& binding : k_dynamic_category_bindings) {
        if (binding.category_id == category_id) {
            return binding;
        }
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<DynamicCategoryBinding>
dynamic_category_binding_from_activation(std::uint32_t activation_flag) noexcept {
    for (const auto& binding : k_dynamic_category_bindings) {
        if (binding.activation_flag == activation_flag) {
            return binding;
        }
    }
    return std::nullopt;
}

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

inline constexpr std::array<WrapperSourceEvidence, 3> k_wrapper_sources{{
    {WrapperSourceIndex::stage_member_3, true, true, true},
    {WrapperSourceIndex::stage_member_6, true, true, true},
    {WrapperSourceIndex::external_global_source, true, false, false},
}};

} // namespace dmc::rengine::hits::evidence

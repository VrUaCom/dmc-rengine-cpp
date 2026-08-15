#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_stage_cfg_pac_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

struct StageCfgResourceSelectionEvidence final {
    std::uint64_t resource_selector_va{};
    std::uint32_t resource_kind{};
    std::uint64_t stage_resource_group_table_va{};
    std::uint32_t stage_group_divisor{};
    std::uint32_t stage_cfg_resource_subrecord_offset_in_row{};
    std::uint32_t stage_cfg_filename_pointer_offset_in_row{};
    std::string_view filename_pattern{};
};

struct PacContainerLayoutEvidence final {
    std::uint32_t slot_count_offset{};
    std::uint32_t slot_offset_table_offset{};
    std::uint32_t slot_offset_width_bytes{};
};

struct StageCfgCollisionSlotGenerationEvidence final {
    std::string_view generation_name{};
    std::uint32_t related_source_slot{};
    std::uint32_t entry_table_slot{};
    std::uint32_t primitive_descriptor_table_slot{};
    std::uint32_t minimum_slot_count_for_source{};
    std::uint32_t minimum_slot_count_for_entry_table{};
    std::uint32_t minimum_slot_count_for_descriptor_table{};
    std::uint32_t source_offset_field{};
    std::uint32_t entry_table_offset_field{};
    std::uint32_t descriptor_table_offset_field{};
    std::uint64_t observed_c260_callsite_va{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<StageCfgResourceSelectionEvidence>
    stage_cfg_selection(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_stage_cfg_selection;
    }

    [[nodiscard]] static std::optional<PacContainerLayoutEvidence>
    pac_layout(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_pac_layout;
    }

    [[nodiscard]] static std::optional<StageCfgCollisionSlotGenerationEvidence>
    collision_slots(std::string_view sha256, std::size_t index) noexcept {
        if (!matches_canonical_target(sha256) || index >= k_collision_slot_generations.size()) {
            return std::nullopt;
        }
        return k_collision_slot_generations[index];
    }

private:
    inline static constexpr StageCfgResourceSelectionEvidence k_stage_cfg_selection{
        0x1401AE310ULL,
        1U,
        0x1405C4A50ULL,
        100U,
        0x10U,
        0x18U,
        "room\\stXXXcfg.pac",
    };

    inline static constexpr PacContainerLayoutEvidence k_pac_layout{
        0x04U,
        0x08U,
        4U,
    };

    inline static constexpr std::array<StageCfgCollisionSlotGenerationEvidence, 2>
        k_collision_slot_generations{{
            {
                "modern_cem_stage_cfg",
                38U,
                39U,
                40U,
                39U,
                40U,
                41U,
                0xA0U,
                0xA4U,
                0xA8U,
                0x14009823FULL,
            },
            {
                "legacy_cem008_stage_cfg",
                21U,
                22U,
                23U,
                22U,
                23U,
                24U,
                0x5CU,
                0x60U,
                0x64U,
                0x1400B6483ULL,
            },
        }};
};

} // namespace detail

[[nodiscard]] inline std::optional<StageCfgResourceSelectionEvidence>
stage_cfg_resource_selection_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::stage_cfg_selection(sha256);
}

[[nodiscard]] inline std::optional<PacContainerLayoutEvidence>
pac_container_layout_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::pac_layout(sha256);
}

[[nodiscard]] inline std::optional<StageCfgCollisionSlotGenerationEvidence>
stage_cfg_collision_slot_generation_evidence(std::string_view sha256,
                                             std::size_t index) noexcept {
    return detail::EvidenceStore::collision_slots(sha256, index);
}

} // namespace dmc::rengine::profiles::dmc3::hits_stage_cfg_pac_evidence

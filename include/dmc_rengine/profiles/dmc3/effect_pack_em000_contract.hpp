#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::profiles::dmc3 {

// Corpus-bound structural and grouping evidence for em000_041.pnst.
//
// This contract is intentionally kept separate from the older st001/st114
// one-line/one-payload EffectPackContract until the canonical parser is
// generalized and covered by real-corpus tests. Names below describe byte
// structure and manifest-domain relationships only; they do not expand the
// historical meaning of the G/V/E/P/T/A/M letters.
struct EffectPackEm000Contract final {
    static constexpr std::size_t manifest_entry_count = 173U;
    static constexpr std::size_t inner_declared_slot_count = 186U;
    static constexpr std::size_t inner_populated_slot_count = 183U;

    enum class GroupShape : std::uint8_t {
        single_member,
        mod_with_optional_companion,
        unknown,
    };

    struct KindObservation final {
        char kind{};
        std::size_t manifest_count{};
        GroupShape shape{GroupShape::unknown};
    };

    static constexpr std::array<KindObservation, 7> kinds{
        KindObservation{'G', 12U, GroupShape::single_member},
        KindObservation{'V', 50U, GroupShape::single_member},
        KindObservation{'E', 45U, GroupShape::single_member},
        KindObservation{'P', 34U, GroupShape::single_member},
        KindObservation{'T', 8U, GroupShape::single_member},
        KindObservation{'A', 11U, GroupShape::single_member},
        KindObservation{'M', 13U, GroupShape::mod_with_optional_companion},
    };

    static constexpr std::array<std::size_t, 3> empty_mod_companion_slots{
        161U, 163U, 167U,
    };

    static constexpr std::size_t mod_group_first_slot = 160U;
    static constexpr std::size_t mod_group_slot_stride = 2U;
    static constexpr std::size_t mod_optional_companion_size = 16U;
    static constexpr std::uint32_t mod_optional_companion_first_u32 = 0x31U;

    static constexpr std::size_t g_extent = 96U;
    static constexpr std::size_t v_extent = 368U;
    static constexpr std::size_t e_extent = 544U;
    static constexpr std::size_t a_extent = 336U;

    // E record envelope: all 45 bound records begin with byte 0x09 and use
    // subtype selector 1, 2, or 5. The high-level effect meaning is open.
    struct ERecord final {
        static constexpr std::size_t type_byte_offset = 0x00U;
        static constexpr std::size_t subtype_offset = 0x01U;
        static constexpr std::size_t raw_modifier_offset = 0x02U;
        static constexpr std::size_t t_selector_raw_offset = 0x04U;
        static constexpr std::size_t a_selector_raw_offset = 0x08U;
        static constexpr std::size_t m_selector_raw_offset = 0x28U;

        static constexpr std::uint8_t observed_type_byte = 0x09U;
        static constexpr std::uint8_t subtype_1 = 1U;
        static constexpr std::uint8_t subtype_2 = 2U;
        static constexpr std::uint8_t subtype_5 = 5U;
        static constexpr std::uint16_t observed_no_selector = 0xFFFFU;

        static constexpr std::size_t subtype_1_modifier_0000_count = 8U;
        static constexpr std::size_t subtype_1_modifier_ffff_count = 7U;
        static constexpr std::size_t subtype_2_modifier_0000_count = 10U;
        static constexpr std::size_t subtype_2_modifier_ffff_count = 1U;
        static constexpr std::size_t subtype_5_modifier_0000_count = 19U;

        [[nodiscard]] static constexpr std::uint16_t low16(
            std::uint32_t raw) noexcept {
            return static_cast<std::uint16_t>(raw & 0xFFFFU);
        }
    };

    // P is explicitly variable-size in em000. These are observations, not an
    // exhaustive universal subtype table.
    static constexpr std::array<std::size_t, 4> observed_p_extents{
        336U, 528U, 704U, 896U,
    };

    struct PRecord final {
        static constexpr std::size_t relative_table_base = 0x10U;
        static constexpr std::size_t table_span_field = 0x10U;
        static constexpr std::size_t primary_block_size = 0x130U;
        static constexpr std::size_t embedded_name_offset_in_primary = 0x02U;
        static constexpr std::size_t auxiliary_header_size = 0x10U;
        static constexpr std::size_t auxiliary_block_size = 0xB0U;

        static constexpr std::size_t aux_count_0_records = 21U;
        static constexpr std::size_t aux_count_1_records = 10U;
        static constexpr std::size_t aux_count_2_records = 1U;
        static constexpr std::size_t aux_count_3_records = 2U;

        [[nodiscard]] static constexpr std::size_t align16(
            std::size_t value) noexcept {
            return (value + 0x0FU) & ~std::size_t{0x0FU};
        }

        // The table contains N+2 non-zero entries and is padded to 16 bytes.
        [[nodiscard]] static constexpr std::size_t table_span_for_aux_count(
            std::size_t auxiliary_block_count) noexcept {
            return align16((auxiliary_block_count + 2U) * sizeof(std::uint32_t));
        }

        [[nodiscard]] static constexpr std::size_t expected_extent(
            std::size_t auxiliary_block_count) noexcept {
            const auto table_span = table_span_for_aux_count(auxiliary_block_count);
            const auto fixed = relative_table_base + table_span + primary_block_size;
            if (auxiliary_block_count == 0U) {
                return fixed;
            }
            return fixed + auxiliary_header_size +
                auxiliary_block_count * auxiliary_block_size;
        }
    };

    static constexpr std::size_t wrapped_texture_descriptor_size = 0x70U;

    [[nodiscard]] static constexpr GroupShape shape_for(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return entry.shape;
            }
        }
        return GroupShape::unknown;
    }
};

static_assert(EffectPackEm000Contract::manifest_entry_count ==
    12U + 50U + 45U + 34U + 8U + 11U + 13U);
static_assert(EffectPackEm000Contract::inner_populated_slot_count ==
    EffectPackEm000Contract::inner_declared_slot_count - 3U);

static_assert(
    EffectPackEm000Contract::ERecord::subtype_1_modifier_0000_count +
    EffectPackEm000Contract::ERecord::subtype_1_modifier_ffff_count +
    EffectPackEm000Contract::ERecord::subtype_2_modifier_0000_count +
    EffectPackEm000Contract::ERecord::subtype_2_modifier_ffff_count +
    EffectPackEm000Contract::ERecord::subtype_5_modifier_0000_count == 45U);

static_assert(EffectPackEm000Contract::PRecord::expected_extent(0U) == 336U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(1U) == 528U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(2U) == 704U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(3U) == 896U);
static_assert(
    EffectPackEm000Contract::PRecord::aux_count_0_records +
    EffectPackEm000Contract::PRecord::aux_count_1_records +
    EffectPackEm000Contract::PRecord::aux_count_2_records +
    EffectPackEm000Contract::PRecord::aux_count_3_records == 34U);

} // namespace dmc::rengine::profiles::dmc3

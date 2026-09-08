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

    // Shared directed-edge selector grammar recovered independently in all
    // bound G and V records. The low byte chooses a manifest kind and the next
    // u16 stores an id from exactly that kind domain. The high byte is raw.
    struct DirectedSelector final {
        enum class TargetKind : std::uint8_t {
            p = 0U,
            e = 1U,
            g = 2U,
            v = 3U,
            unknown = 0xFFU,
        };

        struct ModifierCount final {
            std::uint8_t value{};
            std::size_t count{};
        };

        static constexpr std::size_t v_packed_selector_offset = 0x04U;
        static constexpr std::size_t v_target_id_offset = 0x06U;
        static constexpr std::size_t g_packed_selector_offset = 0x24U;
        static constexpr std::size_t g_target_id_offset = 0x26U;

        static constexpr std::size_t observed_v_edge_count = 50U;
        static constexpr std::size_t observed_g_edge_count = 12U;

        static constexpr std::size_t v_to_p_count = 27U;
        static constexpr std::size_t v_to_e_count = 11U;
        static constexpr std::size_t v_to_g_count = 8U;
        static constexpr std::size_t v_to_v_count = 4U;
        static constexpr std::size_t g_to_p_count = 6U;
        static constexpr std::size_t g_to_e_count = 4U;
        static constexpr std::size_t g_to_v_count = 2U;

        static constexpr std::size_t bound_terminal_p_chain_count = 41U;
        static constexpr std::size_t bound_terminal_e_chain_count = 21U;
        static constexpr std::size_t bound_max_chain_edges = 2U;

        static constexpr std::size_t unique_p_records_reached = 18U;
        static constexpr std::size_t unique_e_records_reached = 15U;
        static constexpr std::size_t total_p_records = 34U;
        static constexpr std::size_t total_e_records = 45U;

        // Complete high-byte census for the bound em000 graph. These values
        // are deliberately not interpreted as runtime modes/flags yet.
        static constexpr std::array<ModifierCount, 6> g_modifier_histogram{
            ModifierCount{0U, 2U},
            ModifierCount{48U, 3U},
            ModifierCount{76U, 1U},
            ModifierCount{202U, 2U},
            ModifierCount{252U, 2U},
            ModifierCount{255U, 2U},
        };

        static constexpr std::array<ModifierCount, 8> v_modifier_histogram{
            ModifierCount{0U, 17U},
            ModifierCount{1U, 3U},
            ModifierCount{17U, 1U},
            ModifierCount{82U, 1U},
            ModifierCount{206U, 14U},
            ModifierCount{208U, 1U},
            ModifierCount{238U, 4U},
            ModifierCount{255U, 9U},
        };

        [[nodiscard]] static constexpr TargetKind target_kind(
            std::uint16_t packed_selector) noexcept {
            switch (static_cast<std::uint8_t>(packed_selector & 0x00FFU)) {
            case 0U: return TargetKind::p;
            case 1U: return TargetKind::e;
            case 2U: return TargetKind::g;
            case 3U: return TargetKind::v;
            default: return TargetKind::unknown;
            }
        }

        [[nodiscard]] static constexpr std::uint8_t raw_modifier(
            std::uint16_t packed_selector) noexcept {
            return static_cast<std::uint8_t>(packed_selector >> 8U);
        }
    };

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

    // T is an 0x70-byte DMC wrapper followed by an embedded DDS. The fields
    // below are corpus-bound metric relations; runtime names remain open.
    struct TRecord final {
        static constexpr std::size_t observed_record_count = 8U;
        static constexpr std::size_t wrapper_extent = 0x70U;
        static constexpr std::size_t embedded_dds_magic_offset = 0x70U;

        static constexpr std::size_t packed_dimension_offset = 0x10U;
        static constexpr std::size_t width_times_four_offset = 0x18U;
        static constexpr std::size_t dds_data_extent_offset = 0x38U;
        static constexpr std::size_t duplicate_packed_dimension_offset = 0x44U;
        static constexpr std::size_t reciprocal_dimension_a_offset = 0x48U;
        static constexpr std::size_t reciprocal_dimension_b_offset = 0x4CU;
        static constexpr std::size_t complete_dds_extent_offset = 0x64U;

        static constexpr std::size_t observed_256_square_count = 1U;
        static constexpr std::size_t observed_128_square_count = 7U;
        static constexpr std::size_t observed_dxt5_count = 8U;

        static constexpr std::uint32_t packed_128_square = 0x00800080U;
        static constexpr std::uint32_t packed_256_square = 0x01000100U;
    };

    // A is a fixed 0x150-byte T-linked record in em000. The 33x0x0A entries
    // have rectangle-like 0..256 grid arithmetic, but the historical meaning
    // of A and the entry flags remains unpromoted until the EXE consumer is
    // recovered.
    struct ARecord final {
        static constexpr std::size_t constant_01_offset = 0x00U;
        static constexpr std::size_t t_selector_offset = 0x01U;
        static constexpr std::size_t raw_selector_a_offset = 0x02U;
        static constexpr std::size_t raw_selector_b_offset = 0x03U;
        static constexpr std::uint8_t observed_constant_01 = 1U;

        static constexpr std::size_t entry_base = 0x04U;
        static constexpr std::size_t entry_size = 0x0AU;
        static constexpr std::size_t entry_capacity = 33U;
        static constexpr std::size_t final_zero_tail_size = 0x02U;

        static constexpr std::size_t entry_raw_flags_offset = 0x00U;
        static constexpr std::size_t entry_grid_x_offset = 0x02U;
        static constexpr std::size_t entry_grid_y_offset = 0x04U;
        static constexpr std::size_t entry_grid_w_offset = 0x06U;
        static constexpr std::size_t entry_grid_h_offset = 0x08U;
        static constexpr std::uint16_t observed_grid_domain = 256U;
        static constexpr std::uint16_t observed_grid_quantum = 32U;

        static constexpr std::size_t observed_record_count = 11U;
        static constexpr std::size_t observed_nonzero_entry_count = 54U;
        static constexpr std::array<std::size_t, 4> observed_active_entry_counts{
            2U, 4U, 10U, 16U,
        };

        // 54/54 active entries map to exact integer physical pixel rectangles
        // through grid * selected_texture_dimension / 256. This is a corpus
        // relation, not yet an EXE-confirmed UV semantic.
        static constexpr std::size_t exact_scaled_pixel_entry_count = 54U;

        static constexpr std::size_t first_entry_flag_one_records = 9U;
        static constexpr std::size_t all_active_flags_zero_records = 2U;
        static constexpr std::uint8_t all_zero_flags_header_selector_b = 2U;
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

        struct PrimaryVariant final {
            std::uint8_t byte0{};
            std::uint8_t byte1{};
            std::size_t observed_count{};
        };

        static constexpr std::array<PrimaryVariant, 3> observed_primary_variants{
            PrimaryVariant{0x02U, 0x03U, 30U},
            PrimaryVariant{0x01U, 0x01U, 3U},
            PrimaryVariant{0x00U, 0x04U, 1U},
        };

        // Only the bound 02/03 primary variant uses these locations as
        // manifest selectors. The other observed variants contain different
        // data at +0xE0 and therefore must not be decoded through this layout.
        static constexpr std::uint8_t texture_variant_byte0 = 0x02U;
        static constexpr std::uint8_t texture_variant_byte1 = 0x03U;
        static constexpr std::size_t texture_variant_count = 30U;
        static constexpr std::size_t t_selector_low16_offset_in_primary = 0xE0U;
        static constexpr std::size_t a_selector_u16_offset_in_primary = 0x106U;

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
    EffectPackEm000Contract::DirectedSelector::v_to_p_count +
    EffectPackEm000Contract::DirectedSelector::v_to_e_count +
    EffectPackEm000Contract::DirectedSelector::v_to_g_count +
    EffectPackEm000Contract::DirectedSelector::v_to_v_count ==
    EffectPackEm000Contract::DirectedSelector::observed_v_edge_count);
static_assert(
    EffectPackEm000Contract::DirectedSelector::g_to_p_count +
    EffectPackEm000Contract::DirectedSelector::g_to_e_count +
    EffectPackEm000Contract::DirectedSelector::g_to_v_count ==
    EffectPackEm000Contract::DirectedSelector::observed_g_edge_count);
static_assert(
    EffectPackEm000Contract::DirectedSelector::bound_terminal_p_chain_count +
    EffectPackEm000Contract::DirectedSelector::bound_terminal_e_chain_count ==
    EffectPackEm000Contract::DirectedSelector::observed_v_edge_count +
    EffectPackEm000Contract::DirectedSelector::observed_g_edge_count);
static_assert(
    EffectPackEm000Contract::DirectedSelector::unique_p_records_reached <=
    EffectPackEm000Contract::DirectedSelector::total_p_records);
static_assert(
    EffectPackEm000Contract::DirectedSelector::unique_e_records_reached <=
    EffectPackEm000Contract::DirectedSelector::total_e_records);

static_assert(
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[0].count +
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[1].count +
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[2].count +
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[3].count +
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[4].count +
    EffectPackEm000Contract::DirectedSelector::g_modifier_histogram[5].count ==
    EffectPackEm000Contract::DirectedSelector::observed_g_edge_count);
static_assert(
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[0].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[1].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[2].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[3].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[4].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[5].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[6].count +
    EffectPackEm000Contract::DirectedSelector::v_modifier_histogram[7].count ==
    EffectPackEm000Contract::DirectedSelector::observed_v_edge_count);

static_assert(
    EffectPackEm000Contract::ERecord::subtype_1_modifier_0000_count +
    EffectPackEm000Contract::ERecord::subtype_1_modifier_ffff_count +
    EffectPackEm000Contract::ERecord::subtype_2_modifier_0000_count +
    EffectPackEm000Contract::ERecord::subtype_2_modifier_ffff_count +
    EffectPackEm000Contract::ERecord::subtype_5_modifier_0000_count == 45U);

static_assert(
    EffectPackEm000Contract::TRecord::observed_256_square_count +
    EffectPackEm000Contract::TRecord::observed_128_square_count ==
    EffectPackEm000Contract::TRecord::observed_record_count);
static_assert(
    EffectPackEm000Contract::TRecord::observed_dxt5_count ==
    EffectPackEm000Contract::TRecord::observed_record_count);

static_assert(
    0x04U +
    EffectPackEm000Contract::ARecord::entry_capacity *
        EffectPackEm000Contract::ARecord::entry_size +
    EffectPackEm000Contract::ARecord::final_zero_tail_size ==
    EffectPackEm000Contract::a_extent);
static_assert(
    EffectPackEm000Contract::ARecord::exact_scaled_pixel_entry_count ==
    EffectPackEm000Contract::ARecord::observed_nonzero_entry_count);
static_assert(
    EffectPackEm000Contract::ARecord::first_entry_flag_one_records +
    EffectPackEm000Contract::ARecord::all_active_flags_zero_records ==
    EffectPackEm000Contract::ARecord::observed_record_count);

static_assert(EffectPackEm000Contract::PRecord::expected_extent(0U) == 336U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(1U) == 528U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(2U) == 704U);
static_assert(EffectPackEm000Contract::PRecord::expected_extent(3U) == 896U);
static_assert(
    EffectPackEm000Contract::PRecord::aux_count_0_records +
    EffectPackEm000Contract::PRecord::aux_count_1_records +
    EffectPackEm000Contract::PRecord::aux_count_2_records +
    EffectPackEm000Contract::PRecord::aux_count_3_records == 34U);
static_assert(
    EffectPackEm000Contract::PRecord::observed_primary_variants[0].observed_count +
    EffectPackEm000Contract::PRecord::observed_primary_variants[1].observed_count +
    EffectPackEm000Contract::PRecord::observed_primary_variants[2].observed_count == 34U);

} // namespace dmc::rengine::profiles::dmc3

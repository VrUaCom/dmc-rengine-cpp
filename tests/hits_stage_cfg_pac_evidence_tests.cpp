#include "dmc_rengine/profiles/dmc3/hits_stage_cfg_pac_evidence.hpp"

#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::rengine::profiles::dmc3::hits_stage_cfg_pac_evidence;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    const auto selection = stage_cfg_resource_selection_evidence(canonical_sha);
    assert(selection.has_value());
    assert(selection->resource_selector_va == 0x1401AE310ULL);
    assert(selection->resource_kind == 1U);
    assert(selection->stage_resource_group_table_va == 0x1405C4A50ULL);
    assert(selection->stage_group_divisor == 100U);
    assert(selection->stage_cfg_resource_subrecord_offset_in_row == 0x10U);
    assert(selection->stage_cfg_filename_pointer_offset_in_row == 0x18U);
    assert(selection->filename_pattern == "room\\stXXXcfg.pac");

    const auto pac = pac_container_layout_evidence(canonical_sha);
    assert(pac.has_value());
    assert(pac->slot_count_offset == 0x04U);
    assert(pac->slot_offset_table_offset == 0x08U);
    assert(pac->slot_offset_width_bytes == 4U);

    const auto modern = stage_cfg_collision_slot_generation_evidence(canonical_sha, 0U);
    assert(modern.has_value());
    assert(modern->generation_name == "modern_cem_stage_cfg");
    assert(modern->related_source_slot == 38U);
    assert(modern->entry_table_slot == 39U);
    assert(modern->primitive_descriptor_table_slot == 40U);
    assert(modern->minimum_slot_count_for_source == 39U);
    assert(modern->minimum_slot_count_for_entry_table == 40U);
    assert(modern->minimum_slot_count_for_descriptor_table == 41U);
    assert(modern->source_offset_field == 0xA0U);
    assert(modern->entry_table_offset_field == 0xA4U);
    assert(modern->descriptor_table_offset_field == 0xA8U);
    assert(modern->observed_c260_callsite_va == 0x14009823FULL);

    const auto legacy = stage_cfg_collision_slot_generation_evidence(canonical_sha, 1U);
    assert(legacy.has_value());
    assert(legacy->generation_name == "legacy_cem008_stage_cfg");
    assert(legacy->related_source_slot == 21U);
    assert(legacy->entry_table_slot == 22U);
    assert(legacy->primitive_descriptor_table_slot == 23U);
    assert(legacy->minimum_slot_count_for_source == 22U);
    assert(legacy->minimum_slot_count_for_entry_table == 23U);
    assert(legacy->minimum_slot_count_for_descriptor_table == 24U);
    assert(legacy->source_offset_field == 0x5CU);
    assert(legacy->entry_table_offset_field == 0x60U);
    assert(legacy->descriptor_table_offset_field == 0x64U);
    assert(legacy->observed_c260_callsite_va == 0x1400B6483ULL);

    assert(!stage_cfg_collision_slot_generation_evidence(canonical_sha, 2U).has_value());
    assert(!stage_cfg_resource_selection_evidence(packed_sha).has_value());
    assert(!pac_container_layout_evidence(packed_sha).has_value());
    assert(!stage_cfg_collision_slot_generation_evidence(packed_sha, 0U).has_value());

    return 0;
}

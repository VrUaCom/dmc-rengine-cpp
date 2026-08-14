#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <cassert>

int main() {
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;
    using dmc::rengine::profiles::dmc3::wave2_stage_descriptor_universe;
    using dmc::rengine::profiles::dmc3::wave2_stage_resource_bank_a;

    const auto& descriptor = wave2_stage_resource_bank_a();
    assert(descriptor.valid());
    assert(&descriptor == &phase12_stage_resource_table());
    assert(descriptor.evidence_packet_id == "dmc3-stage-wave2");
    assert(descriptor.artifact_size == 6356432ULL);
    assert(descriptor.file_offset == 0x005C30A0ULL);
    assert(descriptor.rva == 0x005C4AA0U);
    assert(descriptor.va == 0x1405C4AA0ULL);
    assert(descriptor.row_count == 110U);
    assert(descriptor.cell_stride == 0x10U);
    assert(descriptor.kind16_offset == 0U);
    assert(descriptor.path_pointer_offset == 0x08U);
    assert(descriptor.entry_count() == 440U);
    assert(descriptor.table_size_bytes() == 0x1B80ULL);
    assert(descriptor.role_for_column(0U) == StageResourceRole::script);
    assert(descriptor.role_for_column(1U) == StageResourceRole::room_config);
    assert(descriptor.role_for_column(2U) == StageResourceRole::room_effects);
    assert(descriptor.role_for_column(3U) == StageResourceRole::room_sound);
    assert(!descriptor.role_for_column(4U).has_value());

    const auto& universe = wave2_stage_descriptor_universe();
    assert(universe.valid());
    assert(universe.evidence_packet_id == "dmc3-stage-wave2");
    assert(universe.observed_descriptor_count() == 189U);
    assert(universe.banks[0].va == 0x1405C4AA0ULL);
    assert(universe.banks[0].row_count == 110U);
    assert(universe.banks[0].numeric_stage_ids.front() == 0U);
    assert(universe.banks[0].numeric_stage_ids.back() == 307U);
    assert(universe.banks[1].va == 0x1405C3080ULL);
    assert(universe.banks[1].row_count == 79U);
    assert(universe.banks[1].numeric_stage_ids.front() == 308U);
    assert(universe.banks[1].numeric_stage_ids.back() == 910U);
    assert(universe.selector_table_va == 0x1405C4440ULL);
    assert(universe.selector_entry_count == 193U);
    assert(universe.group_base_table_va == 0x1405C4A50ULL);
    assert(universe.group_base_count == 10U);

    auto wrong_size = descriptor;
    wrong_size.artifact_size =
        descriptor.file_offset + descriptor.table_size_bytes() - 1U;
    assert(!wrong_size.valid());

    auto invalid = descriptor;
    invalid.cell_stride = 0U;
    assert(!invalid.valid());

    auto invalid_path_offset = descriptor;
    invalid_path_offset.path_pointer_offset = 9U;
    assert(!invalid_path_offset.valid());

    return 0;
}

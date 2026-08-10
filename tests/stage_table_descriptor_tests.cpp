#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <cassert>

int main() {
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;

    const auto& descriptor = phase12_stage_resource_table();
    assert(descriptor.valid());
    assert(descriptor.artifact_size == 6356432ULL);
    assert(descriptor.file_offset == 0x005C30A8ULL);
    assert(descriptor.rva == 0x005C4AA8U);
    assert(descriptor.va == 0x1405C4AA8ULL);
    assert(descriptor.row_count == 110U);
    assert(descriptor.entry_count() == 440U);
    assert(descriptor.cell_stride == 0x10U);
    assert(descriptor.path_pointer_offset == 0U);
    assert(descriptor.table_size_bytes() == 0x1B80ULL);
    assert(descriptor.role_for_column(0U) == StageResourceRole::script);
    assert(descriptor.role_for_column(1U) == StageResourceRole::room_config);
    assert(descriptor.role_for_column(2U) == StageResourceRole::room_effects);
    assert(descriptor.role_for_column(3U) == StageResourceRole::room_sound);
    assert(!descriptor.role_for_column(4U).has_value());

    return 0;
}

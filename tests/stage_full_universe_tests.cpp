#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::profiles::dmc3::StageResourceTableReadResult make_table(
    const dmc::rengine::profiles::dmc3::StageResourceTableDescriptor& descriptor,
    std::string prefix) {
    using namespace dmc::rengine::profiles::dmc3;
    StageResourceTableReadResult result;
    result.rows.reserve(descriptor.row_count);
    for (std::uint32_t row_index = 0U; row_index < descriptor.row_count; ++row_index) {
        StageResourceTableRowObservation row{.row_index = row_index, .cells = {}};
        for (std::uint32_t column = 0U; column < 4U; ++column) {
            row.cells[column] = StageResourceTableCellObservation{
                .row_index = row_index,
                .column_index = column,
                .role = *descriptor.role_for_column(column),
                .cell_file_offset = descriptor.file_offset +
                    (static_cast<std::uint64_t>(row_index) * 4ULL + column) * 0x10ULL,
                .cell_rva = descriptor.rva + (row_index * 4U + column) * 0x10U,
                .cell_va = descriptor.va +
                    (static_cast<std::uint64_t>(row_index) * 4ULL + column) * 0x10ULL,
                .kind16 = static_cast<std::uint16_t>(column),
                .path_pointer_va = 0x140700000ULL +
                    static_cast<std::uint64_t>(row_index) * 0x100ULL + column * 0x20ULL,
                .path_file_offset = 0x600000ULL +
                    static_cast<std::uint64_t>(row_index) * 0x100ULL + column * 0x20ULL,
                .logical_path = prefix + "/row_" + std::to_string(row_index) +
                    "_col_" + std::to_string(column) + ".pac",
            };
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}

} // namespace

int main() {
    using namespace dmc::rengine::profiles::dmc3;

    const auto& banks = wave2_stage_resource_banks();
    assert(banks[0].valid());
    assert(banks[1].valid());
    assert(banks[0].is_promoted_wave2_authority());
    assert(banks[1].is_promoted_wave2_authority());
    assert(banks[0].row_count == 110U);
    assert(banks[1].row_count == 79U);
    assert(banks[0].row_size_bytes() == 0x40ULL);
    assert(banks[1].row_size_bytes() == 0x40ULL);
    assert(banks[1].file_offset == 0x005C1680ULL);
    assert(banks[1].va == 0x1405C3080ULL);

    const auto& universe = wave2_stage_descriptor_universe();
    assert(universe.valid());
    assert(universe.observed_descriptor_count() == 189U);
    assert(universe.selector_entry_count == 193U);
    assert(universe.group_base_count == 10U);

    assert(runtime_stage_id_for_table_row(banks[0], 0U) == 0U);
    assert(runtime_stage_id_for_table_row(banks[0], 109U) == 307U);
    assert(runtime_stage_id_for_table_row(banks[1], 0U) == 308U);
    assert(runtime_stage_id_for_table_row(banks[1], 55U) == 600U);
    assert(runtime_stage_id_for_table_row(banks[1], 78U) == 910U);

    std::array<StageResourceTableReadResult, 2> tables{
        make_table(banks[0], "a"),
        make_table(banks[1], "b"),
    };
    const auto catalog = StageCatalogBuilder::build_full_universe(tables, banks);
    assert(catalog.complete_full_universe());
    assert(catalog.entries.size() == 189U);
    assert(catalog.covers_full_stage_universe());

    const auto* st307 = catalog.find_numeric_stage(307U);
    assert(st307 != nullptr);
    assert(st307->row_index == 109U);
    assert(st307->source_table_id == banks[0].id);
    assert(st307->source_row_index == 109U);

    const auto* st308 = catalog.find_numeric_stage(308U);
    assert(st308 != nullptr);
    assert(st308->row_index == 110U);
    assert(st308->source_table_id == banks[1].id);
    assert(st308->source_row_index == 0U);

    const auto* st600 = catalog.find_numeric_stage(600U);
    assert(st600 != nullptr);
    assert(st600->row_index == 165U);
    assert(st600->source_row_index == 55U);

    const auto* st910 = catalog.find_numeric_stage(910U);
    assert(st910 != nullptr);
    assert(st910->row_index == 188U);

    // Structural validity and canonical authority are distinct. These mutated
    // descriptors remain bounded/readable schemas but cannot drive promoted
    // Stage identity or catalog construction.
    auto shifted = banks[1];
    shifted.va += 0x40ULL;
    shifted.rva += 0x40U;
    shifted.file_offset += 0x40ULL;
    assert(shifted.valid());
    assert(!shifted.is_promoted_wave2_authority());
    assert(!runtime_stage_id_for_table_row(shifted, 0U).has_value());

    auto wrong_count = banks[1];
    wrong_count.row_count = 78U;
    assert(wrong_count.valid());
    assert(!wrong_count.is_promoted_wave2_authority());
    assert(!runtime_stage_id_for_table_row(wrong_count, 0U).has_value());

    const std::array<StageResourceTableDescriptor, 2> shifted_banks{
        banks[0], shifted,
    };
    const auto rejected = StageCatalogBuilder::build_full_universe(
        tables, shifted_banks);
    assert(!rejected.complete_full_universe());

    return 0;
}

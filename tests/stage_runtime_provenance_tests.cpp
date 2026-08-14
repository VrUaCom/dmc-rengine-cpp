#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    using namespace dmc::rengine::profiles::dmc3;

    StageResourceTableRowObservation observation{.row_index = 55U, .cells = {}};
    const std::array<std::string, 4> paths{
        "scr/st600.pac",
        "room/st600cfg.pac",
        "room/st004_effect.pac",
        "se/snd_r004.pac",
    };
    const std::array<StageResourceRole, 4> roles{
        StageResourceRole::script,
        StageResourceRole::room_config,
        StageResourceRole::room_effects,
        StageResourceRole::room_sound,
    };
    for (std::uint32_t column = 0U; column < 4U; ++column) {
        observation.cells[column] = StageResourceTableCellObservation{
            .row_index = observation.row_index,
            .column_index = column,
            .role = roles[column],
            .cell_file_offset = 0x1000U + column * 0x10U,
            .cell_rva = 0x2000U + column * 0x10U,
            .cell_va = 0x140002000ULL + column * 0x10U,
            .kind16 = static_cast<std::uint16_t>(1U),
            .path_pointer_va = 0x140003000ULL + column * 0x20U,
            .path_file_offset = 0x3000U + column * 0x20U,
            .logical_path = paths[column],
        };
    }
    assert(observation.complete());

    const StageCatalogEntry entry{
        .catalog_entry_id = "dmc3-stage-descriptor-bank-b/row/55",
        .row_index = 165U,
        .numeric_stage_id = 600U,
        .evidence_id = "dmc3-stage-wave2",
        .observation = observation,
        .semantic_stage_id = std::string{"semantic-stage-fixture"},
        .source_table_id = "dmc3-stage-descriptor-bank-b",
        .source_row_index = 55U,
    };
    assert(entry.complete());

    constexpr std::array<std::uint32_t, 0> no_archives{};
    const auto bootstrap = VolumeBootstrapPolicy::plan(no_archives);
    const RuntimeSourceBindings bindings{
        .physical_source_id = "unmounted-physical",
        .archives = {},
    };
    dmc::rengine::gdspaces::SourceRegistry sources;

    const auto report = StageRuntimeResolver::resolve_entry(
        entry, bootstrap, bindings, sources);
    assert(!report.complete());
    assert(report.catalog_entry_id == entry.catalog_entry_id);
    assert(report.table_row_index == 165U);
    assert(report.source_table_id == entry.source_table_id);
    assert(report.source_row_index == 55U);
    assert(report.numeric_stage_id == 600U);
    assert(report.semantic_stage_id == entry.semantic_stage_id);
    assert(report.plan.numeric_stage_id == 600U);
    assert(report.plan.semantic_stage_id == "semantic-stage-fixture");
    assert(report.plan.resources[2].logical_path == "room/st004_effect.pac");
    assert(report.plan.resources[3].logical_path == "se/snd_r004.pac");
    return 0;
}

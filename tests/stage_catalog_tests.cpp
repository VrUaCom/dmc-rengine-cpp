#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>

namespace {

[[nodiscard]] dmc::rengine::profiles::dmc3::StageResourceTableReadResult
make_table_observations() {
    using dmc::rengine::profiles::dmc3::StageResourceTableCellObservation;
    using dmc::rengine::profiles::dmc3::StageResourceTableReadResult;
    using dmc::rengine::profiles::dmc3::StageResourceTableRowObservation;
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;

    const auto& descriptor = phase12_stage_resource_table();
    StageResourceTableReadResult result;
    result.rows.reserve(descriptor.row_count);

    for (std::uint32_t row_index = 0U; row_index < descriptor.row_count; ++row_index) {
        StageResourceTableRowObservation row{.row_index = row_index, .cells = {}};
        for (std::uint32_t column_index = 0U; column_index < 4U; ++column_index) {
            auto logical_path = std::string{"resource/row_"} +
                std::to_string(row_index) + "_column_" +
                std::to_string(column_index) + ".pac";

            // Exact repeated EXE reference across two rows. This should be
            // recorded structurally without inventing semantic stage sharing.
            if ((row_index == 3U || row_index == 77U) && column_index == 2U) {
                logical_path = "room/shared_effects.pac";
            }

            // Case-different text is intentionally not grouped by StageCatalog.
            // Runtime provider normalization belongs to GDSpaces resolution.
            if (row_index == 8U && column_index == 0U) {
                logical_path = "SCR/CASE_SENSITIVE_OBSERVATION.PAC";
            }
            if (row_index == 9U && column_index == 0U) {
                logical_path = "scr/case_sensitive_observation.pac";
            }

            row.cells[column_index] = StageResourceTableCellObservation{
                .row_index = row_index,
                .column_index = column_index,
                .role = *descriptor.role_for_column(column_index),
                .cell_file_offset = descriptor.file_offset +
                    (static_cast<std::uint64_t>(row_index) * 4U + column_index) *
                        descriptor.cell_stride,
                .cell_rva = descriptor.rva +
                    (row_index * 4U + column_index) * descriptor.cell_stride,
                .cell_va = descriptor.va +
                    (static_cast<std::uint64_t>(row_index) * 4U + column_index) *
                        descriptor.cell_stride,
                .path_pointer_va = 0x140700000ULL +
                    static_cast<std::uint64_t>(row_index) * 0x100U +
                    static_cast<std::uint64_t>(column_index) * 0x20U,
                .path_file_offset = 0x600000U +
                    static_cast<std::uint64_t>(row_index) * 0x100U +
                    static_cast<std::uint64_t>(column_index) * 0x20U,
                .logical_path = std::move(logical_path),
            };
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}

} // namespace

int main() {
    using dmc::rengine::profiles::dmc3::StageCatalogBuilder;
    using dmc::rengine::profiles::dmc3::StageCatalogLoader;
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;

    const auto& descriptor = phase12_stage_resource_table();
    const auto observations = make_table_observations();
    assert(observations.complete(descriptor));

    const auto catalog = StageCatalogBuilder::build(observations, descriptor);
    assert(catalog.complete(descriptor));
    assert(catalog.entries.size() == 110U);
    assert(catalog.table_id == descriptor.id);
    assert(catalog.evidence_id == descriptor.evidence_packet_id);

    const auto* first = catalog.find(0U);
    assert(first != nullptr);
    assert(first->catalog_entry_id == descriptor.id + "/row/0");
    assert(first->row_index == 0U);
    assert(!first->semantic_stage_id.has_value());
    assert(first->complete());

    const auto first_plan = first->resource_plan();
    assert(first_plan.valid());
    // Existing StageResourceRowPlan calls this field stage_id. For catalog-driven
    // resolution it carries row identity only, never an inferred gameplay id.
    assert(first_plan.stage_id == first->catalog_entry_id);
    assert(first_plan.resources[0].logical_path ==
        "resource/row_0_column_0.pac");

    assert(catalog.find(109U) != nullptr);
    assert(catalog.find(110U) == nullptr);

    assert(catalog.repeated_references.size() == 1U);
    assert(catalog.repeated_references[0].logical_path ==
        "room/shared_effects.pac");
    assert(catalog.repeated_references[0].uses.size() == 2U);
    assert(catalog.repeated_references[0].uses[0].row_index == 3U);
    assert(catalog.repeated_references[0].uses[1].row_index == 77U);

    // The catalog does not normalize case or infer alias/variant relationships.
    for (const auto& repeated : catalog.repeated_references) {
        assert(repeated.logical_path != "SCR/CASE_SENSITIVE_OBSERVATION.PAC");
        assert(repeated.logical_path != "scr/case_sensitive_observation.pac");
    }

    auto incomplete = observations;
    incomplete.rows[42U].cells[1].logical_path.clear();
    const auto partial_catalog = StageCatalogBuilder::build(incomplete, descriptor);
    assert(!partial_catalog.complete(descriptor));
    const auto* incomplete_entry = partial_catalog.find(42U);
    assert(incomplete_entry != nullptr);
    assert(!incomplete_entry->complete());

    // Production catalog loading is hash-gated before the structural reader is
    // trusted. Public synthetic bytes must therefore fail the canonical gate.
    constexpr std::array<std::byte, 4> noncanonical_bytes{
        std::byte{0x44}, std::byte{0x4D}, std::byte{0x43}, std::byte{0x33},
    };
    const auto rejected = StageCatalogLoader::load_canonical(
        std::span<const std::byte>{noncanonical_bytes},
        dmc::rengine::exe::PeImage{});
    assert(!rejected.canonical_artifact);
    assert(!rejected.complete());
    assert(rejected.catalog.entries.empty());
    assert(!rejected.catalog.diagnostics.empty());

    return 0;
}

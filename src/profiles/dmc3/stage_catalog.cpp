#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string make_catalog_entry_id(
    const StageResourceTableDescriptor& descriptor,
    std::uint32_t row_index) {
    return descriptor.id + "/row/" + std::to_string(row_index);
}

void add_diagnostic(
    StageCatalog& catalog,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    catalog.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

} // namespace

bool StageCatalogEntry::complete() const noexcept {
    return !catalog_entry_id.empty() && !evidence_id.empty() &&
        observation.row_index == row_index && observation.complete();
}

StageResourceRowPlan StageCatalogEntry::resource_plan() const {
    auto plan = make_stage_resource_plan_from_table_row(
        catalog_entry_id,
        observation.logical_paths(),
        evidence_id);
    if (semantic_stage_id.has_value()) {
        plan.semantic_stage_id = *semantic_stage_id;
    }
    return plan;
}

bool StageCatalog::complete(
    const StageResourceTableDescriptor& descriptor) const noexcept {
    if (!descriptor.valid() || table_id != descriptor.id ||
        evidence_id != descriptor.evidence_packet_id ||
        entries.size() != descriptor.row_count) {
        return false;
    }

    if (std::any_of(
            diagnostics.begin(), diagnostics.end(),
            [](const gdspaces::Diagnostic& diagnostic) {
                return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
            })) {
        return false;
    }

    for (std::uint32_t row_index = 0U; row_index < descriptor.row_count; ++row_index) {
        if (entries[row_index].row_index != row_index || !entries[row_index].complete()) {
            return false;
        }
    }
    return true;
}

const StageCatalogEntry* StageCatalog::find(
    std::uint32_t row_index) const noexcept {
    const auto iterator = std::find_if(
        entries.begin(), entries.end(),
        [row_index](const StageCatalogEntry& entry) {
            return entry.row_index == row_index;
        });
    return iterator == entries.end() ? nullptr : &*iterator;
}

StageCatalog StageCatalogBuilder::build(
    const StageResourceTableReadResult& table,
    const StageResourceTableDescriptor& descriptor) {
    StageCatalog catalog{
        .table_id = descriptor.id,
        .evidence_id = descriptor.evidence_packet_id,
        .entries = {},
        .repeated_references = {},
        .diagnostics = table.diagnostics,
    };

    if (!descriptor.valid()) {
        add_diagnostic(
            catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.invalid-descriptor",
            "The stage catalog cannot be built from an invalid table descriptor.");
        return catalog;
    }

    if (table.rows.size() != descriptor.row_count) {
        add_diagnostic(
            catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.row-count-mismatch",
            "The observed stage-table row count does not match the canonical descriptor.");
    }

    catalog.entries.reserve(table.rows.size());
    for (std::size_t index = 0U; index < table.rows.size(); ++index) {
        const auto& row = table.rows[index];
        if (row.row_index != index) {
            add_diagnostic(
                catalog,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-catalog.row-order-mismatch",
                "Stage-table observations are not in deterministic executable row order.");
        }

        catalog.entries.push_back(StageCatalogEntry{
            .catalog_entry_id = make_catalog_entry_id(descriptor, row.row_index),
            .row_index = row.row_index,
            .evidence_id = descriptor.evidence_packet_id,
            .observation = row,
            .semantic_stage_id = std::nullopt,
        });

        for (const auto& cell : row.cells) {
            if (!cell.valid()) {
                continue;
            }

            auto group = std::find_if(
                catalog.repeated_references.begin(),
                catalog.repeated_references.end(),
                [&cell](const StageCatalogRepeatedReference& candidate) {
                    return candidate.logical_path == cell.logical_path;
                });
            if (group == catalog.repeated_references.end()) {
                catalog.repeated_references.push_back(StageCatalogRepeatedReference{
                    .logical_path = cell.logical_path,
                    .uses = {},
                });
                group = std::prev(catalog.repeated_references.end());
            }
            group->uses.push_back(StageCatalogReferenceUse{
                .row_index = row.row_index,
                .column_index = cell.column_index,
                .role = cell.role,
            });
        }
    }

    std::erase_if(
        catalog.repeated_references,
        [](const StageCatalogRepeatedReference& reference) {
            return !reference.repeated();
        });

    return catalog;
}

bool StageCatalogLoadResult::complete() const noexcept {
    return canonical_artifact &&
        catalog.complete(phase12_stage_resource_table());
}

StageCatalogLoadResult StageCatalogLoader::load_canonical(
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    std::size_t max_path_bytes) {
    const auto& descriptor = phase12_stage_resource_table();
    StageCatalogLoadResult result{
        .artifact_sha256 = core::Sha256::compute(executable_bytes).hex(),
        .canonical_artifact = false,
        .catalog = StageCatalog{
            .table_id = descriptor.id,
            .evidence_id = descriptor.evidence_packet_id,
            .entries = {},
            .repeated_references = {},
            .diagnostics = {},
        },
    };

    if (result.artifact_sha256 != descriptor.artifact_sha256) {
        add_diagnostic(
            result.catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.artifact-hash-mismatch",
            "The supplied executable SHA-256 does not match the canonical DMC3 stage-table artifact.");
        return result;
    }

    result.canonical_artifact = true;
    const auto table = StageResourceTableReader::read(
        executable_bytes,
        image,
        descriptor,
        max_path_bytes);
    result.catalog = StageCatalogBuilder::build(table, descriptor);
    return result;
}

} // namespace dmc::rengine::profiles::dmc3

#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"

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

[[nodiscard]] StageCatalog empty_bank_a_catalog(
    const StageResourceTableDescriptor& descriptor) {
    return StageCatalog{
        .coverage = StageCatalogCoverage::wave2_bank_a_compatibility,
        .table_id = descriptor.id,
        .evidence_id = descriptor.evidence_packet_id,
        .entries = {},
        .repeated_references = {},
        .diagnostics = {},
    };
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
    for (std::size_t index = 0U; index < plan.resources.size(); ++index) {
        plan.resources[index].kind16 = observation.cells[index].kind16;
    }
    plan.numeric_stage_id = numeric_stage_id;
    if (semantic_stage_id.has_value()) {
        plan.semantic_stage_id = *semantic_stage_id;
    }
    return plan;
}

bool StageCatalog::complete(
    const StageResourceTableDescriptor& descriptor) const noexcept {
    if (coverage != StageCatalogCoverage::wave2_bank_a_compatibility ||
        !descriptor.valid() || table_id != descriptor.id ||
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
    auto catalog = empty_bank_a_catalog(descriptor);
    catalog.diagnostics = table.diagnostics;

    if (!descriptor.valid()) {
        add_diagnostic(
            catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.invalid-descriptor",
            "The Stage catalog cannot be built from an invalid Bank-A descriptor.");
        return catalog;
    }

    const auto& universe = wave2_stage_descriptor_universe();
    if (!universe.valid() || universe.banks[0].row_count != descriptor.row_count) {
        add_diagnostic(
            catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.bank-a-metadata-invalid",
            "Wave-2 Bank-A numeric identity metadata does not match the compatibility descriptor.");
        return catalog;
    }

    if (table.rows.size() != descriptor.row_count) {
        add_diagnostic(
            catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.row-count-mismatch",
            "The observed Bank-A descriptor row count does not match the corrected Wave-2 descriptor.");
    }

    catalog.entries.reserve(table.rows.size());
    for (std::size_t index = 0U; index < table.rows.size(); ++index) {
        const auto& row = table.rows[index];
        if (row.row_index != index) {
            add_diagnostic(
                catalog,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-catalog.row-order-mismatch",
                "Bank-A observations are not in deterministic executable row order.");
        }

        std::optional<std::uint16_t> numeric_stage_id;
        if (row.row_index < universe.banks[0].numeric_stage_ids.size()) {
            numeric_stage_id = universe.banks[0].numeric_stage_ids[row.row_index];
        } else {
            add_diagnostic(
                catalog,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-catalog.numeric-stage-id-missing",
                "A Bank-A descriptor row has no corresponding Wave-2 numeric identity.");
        }

        catalog.entries.push_back(StageCatalogEntry{
            .catalog_entry_id = make_catalog_entry_id(descriptor, row.row_index),
            .row_index = row.row_index,
            .numeric_stage_id = numeric_stage_id,
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
        catalog.complete(wave2_stage_resource_bank_a());
}

StageCatalogLoadResult StageCatalogLoader::load_canonical(
    std::span<const std::byte> executable_bytes,
    std::size_t max_path_bytes) {
    const auto& descriptor = wave2_stage_resource_bank_a();
    StageCatalogLoadResult result{
        .artifact_sha256 = core::Sha256::compute(executable_bytes).hex(),
        .canonical_artifact = false,
        .catalog = empty_bank_a_catalog(descriptor),
    };

    if (result.artifact_sha256 != descriptor.artifact_sha256) {
        add_diagnostic(
            result.catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.artifact-hash-mismatch",
            "The supplied executable SHA-256 does not match the canonical DMC3 Wave-2 Stage artifact.");
        return result;
    }

    result.canonical_artifact = true;
    const auto pe = exe::PeReader::read(executable_bytes);
    for (const auto& warning : pe.warnings) {
        add_diagnostic(
            result.catalog,
            gdspaces::DiagnosticSeverity::warning,
            "dmc3.stage-catalog.pe-warning",
            warning);
    }
    for (const auto& error : pe.errors) {
        add_diagnostic(
            result.catalog,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.pe-error",
            error);
    }
    if (!pe.ok()) {
        return result;
    }

    const auto table = StageResourceTableReader::read(
        executable_bytes,
        *pe.image,
        descriptor,
        max_path_bytes);
    result.catalog = StageCatalogBuilder::build(table, descriptor);
    return result;
}

} // namespace dmc::rengine::profiles::dmc3

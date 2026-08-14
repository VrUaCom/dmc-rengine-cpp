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
    std::uint32_t source_row_index) {
    return descriptor.id + "/row/" + std::to_string(source_row_index);
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

[[nodiscard]] bool has_error(const StageCatalog& catalog) noexcept {
    return std::any_of(
        catalog.diagnostics.begin(), catalog.diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

void add_reference(
    StageCatalog& catalog,
    const StageResourceTableCellObservation& cell,
    std::uint32_t global_row_index,
    const StageResourceTableDescriptor& descriptor) {
    if (!cell.valid()) {
        return;
    }
    auto group = std::find_if(
        catalog.repeated_references.begin(), catalog.repeated_references.end(),
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
        .row_index = global_row_index,
        .column_index = cell.column_index,
        .role = cell.role,
        .source_table_id = descriptor.id,
        .source_row_index = cell.row_index,
    });
}

void finalize_repeated_references(StageCatalog& catalog) {
    std::erase_if(
        catalog.repeated_references,
        [](const StageCatalogRepeatedReference& reference) {
            return !reference.repeated();
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

[[nodiscard]] StageCatalog empty_full_catalog() {
    return StageCatalog{
        .coverage = StageCatalogCoverage::full_selector_universe,
        .table_id = "dmc3-stage-descriptor-universe",
        .evidence_id = wave2_stage_descriptor_universe().evidence_packet_id,
        .entries = {},
        .repeated_references = {},
        .diagnostics = {},
    };
}

} // namespace

bool StageCatalogEntry::complete() const noexcept {
    if (catalog_entry_id.empty() || evidence_id.empty() || !observation.complete()) {
        return false;
    }
    if (source_table_id.empty()) {
        return observation.row_index == row_index;
    }
    return observation.row_index == source_row_index;
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
        entries.size() != descriptor.row_count || has_error(*this)) {
        return false;
    }

    for (std::uint32_t row_index = 0U; row_index < descriptor.row_count; ++row_index) {
        const auto& entry = entries[row_index];
        if (entry.row_index != row_index || !entry.complete() ||
            entry.source_table_id != descriptor.id ||
            entry.source_row_index != row_index ||
            entry.numeric_stage_id != runtime_stage_id_for_table_row(descriptor, row_index)) {
            return false;
        }
    }
    return true;
}

bool StageCatalog::complete_full_universe() const noexcept {
    const auto& metadata = wave2_stage_descriptor_universe();
    const auto& descriptors = wave2_stage_resource_banks();
    if (coverage != StageCatalogCoverage::full_selector_universe ||
        !metadata.valid() || table_id != "dmc3-stage-descriptor-universe" ||
        evidence_id != metadata.evidence_packet_id ||
        entries.size() != metadata.observed_descriptor_count() || has_error(*this)) {
        return false;
    }

    std::uint32_t global_row{};
    for (const auto& descriptor : descriptors) {
        if (!descriptor.valid()) {
            return false;
        }
        for (std::uint32_t source_row = 0U; source_row < descriptor.row_count;
             ++source_row, ++global_row) {
            const auto& entry = entries[global_row];
            if (!entry.complete() || entry.row_index != global_row ||
                entry.source_table_id != descriptor.id ||
                entry.source_row_index != source_row ||
                entry.numeric_stage_id !=
                    runtime_stage_id_for_table_row(descriptor, source_row)) {
                return false;
            }
        }
    }
    return global_row == metadata.observed_descriptor_count();
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

const StageCatalogEntry* StageCatalog::find_numeric_stage(
    std::uint16_t numeric_stage_id_value) const noexcept {
    const auto iterator = std::find_if(
        entries.begin(), entries.end(),
        [numeric_stage_id_value](const StageCatalogEntry& entry) {
            return entry.numeric_stage_id.has_value() &&
                *entry.numeric_stage_id == numeric_stage_id_value;
        });
    return iterator == entries.end() ? nullptr : &*iterator;
}

StageCatalog StageCatalogBuilder::build(
    const StageResourceTableReadResult& table,
    const StageResourceTableDescriptor& descriptor) {
    auto catalog = empty_bank_a_catalog(descriptor);
    catalog.diagnostics = table.diagnostics;

    if (!descriptor.valid() || descriptor.va != wave2_stage_resource_bank_a().va) {
        add_diagnostic(catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.invalid-descriptor",
            "The Bank-A compatibility catalog requires the exact promoted Wave-2 Bank-A descriptor.");
        return catalog;
    }

    if (table.rows.size() != descriptor.row_count) {
        add_diagnostic(catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.row-count-mismatch",
            "The observed Bank-A descriptor row count does not match the promoted Wave-2 descriptor.");
    }

    catalog.entries.reserve(table.rows.size());
    for (std::size_t index = 0U; index < table.rows.size(); ++index) {
        const auto& row = table.rows[index];
        if (row.row_index != index) {
            add_diagnostic(catalog, gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-catalog.row-order-mismatch",
                "Bank-A observations are not in deterministic executable row order.");
        }
        catalog.entries.push_back(StageCatalogEntry{
            .catalog_entry_id = make_catalog_entry_id(descriptor, row.row_index),
            .row_index = static_cast<std::uint32_t>(index),
            .numeric_stage_id = runtime_stage_id_for_table_row(descriptor, row.row_index),
            .evidence_id = descriptor.evidence_packet_id,
            .observation = row,
            .semantic_stage_id = std::nullopt,
            .source_table_id = descriptor.id,
            .source_row_index = row.row_index,
        });
        for (const auto& cell : row.cells) {
            add_reference(catalog, cell, static_cast<std::uint32_t>(index), descriptor);
        }
    }
    finalize_repeated_references(catalog);
    return catalog;
}

StageCatalog StageCatalogBuilder::build_full_universe(
    const std::array<StageResourceTableReadResult, 2>& tables,
    const std::array<StageResourceTableDescriptor, 2>& descriptors) {
    auto catalog = empty_full_catalog();
    const auto& authority = wave2_stage_resource_banks();
    std::uint32_t global_row{};
    catalog.entries.reserve(wave2_stage_descriptor_universe().observed_descriptor_count());

    for (std::size_t bank_index = 0U; bank_index < descriptors.size(); ++bank_index) {
        const auto& descriptor = descriptors[bank_index];
        const auto& table = tables[bank_index];
        catalog.diagnostics.insert(
            catalog.diagnostics.end(), table.diagnostics.begin(), table.diagnostics.end());

        if (!descriptor.valid() || descriptor.id != authority[bank_index].id ||
            descriptor.va != authority[bank_index].va ||
            table.rows.size() != descriptor.row_count) {
            add_diagnostic(catalog, gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-catalog.full-bank-invalid",
                "A promoted Wave-2 Stage descriptor bank is invalid or incomplete.");
        }

        for (std::size_t source_index = 0U; source_index < table.rows.size(); ++source_index) {
            const auto& row = table.rows[source_index];
            if (row.row_index != source_index) {
                add_diagnostic(catalog, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-catalog.full-row-order-mismatch",
                    "A Stage descriptor bank is not in deterministic executable row order.");
            }
            catalog.entries.push_back(StageCatalogEntry{
                .catalog_entry_id = make_catalog_entry_id(descriptor, row.row_index),
                .row_index = global_row,
                .numeric_stage_id = runtime_stage_id_for_table_row(descriptor, row.row_index),
                .evidence_id = descriptor.evidence_packet_id,
                .observation = row,
                .semantic_stage_id = std::nullopt,
                .source_table_id = descriptor.id,
                .source_row_index = row.row_index,
            });
            for (const auto& cell : row.cells) {
                add_reference(catalog, cell, global_row, descriptor);
            }
            ++global_row;
        }
    }
    finalize_repeated_references(catalog);
    return catalog;
}

bool StageCatalogLoadResult::complete() const noexcept {
    if (!canonical_artifact) {
        return false;
    }
    if (catalog.coverage == StageCatalogCoverage::full_selector_universe) {
        return catalog.complete_full_universe();
    }
    return catalog.complete(wave2_stage_resource_bank_a());
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
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.artifact-hash-mismatch",
            "The supplied executable SHA-256 does not match the canonical DMC3 Wave-2 Stage artifact.");
        return result;
    }

    result.canonical_artifact = true;
    const auto pe = exe::PeReader::read(executable_bytes);
    for (const auto& warning : pe.warnings) {
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::warning,
            "dmc3.stage-catalog.pe-warning", warning);
    }
    for (const auto& error : pe.errors) {
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.pe-error", error);
    }
    if (!pe.ok()) {
        return result;
    }

    const auto table = StageResourceTableReader::read(
        executable_bytes, *pe.image, descriptor, max_path_bytes);
    result.catalog = StageCatalogBuilder::build(table, descriptor);
    return result;
}

StageCatalogLoadResult StageCatalogLoader::load_canonical_full_universe(
    std::span<const std::byte> executable_bytes,
    std::size_t max_path_bytes) {
    const auto& descriptors = wave2_stage_resource_banks();
    StageCatalogLoadResult result{
        .artifact_sha256 = core::Sha256::compute(executable_bytes).hex(),
        .canonical_artifact = false,
        .catalog = empty_full_catalog(),
    };

    if (result.artifact_sha256 != descriptors[0].artifact_sha256) {
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.artifact-hash-mismatch",
            "The supplied executable SHA-256 does not match the canonical DMC3 Wave-2 Stage artifact.");
        return result;
    }

    result.canonical_artifact = true;
    const auto pe = exe::PeReader::read(executable_bytes);
    for (const auto& warning : pe.warnings) {
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::warning,
            "dmc3.stage-catalog.pe-warning", warning);
    }
    for (const auto& error : pe.errors) {
        add_diagnostic(result.catalog, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-catalog.pe-error", error);
    }
    if (!pe.ok()) {
        return result;
    }

    std::array<StageResourceTableReadResult, 2> tables{};
    for (std::size_t index = 0U; index < descriptors.size(); ++index) {
        tables[index] = StageResourceTableReader::read(
            executable_bytes, *pe.image, descriptors[index], max_path_bytes);
    }
    result.catalog = StageCatalogBuilder::build_full_universe(tables, descriptors);
    return result;
}

} // namespace dmc::rengine::profiles::dmc3

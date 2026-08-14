#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] gdspaces::StageResourceCategory category_for_role(
    StageResourceRole role) noexcept {
    switch (role) {
    case StageResourceRole::script:
        return gdspaces::StageResourceCategory::scripts;
    case StageResourceRole::room_config:
        return gdspaces::StageResourceCategory::unknown;
    case StageResourceRole::room_effects:
        return gdspaces::StageResourceCategory::effects;
    case StageResourceRole::room_sound:
        return gdspaces::StageResourceCategory::sounds;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] gdspaces::DiagnosticSeverity severity_for(
    RuntimeResolutionStatus status) noexcept {
    switch (status) {
    case RuntimeResolutionStatus::resolved:
        return gdspaces::DiagnosticSeverity::info;
    case RuntimeResolutionStatus::not_found:
        return gdspaces::DiagnosticSeverity::warning;
    case RuntimeResolutionStatus::ambiguous:
    case RuntimeResolutionStatus::invalid_request:
    case RuntimeResolutionStatus::invalid_source_configuration:
        return gdspaces::DiagnosticSeverity::error;
    }
    return gdspaces::DiagnosticSeverity::error;
}

} // namespace

bool StageRuntimeResolutionReport::complete() const noexcept {
    if (catalog_entry_id.empty() || !plan.valid() ||
        std::any_of(
            diagnostics.begin(), diagnostics.end(),
            [](const gdspaces::Diagnostic& diagnostic) {
                return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
            })) {
        return false;
    }

    return std::all_of(
        resources.begin(), resources.end(),
        [](const StageRuntimeResourceResolution& resource) {
            return resource.resolved();
        });
}

std::vector<gdspaces::StageMemberCandidate>
StageRuntimeResolutionReport::resolved_candidates() const {
    std::vector<gdspaces::StageMemberCandidate> candidates;
    if (!complete()) {
        return candidates;
    }

    candidates.reserve(resources.size());
    for (const auto& resource : resources) {
        candidates.push_back(gdspaces::StageMemberCandidate{
            .resource = *resource.runtime.resolved,
            .category = category_for_role(resource.reference.role),
            .role = std::string{to_string(resource.reference.role)},
        });
    }
    return candidates;
}

StageRuntimeResolutionReport StageRuntimeResolver::resolve_entry(
    const StageCatalogEntry& entry,
    const VolumeBootstrapPlan& bootstrap,
    const RuntimeSourceBindings& bindings,
    const gdspaces::SourceRegistry& sources) {
    if (!entry.complete()) {
        StageRuntimeResolutionReport report{
            .catalog_entry_id = entry.catalog_entry_id,
            .table_row_index = entry.row_index,
            .plan = {},
            .resources = {},
            .diagnostics = {},
        };
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.invalid-catalog-entry",
            .message = "The StageCatalog entry is incomplete and cannot be resolved.",
            .resource = std::nullopt,
        });
        return report;
    }

    return resolve_row(
        entry.catalog_entry_id,
        entry.evidence_id,
        entry.observation,
        bootstrap,
        bindings,
        sources);
}

StageRuntimeResolutionReport StageRuntimeResolver::resolve_row(
    std::string catalog_entry_id,
    std::string evidence_id,
    const StageResourceTableRowObservation& row,
    const VolumeBootstrapPlan& bootstrap,
    const RuntimeSourceBindings& bindings,
    const gdspaces::SourceRegistry& sources) {
    StageRuntimeResolutionReport report{
        .catalog_entry_id = catalog_entry_id,
        .table_row_index = row.row_index,
        .plan = {},
        .resources = {},
        .diagnostics = {},
    };

    if (catalog_entry_id.empty()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.missing-catalog-entry-id",
            .message = "A stable executable table-row identity is required before stage resources can be resolved.",
            .resource = std::nullopt,
        });
        return report;
    }

    if (!row.complete()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.incomplete-exe-row",
            .message = "The executable stage-table row is incomplete and cannot be resolved.",
            .resource = std::nullopt,
        });
        return report;
    }

    report.plan = make_stage_resource_plan_from_table_row(
        catalog_entry_id, row.logical_paths(), std::move(evidence_id));
    if (!report.plan.valid()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.invalid-row-plan",
            .message = "The executable stage-table row did not produce a valid four-role resource plan.",
            .resource = std::nullopt,
        });
        return report;
    }

    for (std::size_t index = 0U; index < report.resources.size(); ++index) {
        const auto& reference = report.plan.resources[index];
        auto runtime = RuntimeResourceResolver::resolve(
            reference.logical_path,
            bootstrap,
            bindings,
            sources);

        report.resources[index] = StageRuntimeResourceResolution{
            .reference = reference,
            .runtime = std::move(runtime),
        };

        if (!report.resources[index].runtime.ok()) {
            const auto status = report.resources[index].runtime.status;
            report.diagnostics.push_back(gdspaces::Diagnostic{
                .severity = severity_for(status),
                .code = "dmc3.stage-runtime.resource-" +
                    std::string{to_string(status)},
                .message = "Stage resource role " +
                    std::string{to_string(reference.role)} +
                    " did not resolve uniquely: " +
                    report.resources[index].runtime.detail,
                .resource = std::nullopt,
            });
        }
    }

    return report;
}

} // namespace dmc::rengine::profiles::dmc3

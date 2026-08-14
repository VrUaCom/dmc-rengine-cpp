#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
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

[[nodiscard]] std::optional<std::string> list_fallback_request(
    std::string_view logical_path) {
    const auto slash = logical_path.find_last_of("/\\");
    const auto dot = logical_path.find_last_of('.');
    if (dot == std::string_view::npos ||
        (slash != std::string_view::npos && dot < slash)) {
        return std::nullopt;
    }

    std::string result{logical_path.substr(0U, dot)};
    result += ".lst";
    return result;
}

void add_resolution_diagnostic(
    StageRuntimeResolutionReport& report,
    StageResourceRole role,
    RuntimeResolutionStatus status,
    std::string_view prefix,
    std::string_view detail) {
    report.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity_for(status),
        .code = "dmc3.stage-runtime." + std::string{prefix} + "-" +
            std::string{to_string(status)},
        .message = "Stage resource role " + std::string{to_string(role)} +
            " did not resolve uniquely: " + std::string{detail},
        .resource = std::nullopt,
    });
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

    auto report = resolve_row(
        entry.catalog_entry_id,
        entry.evidence_id,
        entry.observation,
        bootstrap,
        bindings,
        sources);

    // Raw-row resolution carries structural kind16/path evidence but cannot know
    // gameplay semantics. Restore only the separately evidenced semantic stage
    // identity from the catalog entry, without replacing technical row identity.
    if (report.plan.valid() && entry.semantic_stage_id.has_value()) {
        report.plan.semantic_stage_id = *entry.semantic_stage_id;
    }
    return report;
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
            .message = "A stable executable descriptor-row identity is required before Stage resources can be resolved.",
            .resource = std::nullopt,
        });
        return report;
    }

    if (!row.complete()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.incomplete-exe-row",
            .message = "The executable Stage descriptor row is incomplete and cannot be resolved.",
            .resource = std::nullopt,
        });
        return report;
    }

    report.plan = make_stage_resource_plan_from_table_row(
        catalog_entry_id, row.logical_paths(), std::move(evidence_id));
    for (std::size_t index = 0U; index < report.plan.resources.size(); ++index) {
        report.plan.resources[index].kind16 = row.cells[index].kind16;
    }

    if (!report.plan.valid()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage-runtime.invalid-row-plan",
            .message = "The executable Stage descriptor row did not produce a valid four-role resource plan.",
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
            .list_fallback = std::nullopt,
        };
        auto& resolved_role = report.resources[index];

        if (!resolved_role.runtime.ok()) {
            add_resolution_diagnostic(
                report,
                reference.role,
                resolved_role.runtime.status,
                "resource",
                resolved_role.runtime.detail);
        }

        // Wave 2 directly confirms this branch only after the original path is
        // unavailable and only for kind16 == 0. We reproduce the fallback lookup
        // itself, but a found .lst is not promoted to a normal Stage root because
        // list parsing, recursive ownership and lifetime are still open research.
        if (resolved_role.runtime.status != RuntimeResolutionStatus::not_found ||
            reference.kind16 != 0U) {
            continue;
        }

        const auto fallback_request = list_fallback_request(reference.logical_path);
        if (!fallback_request.has_value()) {
            report.diagnostics.push_back(gdspaces::Diagnostic{
                .severity = gdspaces::DiagnosticSeverity::error,
                .code = "dmc3.stage-runtime.list-fallback-path-unresolved",
                .message = "Wave-2 kind16==0 requires extension rewrite to .lst, but the logical path has no evidenced replaceable extension.",
                .resource = std::nullopt,
            });
            continue;
        }

        resolved_role.list_fallback = RuntimeResourceResolver::resolve(
            *fallback_request,
            bootstrap,
            bindings,
            sources);

        if (resolved_role.list_fallback->ok()) {
            report.diagnostics.push_back(gdspaces::Diagnostic{
                .severity = gdspaces::DiagnosticSeverity::error,
                .code = "dmc3.stage-runtime.list-fallback-required",
                .message = "The primary Stage resource is absent and the evidenced kind16==0 .lst fallback resolves, but list expansion/ownership is not yet reconstructed strongly enough to claim equivalent resolution.",
                .resource = resolved_role.list_fallback->resolved->id,
            });
        } else {
            add_resolution_diagnostic(
                report,
                reference.role,
                resolved_role.list_fallback->status,
                "list-fallback",
                resolved_role.list_fallback->detail);
        }
    }

    return report;
}

} // namespace dmc::rengine::profiles::dmc3

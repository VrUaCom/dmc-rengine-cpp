#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] gdspaces::DiagnosticSeverity severity_from_text(
    std::string_view severity) noexcept {
    if (severity == "error") {
        return gdspaces::DiagnosticSeverity::error;
    }
    if (severity == "warning") {
        return gdspaces::DiagnosticSeverity::warning;
    }
    return gdspaces::DiagnosticSeverity::info;
}

void add_diagnostic(
    StageWorkspaceBuildResult& result,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::optional<gdspaces::ResourceId> resource = std::nullopt) {
    result.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::move(resource),
    });
}

} // namespace

bool StageWorkspaceBuildResult::complete() const noexcept {
    if (!match.complete() || !stage.has_value() || !stage->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

StageWorkspaceBuildResult StageWorkspaceBuilder::build(
    const StageResourceRowPlan& plan,
    std::vector<gdspaces::ResourcePayload> payloads,
    const evidence::EvidencePacket* packet) {
    StageWorkspaceBuildResult result;

    if (!plan.valid()) {
        add_diagnostic(
            result,
            gdspaces::DiagnosticSeverity::error,
            "stage-workspace.invalid-plan",
            "The DMC3 stage resource row plan is invalid.");
        return result;
    }

    if (packet != nullptr && !result.project.import_evidence_packet(*packet)) {
        add_diagnostic(
            result,
            gdspaces::DiagnosticSeverity::error,
            "stage-workspace.evidence-import-failed",
            "The supplied Evidence Packet could not be imported transactionally.");
        return result;
    }

    std::vector<gdspaces::ResourceRef> resources;
    resources.reserve(payloads.size());
    for (auto& payload : payloads) {
        if (!payload.resource.valid()) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stage-workspace.invalid-resource",
                "A supplied GDSpaces payload has an invalid resource identity.");
            continue;
        }

        const auto resource = payload.resource;
        resources.push_back(resource);
        if (!result.project.create_session(
                std::move(payload),
                integration::WorkspaceContext{
                    .stage_context = true,
                    .menu_context = false,
                    .evidence_context = true,
                })) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stage-workspace.session-failed",
                "A ResourceWorkspaceSession could not be created.",
                resource.id);
        }
    }

    result.match = match_stage_resources(plan, resources);
    for (const auto& diagnostic : result.match.diagnostics) {
        add_diagnostic(
            result,
            severity_from_text(diagnostic.severity),
            diagnostic.code,
            diagnostic.role.empty()
                ? diagnostic.message
                : diagnostic.role + ": " + diagnostic.message);
    }

    const gdspaces::StageIdentity identity{
        .profile = "dmc3-hd",
        .stage_id = plan.stage_id,
        .display_name = "Stage " + plan.stage_id,
        .exe_evidence_id = plan.evidence_id,
    };
    auto assembled = gdspaces::StageBundleAssembler::assemble(
        identity, resources);
    for (const auto& diagnostic : assembled.diagnostics) {
        result.diagnostics.push_back(diagnostic);
    }

    if (assembled.bundle.valid()) {
        result.stage = std::move(assembled.bundle);
        const auto attached = result.project.attach_stage_bundle(*result.stage);
        if (attached != result.stage->size()) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stage-workspace.attach-incomplete",
                "Not every assembled stage member has a matching project session.");
        }
    } else {
        add_diagnostic(
            result,
            gdspaces::DiagnosticSeverity::error,
            "stage-workspace.bundle-invalid",
            "StageBundleAssembler did not produce a valid bundle.");
    }

    for (const auto& resource : resources) {
        const auto analysis = integration::ResourceAnalyzer::analyze(
            result.project, resource.id);
        result.analyses.push_back(analysis);
        for (const auto& diagnostic : analysis.diagnostics) {
            if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
                result.diagnostics.push_back(diagnostic);
            }
        }
        static_cast<void>(result.project.link_format_evidence(resource.id));
    }

    if (packet != nullptr && !plan.evidence_id.empty()) {
        for (const auto& match : result.match.matches) {
            if (match.status == StageResourceMatchStatus::unique &&
                !match.candidates.empty()) {
                static_cast<void>(result.project.link_evidence_record(
                    match.candidates.front().id,
                    plan.evidence_id));
            }
        }
    }

    return result;
}

} // namespace dmc::rengine::profiles::dmc3

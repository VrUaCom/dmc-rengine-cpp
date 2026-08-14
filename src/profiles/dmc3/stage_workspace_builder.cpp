#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

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

    result.match = StageResourceMatcher::match(plan, resources);
    result.diagnostics.insert(
        result.diagnostics.end(),
        result.match.diagnostics.begin(),
        result.match.diagnostics.end());

    auto candidates = result.match.unique_candidates();
    std::set<std::string, std::less<>> matched_ids;
    for (const auto& candidate : candidates) {
        matched_ids.insert(candidate.resource.id.canonical());
    }
    for (const auto& resource : resources) {
        if (matched_ids.contains(resource.id.canonical())) {
            continue;
        }
        candidates.push_back(gdspaces::StageMemberCandidate{
            .resource = resource,
            .category = std::nullopt,
            .role = "discovered:" + resource.id.logical_path,
        });
    }

    const auto resource_set_id = std::string{plan.resource_set_key()};
    const auto display_name = plan.semantic_stage_known()
        ? "Stage " + plan.semantic_stage_id
        : "Stage resource set " + resource_set_id;
    const gdspaces::StageIdentity identity{
        .profile = "dmc3-hd",
        .stage_id = resource_set_id,
        .display_name = display_name,
        .exe_evidence_id = plan.evidence_id,
        .resource_set_id = resource_set_id,
        .semantic_stage_id = plan.semantic_stage_id,
        .numeric_stage_id = plan.numeric_stage_id,
    };
    auto bundle = gdspaces::StageBundleAssembler::assemble(
        identity, candidates);
    result.diagnostics.insert(
        result.diagnostics.end(),
        bundle.diagnostics().begin(),
        bundle.diagnostics().end());

    if (bundle.valid()) {
        result.stage = std::move(bundle);
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
        for (const auto& role : result.match.roles) {
            if (role.matches.size() == 1U) {
                static_cast<void>(result.project.link_evidence_record(
                    role.matches.front().id,
                    plan.evidence_id));
            }
        }
    }

    return result;
}

} // namespace dmc::rengine::profiles::dmc3

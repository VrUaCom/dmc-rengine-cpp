#include "dmc_rengine/stageops/scene_controller.hpp"

#include "dmc_rengine/stageops/domain_knowledge.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] StageSceneRefreshResult failed(
    StageSceneRefreshResult result,
    StageSceneRefreshStatus status,
    std::string error,
    const StageOperationsSession* session = nullptr) {
    result.status = status;
    result.error = std::move(error);
    if (session != nullptr) {
        result.final_stage_revision = session->stage_revision();
    }
    return result;
}

} // namespace

StageSceneRefreshResult StageSceneController::refresh(
    StageOperationsSession& session,
    std::vector<StageRuntimeLink> explicit_runtime_links) {
    StageSceneRefreshResult result;
    if (!session.valid()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::invalid_session,
            "Stage Ops session is invalid.");
    }

    // Establish the exact stage revision this refresh intends to rebuild.
    // External changes are promoted into one Stage Ops revision before any
    // parser/domain work begins.
    result.external_change_detected =
        session.detect_external_project_changes();
    result.target_stage_revision = session.stage_revision();

    // Re-run only the shared canonical format analyzers. Formats without an
    // analyzer remain legitimate partial/untyped scene resources; failed
    // analyzers do not.
    result.analysis = session.refresh_resource_analysis();
    if (!result.analysis.complete_for_attempted()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::analysis_failed,
            "One or more canonical resource analyzers failed for the target stage revision.",
            &session);
    }
    if (result.analysis.missing_project_session_count != 0U) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::missing_project_sessions,
            "One or more materialized Stage Ops resources have no ProjectWorkspace session.",
            &session);
    }

    // Build domains explicitly before the snapshot so evidence links can be
    // validated against the exact post-analysis domain identities. High-level
    // refresh must never silently drop a stale/malformed runtime link.
    auto domains = StageDomainAssembler::assemble(
        session.assembly(),
        session.project(),
        result.target_stage_revision);
    if (!domains.valid() || !domains.current_for_active_bytes()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::provisional_snapshot_invalid,
            "Stage Ops domain knowledge does not describe the active resource bytes after analysis.",
            &session);
    }

    result.runtime_link_validation = StageRuntimeLinkValidator::validate(
        domains, explicit_runtime_links);
    if (!result.runtime_link_validation.valid() ||
        result.runtime_link_validation.accepted_link_count !=
            explicit_runtime_links.size()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::runtime_links_invalid,
            "Explicit recovered-runtime links are invalid for the target Stage Ops domain revision.",
            &session);
    }

    auto knowledge = StageDomainKnowledgeBuilder::build(
        std::move(domains), explicit_runtime_links);
    if (!knowledge.valid() || !knowledge.current_for_active_bytes()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::provisional_snapshot_invalid,
            "Stage Ops could not build coherent domain knowledge for the target revision.",
            &session);
    }

    // The actual mutation/TOCTOU gate lives in StageOperationsSession. If any
    // ProjectWorkspace bytes changed between analysis and this commit, the
    // session advances its revision, preserves stale state and rejects this
    // exact target revision.
    if (!session.commit_derived_refresh(result.target_stage_revision)) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::concurrent_change,
            "Stage resource bytes changed while the scene refresh transaction was in flight.",
            &session);
    }

    // Rebuild the published immutable snapshot after the commit rather than
    // publishing the provisional object assembled before the concurrency gate.
    result.snapshot = StageSceneSnapshotBuilder::build(
        session, std::move(explicit_runtime_links));
    result.final_stage_revision = session.stage_revision();
    if (!result.snapshot.valid() ||
        result.snapshot.stage_revision() != result.target_stage_revision ||
        !result.snapshot.current_for_active_bytes()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::final_snapshot_invalid,
            "The committed Stage Ops revision could not be published as a current coherent scene snapshot.",
            &session);
    }

    result.status = StageSceneRefreshStatus::refreshed;
    result.error.clear();
    return result;
}

} // namespace dmc::rengine::stageops

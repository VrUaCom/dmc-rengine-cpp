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

[[nodiscard]] StageSceneRefreshResult refresh_impl(
    StageOperationsSession& session,
    StageRuntimeLinkProvider provider) {
    StageSceneRefreshResult result;
    if (!session.valid()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::invalid_session,
            "Stage Ops session is invalid.");
    }

    result.external_change_detected =
        session.detect_external_project_changes();
    result.target_stage_revision = session.stage_revision();

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

    auto provider_result = provider
        ? provider(domains)
        : StageRuntimeLinkProviderResult{};
    if (!provider_result.coherent() || !provider_result.valid) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::runtime_link_provider_failed,
            provider_result.error.empty()
                ? "The runtime-link provider failed without a diagnostic."
                : std::move(provider_result.error),
            &session);
    }

    auto runtime_links = std::move(provider_result.links);
    result.runtime_link_validation = StageRuntimeLinkValidator::validate(
        domains, runtime_links);
    if (!result.runtime_link_validation.valid() ||
        result.runtime_link_validation.accepted_link_count !=
            runtime_links.size()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::runtime_links_invalid,
            "Explicit recovered-runtime links are invalid for the target Stage Ops domain revision.",
            &session);
    }

    auto knowledge = StageDomainKnowledgeBuilder::build(
        std::move(domains), runtime_links);
    if (!knowledge.valid() || !knowledge.current_for_active_bytes()) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::provisional_snapshot_invalid,
            "Stage Ops could not build coherent domain knowledge for the target revision.",
            &session);
    }

    if (!session.commit_derived_refresh(result.target_stage_revision)) {
        return failed(
            std::move(result),
            StageSceneRefreshStatus::concurrent_change,
            "Stage resource bytes changed while the scene refresh transaction was in flight.",
            &session);
    }

    result.snapshot = StageSceneSnapshotBuilder::build(
        session, std::move(runtime_links));
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

} // namespace

StageSceneRefreshResult StageSceneController::refresh(
    StageOperationsSession& session,
    std::vector<StageRuntimeLink> explicit_runtime_links) {
    return refresh_impl(
        session,
        [links = std::move(explicit_runtime_links)](
            const StageDomainWorkspace&) mutable {
            return StageRuntimeLinkProviderResult{
                .valid = true,
                .links = std::move(links),
                .error = {},
            };
        });
}

StageSceneRefreshResult StageSceneController::refresh_with_provider(
    StageOperationsSession& session,
    StageRuntimeLinkProvider provider) {
    return refresh_impl(session, std::move(provider));
}

} // namespace dmc::rengine::stageops

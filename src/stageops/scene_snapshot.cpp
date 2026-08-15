#include "dmc_rengine/stageops/scene_snapshot.hpp"

#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] bool same_identity(
    const StageAssemblyIdentity& left,
    const StageAssemblyIdentity& right) noexcept {
    return left.catalog_entry_id == right.catalog_entry_id &&
        left.global_catalog_row == right.global_catalog_row &&
        left.source_table_id == right.source_table_id &&
        left.source_row_index == right.source_row_index &&
        left.stage.profile == right.stage.profile &&
        left.stage.resource_set_key() == right.stage.resource_set_key() &&
        left.stage.numeric_stage_id == right.stage.numeric_stage_id &&
        left.stage.semantic_stage_id == right.stage.semantic_stage_id &&
        left.stage.exe_evidence_id == right.stage.exe_evidence_id;
}

} // namespace

bool StageSceneSnapshot::valid() const noexcept {
    if (!operations.valid() || !knowledge.valid() || !semantic_graph.valid()) {
        return false;
    }
    if (knowledge.source_stage_revision() != operations.stage_revision ||
        semantic_graph.source_stage_revision != operations.stage_revision) {
        return false;
    }
    if (!same_identity(knowledge.identity(), semantic_graph.source_identity)) {
        return false;
    }
    if (operations.assembly_status != semantic_graph.assembly_status ||
        operations.game_readiness != semantic_graph.game_readiness) {
        return false;
    }
    return true;
}

StageSceneSnapshot StageSceneSnapshotBuilder::build(
    const StageOperationsSession& session,
    std::vector<StageRuntimeLink> explicit_runtime_links) {
    StageSceneSnapshot snapshot;
    if (!session.valid()) {
        return snapshot;
    }

    snapshot.operations = session.snapshot();
    auto domains = StageDomainAssembler::assemble(
        session.assembly(),
        session.project(),
        snapshot.operations.stage_revision);
    if (!domains.valid()) {
        return StageSceneSnapshot{};
    }

    snapshot.knowledge = StageDomainKnowledgeBuilder::build(
        std::move(domains),
        std::move(explicit_runtime_links));
    if (!snapshot.knowledge.valid()) {
        return StageSceneSnapshot{};
    }

    snapshot.semantic_graph = semantic::StageSemanticGraphBuilder::build(
        session.assembly(),
        snapshot.knowledge,
        snapshot.operations.stage_revision);
    if (!snapshot.semantic_graph.valid()) {
        return StageSceneSnapshot{};
    }
    return snapshot;
}

} // namespace dmc::rengine::stageops

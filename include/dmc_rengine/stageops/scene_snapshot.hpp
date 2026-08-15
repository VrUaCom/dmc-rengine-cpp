#pragma once

#include "dmc_rengine/stageops/domain_knowledge.hpp"
#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/semantic_graph.hpp"

#include <cstdint>
#include <vector>

namespace dmc::rengine::stageops {

// Immutable, revision-coherent view of the Stage Ops-owned scene state.
//
// This is not another scene authority: assembly and operations remain owned by
// StageOperationsSession. The snapshot composes already-owned Stage Ops state so
// UI/ModViz/graph consumers cannot accidentally combine knowledge from revision
// N with operations or semantic relationships from revision N+1.
struct StageSceneSnapshot final {
    StageOperationsSnapshot operations;
    StageDomainKnowledgeWorkspace knowledge;
    semantic::StageSemanticGraph semantic_graph;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] std::uint64_t stage_revision() const noexcept {
        return operations.stage_revision;
    }

    [[nodiscard]] const StageDomainWorkspace& domains() const noexcept {
        return knowledge.domains;
    }

    [[nodiscard]] const StageDomainRelationWorkspace& relations() const noexcept {
        return knowledge.relations;
    }

    [[nodiscard]] const StageRuntimeLinkWorkspace& runtime_links() const noexcept {
        return knowledge.runtime_links;
    }

    // Current means the snapshot was derived from the active resource bytes and
    // StageOperationsSession has no newer uncommitted derived invalidation.
    [[nodiscard]] bool current_for_active_bytes() const noexcept {
        return valid() &&
            !operations.derived_state_stale &&
            knowledge.current_for_active_bytes();
    }

    [[nodiscard]] bool requires_derived_refresh() const noexcept {
        return !valid() ||
            operations.derived_state_stale ||
            !knowledge.current_for_active_bytes();
    }
};

class StageSceneSnapshotBuilder final {
public:
    // Pure projection over the current StageOperationsSession. It does not
    // detect external changes implicitly and never clears the stale gate.
    // Callers explicitly run detect_external_project_changes()/analysis refresh
    // and commit_derived_refresh() as part of Stage Ops operations.
    //
    // Runtime links are accepted only as explicit evidence-backed input from a
    // profile/reverse integration layer. Generic Stage Ops never derives them
    // from token text, filenames or graph labels.
    [[nodiscard]] static StageSceneSnapshot build(
        const StageOperationsSession& session,
        std::vector<StageRuntimeLink> explicit_runtime_links = {});
};

} // namespace dmc::rengine::stageops

#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"
#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/semantic_graph.hpp"

#include <cstdint>

namespace dmc::rengine::stageops {

// Immutable, revision-coherent view of the Stage Ops-owned scene state.
//
// This is not another scene authority: assembly and operations remain owned by
// StageOperationsSession. The snapshot composes already-owned Stage Ops state so
// UI/ModViz/graph consumers cannot accidentally combine domains from revision N
// with operations or semantic relationships from revision N+1.
struct StageSceneSnapshot final {
    StageOperationsSnapshot operations;
    StageDomainWorkspace domains;
    semantic::StageSemanticGraph semantic_graph;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] std::uint64_t stage_revision() const noexcept {
        return operations.stage_revision;
    }

    // Current means the snapshot was derived from the active resource bytes and
    // StageOperationsSession has no newer uncommitted derived invalidation.
    [[nodiscard]] bool current_for_active_bytes() const noexcept {
        return valid() &&
            !operations.derived_state_stale &&
            domains.current_for_active_bytes();
    }

    [[nodiscard]] bool requires_derived_refresh() const noexcept {
        return !valid() ||
            operations.derived_state_stale ||
            !domains.current_for_active_bytes();
    }
};

class StageSceneSnapshotBuilder final {
public:
    // Pure projection over the current StageOperationsSession. It does not
    // detect external changes implicitly and never clears the stale gate.
    // Callers explicitly run detect_external_project_changes()/analysis refresh
    // and commit_derived_refresh() as part of Stage Ops operations.
    [[nodiscard]] static StageSceneSnapshot build(
        const StageOperationsSession& session);
};

} // namespace dmc::rengine::stageops

#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"
#include "dmc_rengine/stageops/runtime_links.hpp"
#include "dmc_rengine/stageops/scene_controller.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::bridges::dmc3 {

struct RecoveredStageLinkReport final {
    std::vector<stageops::StageRuntimeLink> links;
    std::size_t door_marker_count{};
    std::size_t box_in_marker_count{};

    [[nodiscard]] bool valid() const noexcept {
        return links.size() == door_marker_count + box_in_marker_count;
    }
};

// Explicit DMC3-only composition layer between product Stage Ops domain state
// and recovered vanilla-runtime contracts. This bridge is compiled separately
// from DMCRengine::Core so generic Stage Ops never acquires a hidden dependency
// on DMC3 recovered source.
class RecoveredStageLinkProvider final {
public:
    [[nodiscard]] static RecoveredStageLinkReport build(
        const stageops::StageDomainWorkspace& domains);
};

// DMC3 composition root for one canonical scene refresh. Runtime links are
// generated from the exact post-analysis StageDomainWorkspace@revision inside
// StageSceneController, then validated and published through the same guarded
// transaction as every other Stage Ops consumer.
class RecoveredStageSceneController final {
public:
    [[nodiscard]] static stageops::StageSceneRefreshResult refresh(
        stageops::StageOperationsSession& session);
};

} // namespace dmc::rengine::bridges::dmc3

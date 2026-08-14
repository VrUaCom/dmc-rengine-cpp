#pragma once

#include "dmc_rengine/stageops/domain_relations.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"
#include "dmc_rengine/stageops/runtime_links.hpp"

#include <cstdint>
#include <vector>

namespace dmc::rengine::stageops {

// One revision-coherent Stage Ops knowledge model over already-assembled scene
// resources. This remains Stage Ops-owned state; Stage Semantic Graph is a
// disposable representation of it rather than an authority alongside it.
struct StageDomainKnowledgeWorkspace final {
    StageDomainWorkspace domains;
    StageDomainRelationWorkspace relations;
    StageRuntimeLinkWorkspace runtime_links;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] const StageAssemblyIdentity& identity() const noexcept {
        return domains.identity;
    }

    [[nodiscard]] std::uint64_t source_stage_revision() const noexcept {
        return domains.source_stage_revision;
    }

    [[nodiscard]] bool current_for_active_bytes() const noexcept {
        return valid() && domains.current_for_active_bytes();
    }
};

class StageDomainKnowledgeBuilder final {
public:
    // Builds Stage Ops structural relations from the supplied domain workspace
    // and validates an explicit set of recovered-runtime links against the same
    // exact domain identity/revision. No semantic relation is inferred from
    // filenames or token text.
    [[nodiscard]] static StageDomainKnowledgeWorkspace build(
        StageDomainWorkspace domains,
        std::vector<StageRuntimeLink> explicit_runtime_links = {});
};

} // namespace dmc::rengine::stageops

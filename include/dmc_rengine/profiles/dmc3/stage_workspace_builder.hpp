#pragma once

#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"

#include <optional>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// Compatibility result for the pre-runtime payload-import path.
//
// IMPORTANT: this is not Stage Ops scene authority and `complete()` means only
// that the supplied payload set matched the legacy row plan and passed the
// builder's project/parser checks. It does not mean StageAssemblyWorkspace is
// complete and never means original-game-ready equivalence.
struct StageWorkspaceBuildResult final {
    integration::ProjectWorkspace project;
    StageResourceMatchReport match;
    std::optional<gdspaces::StageBundle> stage;
    std::vector<integration::ResourceAnalysisReport> analyses;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
};

// Legacy payload-import/synthetic-fixture harness retained during migration.
//
// Canonical runtime-backed stage opening is:
//   StageRuntimeLoader -> StageOpsIngress -> StageAssemblyWorkspace
//
// This class must not gain resource lookup, runtime lifecycle, Stage Ops domain
// assembly, Semantic Graph ownership or ModViz scene-assembly responsibilities.
class StageWorkspaceBuilder final {
public:
    [[deprecated(
        "Legacy payload-import harness; use StageRuntimeLoader + StageOpsIngress for canonical Stage Ops assembly")]]
    [[nodiscard]] static StageWorkspaceBuildResult build(
        const StageResourceRowPlan& plan,
        std::vector<gdspaces::ResourcePayload> payloads,
        const evidence::EvidencePacket* packet = nullptr);
};

} // namespace dmc::rengine::profiles::dmc3

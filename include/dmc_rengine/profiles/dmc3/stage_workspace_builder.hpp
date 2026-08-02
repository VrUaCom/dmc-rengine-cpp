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

struct StageWorkspaceBuildResult final {
    integration::ProjectWorkspace project;
    StageResourceMatchReport match;
    std::optional<gdspaces::StageBundle> stage;
    std::vector<integration::ResourceAnalysisReport> analyses;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
};

class StageWorkspaceBuilder final {
public:
    [[nodiscard]] static StageWorkspaceBuildResult build(
        const StageResourceRowPlan& plan,
        std::vector<gdspaces::ResourcePayload> payloads,
        const evidence::EvidencePacket* packet = nullptr);
};

} // namespace dmc::rengine::profiles::dmc3

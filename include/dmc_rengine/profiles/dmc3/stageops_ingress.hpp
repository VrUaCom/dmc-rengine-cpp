#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageOpsIngressResult final {
    stageops::StageAssemblyWorkspace assembly;
    std::vector<integration::ResourceAnalysisReport> analyses;
    std::vector<gdspaces::Diagnostic> diagnostics;
    std::size_t sessions_created{};
    std::size_t sessions_reused{};

    [[nodiscard]] bool valid() const noexcept;
};

// Canonical bridge from the already-resolved/materialized DMC3 Stage runtime
// report into shared product workspace state.
//
// It does NOT perform a second lookup or container traversal. It first derives
// StageAssemblyWorkspace from the report, then moves the exact materialized root
// and nested payloads into ProjectWorkspace sessions, reusing an existing
// canonical ResourceId only when its immutable source payload agrees.
class StageOpsIngress final {
public:
    [[nodiscard]] static StageOpsIngressResult attach(
        StageRuntimeLoadReport report,
        integration::ProjectWorkspace& project);
};

} // namespace dmc::rengine::profiles::dmc3

#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/patch/guarded_patch.hpp"
#include "dmc_rengine/patch/item_runtime_compiler.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::patch {

struct CopyExecutionDiagnostic final {
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct PatchCopyArtifact final {
    std::string id;
    std::string patch_plan_id;
    std::string rollback_plan_id;
    gdspaces::ResourceId source_resource;
    std::string target_artifact_id;
    std::string source_sha256;
    std::string output_sha256;
    std::vector<std::byte> output_bytes;
    GuardedPatchPlan rollback_plan;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !patch_plan_id.empty() &&
               !rollback_plan_id.empty() && source_resource.valid() &&
               !target_artifact_id.empty() && source_sha256.size() == 64U &&
               output_sha256.size() == 64U &&
               source_sha256 != output_sha256 && !output_bytes.empty() &&
               rollback_plan.id() == rollback_plan_id &&
               rollback_plan.target_sha256() == output_sha256 &&
               rollback_plan.patches().size() == 1U;
    }
};

struct PatchCopyExecutionResult final {
    std::optional<PatchCopyArtifact> artifact;
    std::vector<CopyExecutionDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return artifact.has_value() && artifact->valid() &&
               diagnostics.empty();
    }
};

class PatchCopyExecutor final {
public:
    [[nodiscard]] static PatchCopyExecutionResult execute(
        const integration::ProjectWorkspace& project,
        const CompiledItemRuntimePatchPlan& compiled);
};

} // namespace dmc::rengine::patch

#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/patch/guarded_patch.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::patch {

struct ItemRuntimePatchInput final {
    std::string evidence_record_id;
    std::vector<std::byte> expected;
    std::vector<std::byte> replacement;
    std::string description;

    [[nodiscard]] bool valid() const noexcept {
        return !evidence_record_id.empty() && !expected.empty() &&
               expected.size() == replacement.size() &&
               !description.empty();
    }
};

struct PatchCompilationDiagnostic final {
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct CompiledPatchMetadata final {
    std::string id;
    std::string runtime_request_id;
    std::string validation_plan_id;
    gdspaces::ResourceId item_resource;
    gdspaces::ResourceId executable_resource;
    std::string target_artifact_id;
    std::string evidence_record_id;
    std::uint64_t file_offset{};
    std::uint64_t rva{};
    std::uint64_t va{};
    std::uint64_t size{};
    std::string source_sha256;
    std::string expected_hex;
    std::string replacement_hex;
    std::string description;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !runtime_request_id.empty() &&
               !validation_plan_id.empty() && item_resource.valid() &&
               executable_resource.valid() && !target_artifact_id.empty() &&
               !evidence_record_id.empty() && size != 0U &&
               source_sha256.size() == 64U && !expected_hex.empty() &&
               expected_hex.size() == replacement_hex.size() &&
               !description.empty();
    }
};

struct CompiledItemRuntimePatchPlan final {
    GuardedPatchPlan guarded_plan;
    CompiledPatchMetadata metadata;

    [[nodiscard]] bool valid() const noexcept {
        return metadata.valid() &&
               guarded_plan.id() == metadata.id &&
               guarded_plan.target_sha256() == metadata.source_sha256 &&
               guarded_plan.patches().size() == 1U;
    }
};

struct ItemRuntimePatchCompilationResult final {
    std::optional<CompiledItemRuntimePatchPlan> compiled;
    std::vector<PatchCompilationDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return compiled.has_value() && compiled->valid() &&
               diagnostics.empty();
    }
};

class ItemRuntimePatchCompiler final {
public:
    [[nodiscard]] static ItemRuntimePatchCompilationResult compile(
        const integration::ProjectWorkspace& project,
        const item::RuntimeChangeRequest& request,
        const validation::ItemRuntimeValidationPlan& validation_plan,
        ItemRuntimePatchInput input);
};

} // namespace dmc::rengine::patch

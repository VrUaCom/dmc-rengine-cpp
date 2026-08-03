#include "dmc_rengine/patch/copy_executor.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::patch {
namespace {

void add_diagnostic(
    PatchCopyExecutionResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(CopyExecutionDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

} // namespace

PatchCopyExecutionResult PatchCopyExecutor::execute(
    const integration::ProjectWorkspace& project,
    const CompiledItemRuntimePatchPlan& compiled) {
    PatchCopyExecutionResult result;
    if (!compiled.valid()) {
        add_diagnostic(
            result,
            "copy-executor.compiled-plan-invalid",
            "The compiled guarded patch plan is invalid.");
        return result;
    }

    const auto& metadata = compiled.metadata;
    if (project.graph().find(metadata.id) == nullptr) {
        add_diagnostic(
            result,
            "copy-executor.patch-plan-not-registered",
            "The compiled patch plan is absent from Spider Hub provenance.");
        return result;
    }

    const auto* session = project.find_session(metadata.executable_resource);
    const auto* artifact = project.artifacts().find(metadata.target_artifact_id);
    if (session == nullptr || artifact == nullptr ||
        session->executable_context() == nullptr) {
        add_diagnostic(
            result,
            "copy-executor.source-context-missing",
            "The executable session or target artifact is unavailable.");
        return result;
    }
    if (session->executable_context()->sha256 != artifact->sha256 ||
        metadata.source_sha256 != artifact->sha256) {
        add_diagnostic(
            result,
            "copy-executor.source-identity-mismatch",
            "The executable session, compiled metadata, and artifact SHA-256 do not agree.");
        return result;
    }

    const auto& source_bytes = session->source_payload().bytes;
    const auto source_hash = core::Sha256::compute(
        std::span<const std::byte>{source_bytes}).hex();
    if (source_hash != metadata.source_sha256) {
        add_diagnostic(
            result,
            "copy-executor.source-hash-mismatch",
            "The immutable source payload no longer matches the compiled patch plan SHA-256.");
        return result;
    }

    const auto applied = compiled.guarded_plan.apply(
        std::span<const std::byte>{source_bytes});
    if (!applied.applied) {
        add_diagnostic(
            result,
            "copy-executor.guarded-apply-failed",
            applied.error.empty()
                ? "GuardedPatchPlan rejected the copy execution."
                : applied.error);
        return result;
    }
    if (applied.output == source_bytes) {
        add_diagnostic(
            result,
            "copy-executor.output-unchanged",
            "The guarded patch execution produced no byte changes.");
        return result;
    }

    const auto output_hash = core::Sha256::compute(
        std::span<const std::byte>{applied.output}).hex();
    const auto& patch = compiled.guarded_plan.patches().front();
    const auto rollback_id = "rollback-plan:" + metadata.id;
    GuardedPatchPlan rollback(rollback_id, output_hash);
    if (!rollback.add(BytePatch{
            .offset = patch.offset,
            .expected = patch.replacement,
            .replacement = patch.expected,
            .description = "Rollback for " + metadata.id,
        })) {
        add_diagnostic(
            result,
            "copy-executor.rollback-plan-rejected",
            "The reverse guarded patch could not be constructed.");
        return result;
    }

    const auto rollback_check = rollback.apply(
        std::span<const std::byte>{applied.output});
    if (!rollback_check.applied || rollback_check.output != source_bytes) {
        add_diagnostic(
            result,
            "copy-executor.rollback-verification-failed",
            "The generated rollback plan did not reconstruct the immutable source bytes exactly.");
        return result;
    }

    PatchCopyArtifact copy{
        .id = "patch-execution:" + metadata.id,
        .patch_plan_id = metadata.id,
        .rollback_plan_id = rollback_id,
        .source_resource = metadata.executable_resource,
        .target_artifact_id = metadata.target_artifact_id,
        .source_sha256 = source_hash,
        .output_sha256 = output_hash,
        .output_bytes = applied.output,
        .rollback_plan = std::move(rollback),
    };
    if (!copy.valid()) {
        add_diagnostic(
            result,
            "copy-executor.artifact-invalid",
            "The copied output or rollback metadata failed validity checks.");
        return result;
    }

    result.artifact = std::move(copy);
    return result;
}

} // namespace dmc::rengine::patch
